#include "common.h"
#include "StatusLEDs.h"
#include "task.h"
#include "bq.h"
#include "DebugPrintf.h"
#include <string.h>

// Inits, wakes and auto-addresses the chain, then checks every device: its address, and for the
// BQ79616s an 8-byte scratch-pad write/read round trip and that only the last one is top of stack.
// Part IDs are printed but not checked. The heartbeat blinks only if everything passed.
// In GDB: `p bq_test_fail_step`, `p bq_test_fail_dev`, `p bq_test_addr`, `p bq_test_partid`,
// `p bq_test_scratch`, `p bq_test_comm_ctrl`.
// Progress is printed on the debug UART: UART4 (PC10 TX, PC11 RX), 115200 8N1.

#define BQ_TEST_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE * 4)     // printf needs the extra room
#define BQ_TEST_TASK_PRIORITY   (tskIDLE_PRIORITY + 1)
#define BQ_TEST_NUM_DEVICES     1U      // BQ79616s behind the BQ79600 bridge

static StaticTask_t bq_test_task_buffer;
static StackType_t bq_test_task_stack[BQ_TEST_TASK_STACK_SIZE];

typedef enum {
    BQ_TEST_NONE = 0,
    BQ_TEST_INIT,
    BQ_TEST_WAKE,
    BQ_TEST_AUTO_ADDRESS,
    BQ_TEST_ADDR,               // read failed, or DIR0_ADDR isn't the device's address
    BQ_TEST_PARTID,             // PARTID read failed (the value itself isn't checked)
    BQ_TEST_SCRATCH,            // scratch-pad write/read failed, or read back different bytes
    BQ_TEST_TOP_OF_STACK,       // read failed, or COMM_CTRL's STACK_DEV/TOP_STACK bits are wrong
    BQ_TEST_CONFIG,
    BQ_TEST_VOLTAGES,           // ADC didn't finish, a read failed, or a result was never converted
} bq_test_step_t;

volatile bq_test_step_t bq_test_fail_step = BQ_TEST_NONE;
volatile uint8_t bq_test_fail_dev = 0U;
volatile uint8_t bq_test_addr[BQ_TEST_NUM_DEVICES + 1U];        // [0] = bridge, [n] = BQ79616 n
volatile uint8_t bq_test_partid[BQ_TEST_NUM_DEVICES + 1U];      // bridge: silicon revision; EVM BQ79616: 0x21
volatile uint8_t bq_test_scratch[BQ_TEST_NUM_DEVICES + 1U][8];  // what each BQ79616 read back; [0] unused
volatile uint8_t bq_test_comm_ctrl[BQ_TEST_NUM_DEVICES + 1U];   // BQ79616s only; [0] unused

// Prints why, records where for GDB, and stops the test; no heartbeat from here on
static void fail(bq_test_step_t step, uint8_t dev, const char *what) {
    printf("FAIL: device %u: %s\r\n", dev, what);
    bq_test_fail_step = step;
    bq_test_fail_dev = dev;
    vTaskSuspend(NULL);
}

// Single read of one register from one device
static bool read_byte(uint8_t dev_addr, uint16_t reg_addr, uint8_t *value) {
    static bq_response_frame_t frame;   // static: 129 bytes is a lot for this small stack
    const bq_msg_frame_t msg = {
        .req      = BQ_SINGLE_READ,
        .dev_addr = dev_addr,
        .reg_addr = reg_addr,
        .len      = 1U,
    };

    if (bq_read(&msg, &frame, 1U) != BQ_OK) {
        return false;
    }
    *value = frame.data[0];
    return true;
}

// BQ79600 registers that show what happened on the daisy chain (bq79600-q1.pdf Section 7.5).
// The bridge's COMH faces the stack, so the COMH registers cover the BQ79616 link.
static const struct {
    uint16_t reg;
    const char *name;
    const char *bits;
} bridge_comm_regs[] = {
    {0x2100, "FAULT_SUMMARY",    "b3 FAULT_COMM, b2 FAULT_REG, b1 FAULT_SYS, b0 FAULT_PWR"},
    {0x2104, "FAULT_COMM1",      "b6 FCOMM_DET, b5 FTONE_DET, b4 HB_FAIL, b3 HB_FAST, b2 UART_FRAME, b1 COMMCLR_DET, b0 STOP_DET"},
    {0x2105, "FAULT_COMM2",      "b3 COML_FRAME, b2 COML_PHY, b1 COMH_FRAME, b0 COMH_PHY"},
    {0x2300, "DEBUG_COMM_STAT",  "b4 HW_DAISY_DRV, b3 COML_TX_ON, b2 COML_RX_ON, b1 COMH_TX_ON, b0 COMH_RX_ON (reset 0x13)"},
    {0x2304, "DEBUG_COMH_PHY",   "b4 PERR, b3 BERR_TAG, b2 SYNC2, b1 SYNC1, b0 BIT"},
    {0x2305, "DEBUG_COMH_FRAME", "b4 RR_IERR, b3 RR_SOF, b2 RR_BYTE_ERR, b1 RR_UNEXP, b0 RR_CRC"},
};

