//toDone

#include "BPS_Tasks.h"
#include "config.h"
#include "CANbus.h"
#include "BPSCAN_can_msgs.h"
#include "StatusLEDs.h"
#include "DebugPrintf.h"

// CAN timeout
#define AMPERES_CAN_TIMEOUT_MS AMPERES_MONITOR_TASK_DELAY_MS
#define AMPERES_WATCHDOG_TIMEOUT_MS 500

// CAN message decoding
#define AMPERES_UNPACK_CURRENT_mA(x) (((int32_t)(((uint32_t)(x)[3] << 24) | ((uint32_t)(x)[2] << 16) | ((uint32_t)(x)[1] << 8))) >> 8)
#define AMPERES_UNPACK_FAULT(x) ((uint8_t)((x[0])))

// Printf period macros
#define AMPERES_LOOP_PRINTF_DELAY_MS 2000
#define AMPERES_PRINTF_COUNTER (AMPERES_LOOP_PRINTF_DELAY_MS / AMPERES_MONITOR_TASK_DELAY_MS)

static TimerHandle_t amperes_watchdog_timer;
static StaticTimer_t amperes_timer_buffer;

static bool recv_amp_data = false;

// Global variable
bps_pack_current_t AmperesData = { 0 };

int32_t get_pack_current(void) {
    return AmperesData.Main_Battery_Current;
}

static void vAmperesWatchdogCallback(TimerHandle_t amps_timer)
{

    if (recv_amp_data == false)
    {
        set_faultBit(AMPERES_WATCHDOG_FAULT);
    }
    else {
        recv_amp_data = false;
    }
}

void Task_Amperes_Monitor() {
    TickType_t xLastWakeTime = xTaskGetTickCount();

    uint32_t amps_printf_debug_counter = 0;
    
    // variable used to keep track if the car is currently charging
    bool is_charging = false;

    // Make timer for watchdog
    amperes_watchdog_timer = xTimerCreateStatic(
        "Amperes Watchdog",                         /* Name of the timer */
        pdMS_TO_TICKS(AMPERES_WATCHDOG_TIMEOUT_MS), /* Timer period in ticks */
        pdTRUE,                                  /* auto-reload */
        (void *)0,                               /* Timer ID */
        vAmperesWatchdogCallback,            /* Callback function */
        &amperes_timer_buffer                       /* Buffer to hold timer data */
    );

    xTimerStart(amperes_watchdog_timer, 0);

    while (1)
    {
        amps_printf_debug_counter++;

        // Delays 100 ms
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(AMPERES_MONITOR_TASK_DELAY_MS));

        // Receive from CAN
        uint8_t buffer[CAN_DLC_BPS_PACK_CURRENT] = { 0 };
        if (bps_can_recv(CAN_ID_BPS_PACK_CURRENT, buffer, CAN_DLC_BPS_PACK_CURRENT, AMPERES_CAN_TIMEOUT_MS) == CAN_OK)
        {
            recv_amp_data = true;
            AmperesData.Main_Battery_Current = AMPERES_UNPACK_CURRENT_mA(buffer);
            AmperesData.BPS_Amperes_Fault = AMPERES_UNPACK_FAULT(buffer);

            // Print current at lower rate
            if (amps_printf_debug_counter >= AMPERES_PRINTF_COUNTER)
            {
                printf("\r\n");
                printf("Pack Current: %li mA\r\n", AmperesData.Main_Battery_Current);
                printf("\r\n");
                amps_printf_debug_counter = 0;
            }
        }

        // Set fault bits if needed. If good, set the event group bit
        if (AmperesData.Main_Battery_Current < OVERCURRENT_CHARGE_THRESHOLD_mA) set_faultBit(PACK_OVERCURRENT_CHARGING_FAULT); 

        else if (AmperesData.Main_Battery_Current > OVERCURRENT_DISCHARGE_THRESHOLD_mA) set_faultBit(PACK_OVERCURRENT_DISCHARGING_FAULT);

        else if (AmperesData.BPS_Amperes_Fault != BPS_PACK_CURRENT_BPS_AMPERES_FAULT_OK) {
            switch (AmperesData.BPS_Amperes_Fault) {
                case BPS_PACK_CURRENT_BPS_AMPERES_FAULT_OUT_OF_BOUNDS:
                    set_faultBit(AMPERES_WATCHDOG_FAULT);
                    break;
                case BPS_PACK_CURRENT_BPS_AMPERES_FAULT_OVER_CURRENT_DISCHARGE_:
                    set_faultBit(PACK_OVERCURRENT_DISCHARGING_FAULT);
                    break;
                case BPS_PACK_CURRENT_BPS_AMPERES_FAULT_OVER_CURRENT_CHARGE_:
                    set_faultBit(PACK_OVERCURRENT_CHARGING_FAULT);
                    break;
                case BPS_PACK_CURRENT_BPS_AMPERES_FAULT_MESSAGE_WATCHDOG:
                    set_faultBit(AMPERES_WATCHDOG_FAULT);
                    break;
            }
        }
        else
        {
            if (get_state_bit(AMPERES_MONITOR_GOOD) != STATE_BIT_SET) {
                set_state_bit(AMPERES_MONITOR_GOOD, STATE_BIT_SET);
            }
        }

        if (AmperesData.Main_Battery_Current > CHARGING_THRESHOLD_MA)  
        {
            if (is_charging) {
                set_state_bit(DISCHARGING_BATT_STATE, STATE_BIT_SET);
                set_state_bit(CHARGING_BATT_STATE, STATE_BIT_RESET);
                LED_set(CHARGING_LED, LED_OFF);
                is_charging = false;
            }
        }
        else
        {
            if (!is_charging) {
                set_state_bit(DISCHARGING_BATT_STATE, STATE_BIT_RESET);
                set_state_bit(CHARGING_BATT_STATE, STATE_BIT_SET);
                LED_set(CHARGING_LED, LED_ON);
                is_charging = true;
            }
        }

        // Set event group bit so watchdog knows we ran
        xEventGroupSetBits(xWDogEventGroup_handle, AMPERES_MONITOR_DONE);
    }
}
