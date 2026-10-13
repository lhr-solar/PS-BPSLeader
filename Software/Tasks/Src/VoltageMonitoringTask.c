// TODO: Volttemps have multiple faults, this does not discriminate. Add discrimination later.
// TODO: CAN QueueSet solution

#include "common.h"
#include "BPS_Tasks.h"
#include "CANbus.h"
#include "StatusLEDs.h"
#include "BPSCAN_can_msgs.h"
#include "CarCAN_can_msgs.h"
#include "string.h"
#include <stdlib.h>

#define VOLTAGE_CAN_DELAY_MS 10u

// a mask of all 1's to compare against the volt sensor watchdog bitmap to ensure every bit is set (all info received)
#define VOLT_TAPS_ALL_DATA 0xFFFFFFFF

#define VOLT_TAPS_PER_BOARD (NUM_VOLTAGE_SENSORS / NUM_VOLTTEMP_BOARDS)

#define VOLT_WATCHDOG_TIMEOUT_MS 1000

// get first five bits of volt can message, which is id
#define VOLT_ID_MASK 0x1F

// Printf period macros
#define VOLT_LOOP_PRINTF_DELAY_MS 2000
#define VOLT_PRINTF_COUNTER (VOLT_LOOP_PRINTF_DELAY_MS / VOLT_MONITOR_TASK_DELAY_MS)

// array to hold struct packed can data
bps_voltage_aggregate_arr_t volt_can_data[NUM_VOLTAGE_SENSORS] = { 0 };

// bitmap to hold volt sensor watchdog, starts all bits set (good), corresponding bits are cleared if taps don't check in
uint32_t volt_watchdog_bitmap = 0;

uint32_t exposed_volt_watchdog_bitmap = VOLT_TAPS_ALL_DATA;

// watchdog timer
static TimerHandle_t voltage_watchdog_timer;
static StaticTimer_t volt_timer_buffer;

// array that is indexed to get the CAN id for each volltemp
static const uint32_t voltage_can_ids[NUM_VOLTTEMP_BOARDS] = {
    CAN_ID_BPS_VT0_VOLTAGE_ARR,
    CAN_ID_BPS_VT1_VOLTAGE_ARR,
    CAN_ID_BPS_VT2_VOLTAGE_ARR,
    CAN_ID_BPS_VT3_VOLTAGE_ARR,
    CAN_ID_BPS_VT4_VOLTAGE_ARR,
    CAN_ID_BPS_VT5_VOLTAGE_ARR,
    CAN_ID_BPS_VT6_VOLTAGE_ARR,
    CAN_ID_BPS_VT7_VOLTAGE_ARR
};

extern bps_pack_current_t AmperesData;

// pass in pointer to raw data, return packed structs
static uint8_t volt_can_unpack(uint8_t* raw_volt_can_data, bps_voltage_aggregate_arr_t* volt_can_data) {

    // this function takes payloads from the bps_vt[X]_voltage_arr_t message, and packs it into bps_voltage_aggregate_arr_t

    static TickType_t s_last_rx_times[NUM_VOLTAGE_SENSORS] = { 0 };

    if (raw_volt_can_data == NULL)
    {
        return 0;
    }

    // byte 0 is the tap ID
    uint8_t tap_index = raw_volt_can_data[0] & VOLT_ID_MASK;

    // tap index is out of bounds
    if (tap_index >= NUM_VOLTAGE_SENSORS)
    {
        return 0;
    }

    volt_can_data[tap_index].BPS_Tap_idx = tap_index;

    // byte 1 - 2 is the actual tap data
    volt_can_data[tap_index].BPS_Voltage_Tap_Data = raw_volt_can_data[1];
    volt_can_data[tap_index].BPS_Voltage_Tap_Data |= ((uint16_t)raw_volt_can_data[2] << 8);

    // byte 3 is the fault
    // we'll preserve whatever fault is set, and then in later logic we'll change it depending on tap voltage
    volt_can_data[tap_index].BPS_Voltage_Tap_Fault = raw_volt_can_data[3];

    TickType_t current_time = xTaskGetTickCount();

    // If this is the very first message received since boot (timestamp is 0)
    if (s_last_rx_times[tap_index] == 0)
    {
        volt_can_data[tap_index].BPS_Voltage_Tap_Age = 0;
    }
    else
    {
        // Calculate the delta: Current Time - Last Arrival Time
        volt_can_data[tap_index].BPS_Voltage_Tap_Age = Calculate_TimeDifference(current_time, s_last_rx_times[tap_index]);
    }

    s_last_rx_times[tap_index] = current_time;

    // set the volt sensor watchdog bitmap
    portENTER_CRITICAL();
    // set corresponding bit in recv bitmap
    volt_watchdog_bitmap |= (1U << tap_index);
    portEXIT_CRITICAL();

    return 1;
}

uint32_t get_module_voltage(uint8_t module_num) {

    // module number is 0 indexed
    if (module_num >= NUM_BATTERY_MODULES) {
        return 69420; // Invalid module number
    }
    return volt_can_data[module_num].BPS_Voltage_Tap_Data;
}