// Prints the bridge's daisy-chain fault and status registers. If the BQ79616 never answered,
// clean COMH registers mean nothing came back at all (wrong COM port, or the BQ79616 resetting);
// COMH_PHY/COMH_FRAME set mean something came back but couldn't be decoded (polarity, isolation parts).
static void dump_bridge_comm_regs(void) {
    uint8_t value;

    printf("BQ79600 comm registers:\r\n");
    for (uint8_t i = 0U; i < sizeof(bridge_comm_regs) / sizeof(bridge_comm_regs[0]); i++) {
        if (read_byte(0U, bridge_comm_regs[i].reg, &value)) {
            printf("  0x%04X %-16s 0x%02X  (%s)\r\n", bridge_comm_regs[i].reg, bridge_comm_regs[i].name,
                   value, bridge_comm_regs[i].bits);
        } else {
            printf("  0x%04X %-16s read failed\r\n", bridge_comm_regs[i].reg, bridge_comm_regs[i].name);
        }
    }
}

// Writes 8 bytes to a BQ79616's CUST_MISC1..8 scratch pad and reads them back. The pattern has
// alternating, all-zero and all-one bytes, so an inverted or stuck bit can't match by accident,
// and the last two bytes depend on the device so a reply from the wrong device can't match either.
static bool scratch_round_trip(uint8_t dev) {
    static bq_response_frame_t frame;
    const uint8_t pattern[8] = {0x55U, 0xAAU, 0x00U, 0xFFU, 0x01U, 0x80U, dev, (uint8_t)~dev};
    const bq_msg_frame_t write = {
        .req      = BQ_SINGLE_WRITE,
        .dev_addr = dev,
        .reg_addr = BQ_REG_CUST_MISC1,
        .data     = pattern,
        .len      = sizeof(pattern),
    };
    const bq_msg_frame_t read = {
        .req      = BQ_SINGLE_READ,
        .dev_addr = dev,
        .reg_addr = BQ_REG_CUST_MISC1,
        .len      = sizeof(pattern),
    };

    if (bq_write(&write) != BQ_OK || bq_read(&read, &frame, 1U) != BQ_OK) {
        return false;
    }
    for (uint8_t i = 0U; i < sizeof(pattern); i++) {
        bq_test_scratch[dev][i] = frame.data[i];
    }
    printf("Device %u: scratch pad %02X %02X %02X %02X %02X %02X %02X %02X\r\n", dev,
           frame.data[0], frame.data[1], frame.data[2], frame.data[3],
           frame.data[4], frame.data[5], frame.data[6], frame.data[7]);
    return memcmp(frame.data, pattern, sizeof(pattern)) == 0;
}

