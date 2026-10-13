#include "faultHandler.h"
#include "Contactors.h"
#include "EMC2305_Driver.h"
#include "StatusLEDs.h"

bool fault_bits_initialized = false;

bool system_has_faulted = false;
volatile uint32_t first_fault_id = 0;
static StaticSemaphore_t xFaultSemaphoreBuffer;

const uint8_t fault_bit_arr_size = (1 + ((NUM_FAULTS - 1) / MAX_FAULT_BITS));

// Event group array handles to store fault state bits (split into fault bit buffers)
EventGroupHandle_t faultBits[FAULT_BIT_ARR_SIZE_MACRO];

uint8_t mod_fault_num = 0;
uint32_t mod_fault_value = 0;

// Static buffer arrary to store the event handle
static StaticEventGroup_t faultBitsBuffer[FAULT_BIT_ARR_SIZE_MACRO];

SemaphoreHandle_t faultSemaphore = NULL;

uint8_t faultHandler_init(void)
{

    if (fault_bits_initialized)
        return 1;

    for (uint16_t i = 0; i < fault_bit_arr_size; i++)
    {
        faultBits[i] = xEventGroupCreateStatic(&faultBitsBuffer[i]);

        if (faultBits[i] == NULL)
            return 0;
    }

    faultSemaphore = xSemaphoreCreateBinaryStatic(&xFaultSemaphoreBuffer);

    if (faultSemaphore == NULL)
    {
        return 0;
    }

    fault_bits_initialized = true;
    return 1;
}

const char *const fault_bit_strings[] = {
    // BPS main safety loop faults
    [BPS_FAULT] = "BPS_FAULT",
    [CELL_OVERVOLTAGE_FAULT] = "CELL_OVERVOLTAGE_FAULT",
    [CELL_UNDERVOLTAGE_FAULT] = "CELL_UNDERVOLTAGE_FAULT",
    [BQ_CHIP_FAULT] = "BQ_CHIP_FAULT",
    [CELL_OVERTEMP_FAULT] = "CELL_OVERTEMP_FAULT",
    [PACK_OVERCURRENT_CHARGING_FAULT] = "PACK_OVERCURRENT_CHARGING_FAULT",
    [PACK_OVERCURRENT_DISCHARGING_FAULT] = "PACK_OVERCURRENT_DISCHARGING_FAULT",
    [CONTACTOR_HV_PLUS_FAULT] = "CONTACTOR_HV_PLUS_FAULT",
    [CONTACTOR_HV_MINUS_FAULT] = "CONTACTOR_HV_MINUS_FAULT",
    [CONTACTOR_ARRAY_FAULT] = "CONTACTOR_ARRAY_FAULT",
    [CONTACTOR_ARRAY_PRE_FAULT] = "CONTACTOR_ARRAY_PRE_FAULT",

    [CONTACTOR_CALLBACK_FAULT] = "CONTACTOR_CALLBACK_FAULT",
    [AMPERES_WATCHDOG_FAULT] = "AMPERES_WATCHDOG_FAULT",
    [VOLTTEMP_WATCHDOG_FAULT] = "VOLTTEMP_WATCHDOG_FAULT",
    [BPS_ESTOP1_FAULT] = "BPS_ESTOP1_FAULT",
    [BPS_ESTOP2_FAULT] = "BPS_ESTOP2_FAULT",
    [BPS_ESTOP3_FAULT] = "BPS_ESTOP3_FAULT",

    // Precharge faults
    [ARRAY_GREATER_THAN_BATTERY_FAULT] = "ARRAY_GREATER_THAN_BATTERY_FAULT",
    [PRECHARGE_TIMEOUT_FAULT] = "PRECHARGE_TIMEOUT_FAULT",
    [PRECHARGE_OUT_OF_BOUNDS_FAULT] = "PRECHARGE_OUT_OF_BOUNDS_FAULT",
    [PACK_OVERVOLTAGE_FAULT] = "PACK_OVERVOLTAGE_FAULT",
    [PACK_UNDERVOLTAGE_FAULT] = "PACK_UNDERVOLTAGE_FAULT",
    [FAN_TACHOMETER_FAULT] = "FAN_TACHOMETER_FAULT",

    // Software Errors
    [RTOS_WATCHDOG_ERROR] = "RTOS_WATCHDOG_ERROR",
    [BPS_CAN_ERROR] = "BPS_CAN_ERROR",
    [CAR_CAN_ERROR] = "CAR_CAN_ERROR",
    [FAN_CHIP_ERROR] = "FAN_CHIP_ERROR",
    [ELCON_FAULT] = "ELCON_FAULT",
    [REGEN_FAULT] = "REGEN_FAULT",
    [ADC_ERROR] = "ADC_ERROR",
    [SHT45_ERROR] = "SHT45_ERROR",
    [I2C_ERROR] = "I2C_ERROR"
};