static void volt_can_pack(bps_voltage_aggregate_arr_t volt_can_data, uint8_t* msgArr) {
    if (msgArr == NULL)
    {
        return;
    }

    // bits [0:4]: Tap index (Byte 0)
    msgArr[0] = (volt_can_data.BPS_Tap_idx & VOLT_ID_MASK);

    // bits [8:23]: Voltage data (Bytes 1-2)
    // Using sizeof(uint16_t) ensures 2 bytes are copied
    memcpy(&msgArr[1], &(volt_can_data.BPS_Voltage_Tap_Data), sizeof(uint16_t));

    // bits [24:31]: Fault data (Byte 3)
    msgArr[3] = volt_can_data.BPS_Voltage_Tap_Fault;

    // bits [32:47]: Tap age (Bytes 4-5)
    // Note: The previous implementation used msgArr[4],
    // but the table confirms bits 32-47 occupy bytes 4 and 5.
    memcpy(&msgArr[4], &(volt_can_data.BPS_Voltage_Tap_Age), sizeof(uint16_t));
}

// gets all can data from each tap from a passed in volttemp board, unpacks it and puts it into array
static void can_recv_all_taps(uint32_t can_id_index, bps_voltage_aggregate_arr_t volt_can_data[]) {

    // can recieve for all 4 voltage taps for each volttemp board
    for (uint8_t i = 0; i < VOLT_TAPS_PER_BOARD; i++)
    {

        uint8_t raw_databuffer[CAN_DLC_BPS_VT0_VOLTAGE_ARR] = { 0 };

        // if can recv fails, set the fault bit of the struct on to indicate that this sensor isnt working
        if (bps_can_recv(voltage_can_ids[can_id_index], raw_databuffer, CAN_DLC_BPS_VT0_VOLTAGE_ARR, VOLTAGE_CAN_DELAY_MS) == CAN_OK)
        {
            // unpack the BPS voltage message from BPS CAN to the BPS aggregate array message
            volt_can_unpack(raw_databuffer, volt_can_data);
        }
    }
}

// watchdog function that runs when the timer times out
static void vVoltageWatchdogCallback(TimerHandle_t volt_timer) {
    taskENTER_CRITICAL();
    // check if every tap has sent voltage information since last timer timeout.
    if (volt_watchdog_bitmap != VOLT_TAPS_ALL_DATA)
    {
        // if one hasn't sent, save bitmap to know which one(s) didn't check in, then set fault bit
        exposed_volt_watchdog_bitmap = volt_watchdog_bitmap;
        latch_mod_fault(get_mod_fault_num(exposed_volt_watchdog_bitmap), 0); // Store 0 as the faulted module value since voltage isn't being stored here
        set_faultBit(VOLTTEMP_WATCHDOG_FAULT);
    }
    volt_watchdog_bitmap = 0;
    taskEXIT_CRITICAL();
}

uint32_t get_pack_voltage() {

    uint32_t voltage_sum = 0;

    for (uint8_t module_num = 0; module_num < NUM_BATTERY_MODULES; module_num++)
    {
        voltage_sum += volt_can_data[module_num].BPS_Voltage_Tap_Data;
    }

    return voltage_sum;
}

bool get_volt_segment_status(uint8_t segment_num) {

    // confirm voltage readings are coming in
    if (((exposed_volt_watchdog_bitmap >> (segment_num * MODULES_PER_SEGMENT)) & 0xF) != 0xF)
    {
        return false;
    }

    // make sure no faults
    for (uint8_t tap_num = segment_num * MODULES_PER_SEGMENT; tap_num < (segment_num + 1) * MODULES_PER_SEGMENT; tap_num++)
    {

        if (volt_can_data[tap_num].BPS_Voltage_Tap_Fault != BPS_VOLTAGE_AGGREGATE_ARR_BPS_VOLTAGE_TAP_FAULT_OK)
        {
            return false;
        }
    }
    return true;
}