static void bq_test_task(void *pvParameters) {
    const bq_init_struct_t bq_cfg = {
        .tx_pin           = GPIO_PIN_10,        // USART3 TX -> BQ RX
        .rx_pin           = GPIO_PIN_11,        // USART3 RX <- BQ TX
        .tx_rx_port       = GPIOB,
        .fault_pin        = FAN_SCL_PIN,        // NFAULT
        .fault_port       = FAN_SCL_PORT,
        .huart            = husart3,
        .uart_af          = GPIO_AF7_USART3,
        .wake_tim         = TIM2,               // PB10 AF1 = TIM2_CH3
        .wake_tim_channel = TIM_CHANNEL_3,
        .wake_tim_af      = GPIO_AF1_TIM2,
        .num_devices      = BQ_TEST_NUM_DEVICES,
    };
    uint8_t value;

    printf("\r\nBQ test start, %u BQ79616(s)\r\n", BQ_TEST_NUM_DEVICES);

    if (bq_init(bq_cfg) != BQ_OK) {
        fail(BQ_TEST_INIT, 0U, "bq_init");
    }
    printf("bq_init OK\r\n");

    if (bq_wake() != BQ_OK) {
        fail(BQ_TEST_WAKE, 0U, "bq_wake");
    }
    printf("bq_wake OK\r\n");

    // Also sets the last BQ79616 as top of stack, and checks the addresses with a stack read
    if (bq_auto_address() != BQ_OK) {
        dump_bridge_comm_regs();
        fail(BQ_TEST_AUTO_ADDRESS, 0U, "bq_auto_address");
    }
    printf("bq_auto_address OK\r\n");

    // Device 0 is the bridge, 1..BQ_TEST_NUM_DEVICES the BQ79616s. Each is read on its own
    // with a single read, so a wrong address shows up as no reply or a mismatch.
    for (uint8_t dev = 0U; dev <= BQ_TEST_NUM_DEVICES; dev++) {
        if (!read_byte(dev, BQ_REG_DIR0_ADDR, &value)) {
            fail(BQ_TEST_ADDR, dev, "no reply to DIR0_ADDR read");
        }
        bq_test_addr[dev] = value & BQ_MAX_DEV_ADDR;
        printf("Device %u: address %u\r\n", dev, bq_test_addr[dev]);
        if (bq_test_addr[dev] != dev) {
            fail(BQ_TEST_ADDR, dev, "wrong address");
        }

        if (dev == 0U) {
            // The bridge's PARTID is only its silicon revision, so there's no part number to check
            if (!read_byte(dev, BQ_REG_BRIDGE_PARTID, &value)) {
                fail(BQ_TEST_PARTID, dev, "no reply to PARTID read");
            }
            bq_test_partid[dev] = value;
            printf("Device %u: BQ79600, revision 0x%02X\r\n", dev, value);
            continue;
        }

        if (!read_byte(dev, BQ_REG_PARTID, &value)) {
            fail(BQ_TEST_PARTID, dev, "no reply to PARTID read");
        }
        bq_test_partid[dev] = value;
        // Not checked: the EVM's BQ79616 reads 0x21 but other silicon has read 0x0F, and the
        // datasheet is unclear whether this register is a part ID or a revision
        printf("Device %u: PARTID 0x%02X (EVM BQ79616 reads 0x%02X)\r\n", dev, value, BQ_REG_PARTID_BQ79616);

        // Comms check that doesn't depend on silicon: an 8-byte write and read-back
        if (!scratch_round_trip(dev)) {
            fail(BQ_TEST_SCRATCH, dev, "scratch pad didn't read back what was written");
        }

        // Every BQ79616 is a stack device; only the last one is top of stack
        const uint8_t mask = BQ_REG_COMM_CTRL_STACK_DEV | BQ_REG_COMM_CTRL_TOP_STACK;
        const uint8_t expected = (dev == BQ_TEST_NUM_DEVICES) ? mask : BQ_REG_COMM_CTRL_STACK_DEV;
        if (!read_byte(dev, BQ_REG_COMM_CTRL, &value)) {
            fail(BQ_TEST_TOP_OF_STACK, dev, "no reply to COMM_CTRL read");
        }
        bq_test_comm_ctrl[dev] = value;
        printf("Device %u: COMM_CTRL 0x%02X (expect 0x%02X)%s\r\n", dev, value & mask, expected,
               (dev == BQ_TEST_NUM_DEVICES) ? ", top of stack" : "");
        if ((value & mask) != expected) {
            fail(BQ_TEST_TOP_OF_STACK, dev, "wrong STACK_DEV/TOP_STACK bits");
        }
    }

    printf("PASS\r\n");

    if (bq_config() != BQ_OK) {
        fail(BQ_TEST_CONFIG, 0U, "config did not take :(");
    }

    // Read every second. This also keeps talking to the stack, so the 1 min long comm
    // timeout that bq_config() set never shuts the BQ79616s down.
    static uint16_t cell_mv[BQ_MAX_DEVICES][BQ_NUM_CELLS];
    while (true) {
        if (bq_read_cell_voltages(cell_mv) != BQ_OK) {
            fail(BQ_TEST_VOLTAGES, 0U, "bq_read_cell_voltages");
        }
        for (uint8_t dev = 1U; dev <= BQ_TEST_NUM_DEVICES; dev++) {
            printf("Device %u:", dev);
            for (uint8_t cell = 1U; cell <= BQ_NUM_CELLS; cell++) {
                // mV split into volts and millivolts, so no floating point is needed
                const uint16_t mv = cell_mv[dev - 1U][cell - 1U];
                printf(" C%u %u.%03u V", cell, mv / 1000U, mv % 1000U);
            }
            printf("\r\n");
        }

        toggleHeartbeat();
        vTaskDelay(pdMS_TO_TICKS(500));
        toggleHeartbeat();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

int main(void) {
    HAL_Init();
    SystemClock_Config();

    LEDs_init();

    // USART3 belongs to the BQ, so debug prints go to UART4
    debugPrintf_init_uart(huart4);

    xTaskCreateStatic(
        bq_test_task,
        "BQ Test",
        BQ_TEST_TASK_STACK_SIZE,
        NULL,
        BQ_TEST_TASK_PRIORITY,
        bq_test_task_stack,
        &bq_test_task_buffer
    );

    vTaskStartScheduler();

    while (true) {
        // Scheduler should've started by now
    }

    return 0;
}