static_assert((sizeof(fault_bit_strings) / sizeof(fault_bit_strings[0])) == NUM_FAULTS, "String array elements do not match fault enum!");

uint32_t faultBit_wait(fault_bit_t bit, TickType_t xTicksToWait)
{

    // NUM_FAULTS indiciates you want to wait for all bits
    if (bit > NUM_FAULTS)
    {
        return 0;
    }

    // EventBits_t uxBitsToWaitFor = bit == NUM_FAULTS ?     ALL_FAULT_BITS : (FAULT_BIT(bit));
    if (xSemaphoreTake(faultSemaphore, xTicksToWait) == pdTRUE) {}

    return first_fault_id;
}

// Print faults & set relevant LEDs
void handle_fault(uint32_t fault_bit_index)
{
    // fault printing
    printf("================================\r\n");
    printf("FAULT: %s\r\n", fault_bit_strings[fault_bit_index]);
    printf("================================\r\n");


    // check if a module has faulted, if so, set relevant LED
    if (mod_fault_num & MOD_FAULT_BITMAP_LATCH) {
        LEDsModFaultBitmap_set(mod_fault_num);
    }
    // fault handling
    switch (fault_bit_index)
    {
        case CELL_OVERVOLTAGE_FAULT:
            printf("Cell OV Module: %d, Faulted Voltage: %ldmV, Current Voltage: %ldmV\r\n", mod_fault_num & ~MOD_FAULT_BITMAP_LATCH, mod_fault_value, get_module_voltage(mod_fault_num & ~MOD_FAULT_BITMAP_LATCH));
            LED_set(OVER_V_LED, LED_ON);
            break;

        case PACK_OVERVOLTAGE_FAULT:
            printf("Pack OV Fault: %ldmV\r\n", get_pack_voltage());
            LED_set(OVER_V_LED, LED_ON);
            break;

        case PACK_UNDERVOLTAGE_FAULT:
            printf("Pack UV Fault: %ldmV\r\n", get_pack_voltage());
            LED_set(LOW_V_LED, LED_ON);
            break;

        case CELL_UNDERVOLTAGE_FAULT:
            printf("Cell UV Module: %d, Faulted Voltage: %ldmV, Current Voltage: %ldmV\r\n", mod_fault_num & ~MOD_FAULT_BITMAP_LATCH, mod_fault_value, get_module_voltage(mod_fault_num & ~MOD_FAULT_BITMAP_LATCH));
            LED_set(LOW_V_LED, LED_ON);
            break;

        case PACK_OVERCURRENT_CHARGING_FAULT:
            printf("Pack Overcurrent Charging Fault: %ldmA\r\n", get_pack_current());
            LED_set(OVER_AMP_LED, LED_ON);
            break;

        case PACK_OVERCURRENT_DISCHARGING_FAULT:
            printf("Pack Overcurrent Discharging Fault: %ldmA\r\n", get_pack_current());
            LED_set(OVER_AMP_LED, LED_ON);
            break;

        case CELL_OVERTEMP_FAULT:
            printf("Cell OT Module: %d, Faulted Temperature: %ldmC, Current Temperature: %ldmC\r\n", mod_fault_num & ~MOD_FAULT_BITMAP_LATCH, mod_fault_value, get_module_temperature(mod_fault_num & ~MOD_FAULT_BITMAP_LATCH));
            LED_set(OVER_TEMP_LED, LED_ON);
            break;

        case RTOS_WATCHDOG_ERROR:
            LED_set(WATCHDOG_ERR_LED, LED_ON);
            break;
        
        case CONTACTOR_CALLBACK_FAULT:
            printf("Contactors With Sense Faults: ");
            for(uint8_t i = 0; i < NUM_CONTACTORS; i++){
                if(contactor_get_faulted_status(i)){
                    printf("%s, ", CONTACTOR_NAMES[i]);
                }
            }
            printf("\r\n");
            break;

        case BPS_ESTOP3_FAULT:
            printf("Battery ESTOP Pressed\r\n");
            break;

        default:
            break;
    }
}

bool is_fault_set(uint32_t bit_index)
{   

    // check for all faultts if bit_index == NUM_FAULTS
    if (bit_index == NUM_FAULTS)
    {
        for (uint16_t i = 0; i < fault_bit_arr_size; i++) {
            if (xEventGroupGetBits(faultBits[i]) != 0) {
                return true;
            }
        }
        return false;
    }
    else
    {
        // Select the correct group
        EventGroupHandle_t target_group = get_target_group(bit_index);

        // Get all bits from that group
        EventBits_t all_bits = xEventGroupGetBits(target_group);

        // Mask for the specific bit
        // Using your FAULT_BIT macro and the modulo for the second group offset
        return (all_bits & FAULT_BIT(bit_index % MAX_FAULT_BITS)) != 0;
    }
}