void Task_Voltage_Monitor() {

    // counter to slow printf messages
    uint32_t volt_printf_debug_counter = 0;

    // Make timer for watchdog
    voltage_watchdog_timer = xTimerCreateStatic(
        "Volt Watchdog",                         /* Name of the timer */
        pdMS_TO_TICKS(VOLT_WATCHDOG_TIMEOUT_MS), /* Timer period in ticks */
        pdTRUE,                                  /* auto-reload */
        (void*)0,                               /* Timer ID */
        vVoltageWatchdogCallback,                /* Callback function */
        &volt_timer_buffer                       /* Buffer to hold timer data */
    );

    xTimerStart(voltage_watchdog_timer, 0);

    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1)
    {
        volt_printf_debug_counter++;

        // Delay
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(VOLT_MONITOR_TASK_DELAY_MS));

        // loops through each can ID
        for (uint8_t can_id_index = 0; can_id_index < NUM_VOLTTEMP_BOARDS; can_id_index++)
        {

            // recieve data from all taps from each id
            can_recv_all_taps(can_id_index, volt_can_data);
        }

        // message buffer to hold forward voltage aggregate array can msg
        uint8_t msgBuff[CAN_DLC_BPS_VOLTAGE_AGGREGATE_ARR] = { 0 };

        // flag to determine if voltage is OK (to set state bit)
        bool all_voltage_good = true;

        // max voltage counter used for bounds checking
        uint32_t max_voltage = 0;

        // Loop through every received value
        for (uint8_t i = 0; i < NUM_VOLTAGE_SENSORS; i++)
        {
            // update max voltage
            if (volt_can_data[i].BPS_Voltage_Tap_Data > max_voltage)
            {
                max_voltage = volt_can_data[i].BPS_Voltage_Tap_Data;
            }

            // if voltage is too high or too low, set relevant fault and set fault bit
            if (volt_can_data[i].BPS_Voltage_Tap_Data > CELL_OVERVOLTAGE_THRESHOLD_MV && volt_can_data[i].BPS_Tap_idx != 1 && volt_can_data[i].BPS_Tap_idx != 5)
            {
                volt_can_data[i].BPS_Voltage_Tap_Fault = BPS_VOLTAGE_AGGREGATE_ARR_BPS_VOLTAGE_TAP_FAULT_OVER_VOLTAGE;
                printf("Entering Cell Over Voltage Fault for Tap %d: %dmV\r\n", volt_can_data[i].BPS_Tap_idx, volt_can_data[i].BPS_Voltage_Tap_Data);
                latch_mod_fault(volt_can_data[i].BPS_Tap_idx, volt_can_data[i].BPS_Voltage_Tap_Data); // Store the faulted module value (voltage)
                set_faultBit(CELL_OVERVOLTAGE_FAULT);
                all_voltage_good = false;
            }
            else if (volt_can_data[i].BPS_Voltage_Tap_Data < CELL_UNDERVOLTAGE_THRESHOLD_MV - (2.1 * 0.9 * abs(AmperesData.Main_Battery_Current)) / 1000)
            {

                volt_can_data[i].BPS_Voltage_Tap_Fault = BPS_VOLTAGE_AGGREGATE_ARR_BPS_VOLTAGE_TAP_FAULT_UNDER_VOLTAGE;
                printf("Entering Cell Under Voltage Fault for Tap %d: %dmV\r\n", volt_can_data[i].BPS_Tap_idx, volt_can_data[i].BPS_Voltage_Tap_Data);
                latch_mod_fault(volt_can_data[i].BPS_Tap_idx, volt_can_data[i].BPS_Voltage_Tap_Data); // Store the faulted module value (voltage)
                set_faultBit(CELL_UNDERVOLTAGE_FAULT);
                all_voltage_good = false;
            }
            // pack data for the  msg
            volt_can_pack(volt_can_data[i], msgBuff);
            car_can_send(CAN_ID_BPS_VOLTAGE_AGGREGATE_ARR, msgBuff, CAN_DLC_BPS_VOLTAGE_AGGREGATE_ARR, pdMS_TO_TICKS(VOLTAGE_CAN_DELAY_MS));
        }

        if (volt_printf_debug_counter >= VOLT_PRINTF_COUNTER)
        {
            printf("================================\r\n");
            printf("Voltage values: {");
            for (int j = 0; j < NUM_BATTERY_MODULES; j++)
            {
                printf("%d: %u.%03u V, ", j, volt_can_data[j].BPS_Voltage_Tap_Data / 1000, volt_can_data[j].BPS_Voltage_Tap_Data % 1000);
            }
            printf("}\r\n");
            printf("================================\r\n");


            volt_printf_debug_counter = 0;
        }

        // check if voltage is OK for charging
        if ((max_voltage < CELL_CHARGING_VOLTAGE_THRESHOLD_MV) && (get_state_bit(VOLT_OK_FOR_CHARGING) != STATE_BIT_SET))
        {
            if (get_state_bit(VOLT_OK_FOR_CHARGING) == STATE_BIT_RESET) {
                printf("Cell Voltages are OK for charging\r\n");
            }
            set_state_bit(VOLT_OK_FOR_CHARGING, STATE_BIT_SET);
        }
        else if (((max_voltage >= CELL_CHARGING_VOLTAGE_THRESHOLD_MV) && (get_state_bit(VOLT_OK_FOR_CHARGING) != STATE_BIT_RESET)))
        {
            if (get_state_bit(VOLT_OK_FOR_CHARGING) == STATE_BIT_SET) {
                printf("Cell Voltages are NOT ok for charging\r\n");
            }
            set_state_bit(VOLT_OK_FOR_CHARGING, STATE_BIT_RESET);
        }

        if (all_voltage_good && (get_state_bit(VOLTAGE_MONITOR_GOOD) != STATE_BIT_SET))
        {
            if (get_state_bit(VOLT_OK_FOR_CHARGING) == STATE_BIT_SET) {
                printf("All module voltages checked and safe\r\n");
            }
            set_state_bit(VOLTAGE_MONITOR_GOOD, STATE_BIT_SET);
        }

        // Set event group bit
        xEventGroupSetBits(xWDogEventGroup_handle, /* The event group being updated. */
            VOLT_MONITOR_DONE);     /* The bits being set. */
    }
}
