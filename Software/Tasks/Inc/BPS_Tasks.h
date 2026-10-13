#pragma once

#include "common.h"
#include "BPSCAN_can_msgs.h"
#include "CarCAN_can_msgs.h"
#include "faultHandler.h"
#include <event_groups.h>

// Task configuration
#define TASK_INIT_PRIO                  tskIDLE_PRIORITY + 2
#define TASK_TEMPERATURE_MONITOR_PRIO   tskIDLE_PRIORITY + 3
#define TASK_VOLTAGE_MONITOR_PRIO       tskIDLE_PRIORITY + 3
#define TASK_AMPERES_MONITOR_PRIO       tskIDLE_PRIORITY + 3
#define TASK_PETWDOG_PRIO               tskIDLE_PRIORITY + 1
#define TASK_CAN_FORWARD_PRIO           tskIDLE_PRIORITY + 2
#define TASK_FAULT_HANDLER_PRIO         tskIDLE_PRIORITY + 5
#define TASK_CONTACTOR_MONITOR_PRIO     tskIDLE_PRIORITY + 4
#define TASK_FAN_CONTROLLER_PRIO        tskIDLE_PRIORITY + 3
#define TASK_CAN_STATUS_PRIO            tskIDLE_PRIORITY + 4
#define TASK_PRECHARGE_PRIO             tskIDLE_PRIORITY + 3

#define TEST_TASK_PRIORITY              tskIDLE_PRIORITY + 3

// Task Stack Size 
#define TASK_INIT_STACK_SIZE                     (configMINIMAL_STACK_SIZE*2)
#define TASK_TEMPERATURE_MONITOR_STACK_SIZE      (configMINIMAL_STACK_SIZE*2)
#define TASK_VOLTAGE_MONITOR_STACK_SIZE          (configMINIMAL_STACK_SIZE*2)
#define TASK_AMPERES_MONITOR_STACK_SIZE          (configMINIMAL_STACK_SIZE*2)
#define TASK_PETWDOG_STACK_SIZE                  (configMINIMAL_STACK_SIZE*2)
#define PRECHARGE_TASK_STACK_SIZE                (configMINIMAL_STACK_SIZE*2)
#define FAULT_HANDLER_TASK_STACK_SIZE            (configMINIMAL_STACK_SIZE*2)
#define TASK_CAN_FORWARD_STACK_SIZE              (configMINIMAL_STACK_SIZE*4)
#define TASK_CONTACTOR_MONITORING_STACK_SIZE     (configMINIMAL_STACK_SIZE*2)
#define TASK_FAN_CONTROLLER_STACK_SIZE           (configMINIMAL_STACK_SIZE*2)
#define TASK_CAN_STATUS_STACK_SIZE               (configMINIMAL_STACK_SIZE*2)

#define TEST_TASK_STACK_SIZE                     (configMINIMAL_STACK_SIZE*2)

// (exposed so that tests can init tasks)
// Task Stack Arrays 
extern StackType_t Task_Temperature_Stack_Array[ TASK_TEMPERATURE_MONITOR_STACK_SIZE ];
extern StackType_t Task_Voltage_Stack_Array[ TASK_VOLTAGE_MONITOR_STACK_SIZE ];
extern StackType_t Task_Amperes_Stack_Array[ TASK_AMPERES_MONITOR_STACK_SIZE ];
extern StackType_t Task_Petwdog_Stack_Array[ TASK_PETWDOG_STACK_SIZE ];
extern StackType_t Precharge_Task_Stack[ PRECHARGE_TASK_STACK_SIZE ];
extern StackType_t FaultHandler_Task_Stack[ FAULT_HANDLER_TASK_STACK_SIZE ];
extern StackType_t Task_Can_Forward_Stack[ TASK_CAN_FORWARD_STACK_SIZE ];
extern StackType_t Task_Contactor_Monitoring_Stack[ TASK_CONTACTOR_MONITORING_STACK_SIZE ];
extern StackType_t Init_Task_Stack[ TASK_INIT_STACK_SIZE ];
extern StackType_t Task_Fan_Controller_Stack[ TASK_FAN_CONTROLLER_STACK_SIZE ];
extern StackType_t Task_Can_Status_Stack[ TASK_CAN_STATUS_STACK_SIZE ];

// Task Buffers
extern StaticTask_t Task_Temperature_Buffer;
extern StaticTask_t Task_Voltage_Buffer;
extern StaticTask_t Task_Amperes_Buffer;
extern StaticTask_t Task_Petwdog_Buffer;
extern StaticTask_t Precharge_Task_Buffer;
extern StaticTask_t FaultHandler_Task_Buffer;
extern StaticTask_t Task_Can_Forward_Buffer;
extern StaticTask_t Task_Contactor_Monitoring_Buffer;
extern StaticTask_t Init_Task_Buffer;
extern StaticTask_t Task_Fan_Controller_Buffer;
extern StaticTask_t Task_Can_Status_Buffer;

// Task Delays
#define TEMP_MONITOR_TASK_DELAY_MS      290
#define VOLT_MONITOR_TASK_DELAY_MS      290
#define PRECHARGE_TASK_DELAY_MS         200
#define CONTACTOR_MONITOR_TASK_DELAY_MS 200
#define AMPERES_MONITOR_TASK_DELAY_MS   90
#define FAN_CONTROLLER_TASK_DELAY_MS    300
#define CAN_STATUS_TASK_DELAY_MS        500

// Task Inits
void Task_Init();
void Task_Voltage_Monitor();
void Task_Temperature_Monitor();
void Task_FaultHandler();
void Task_Fan_Controller();
void Task_Amperes_Monitor();
void Task_PetWatchdog();
void Task_CanRxForward();
void Task_Contactor_Monitor();
void Task_Can_Status();
void Task_Precharge();

// access functions for 
// returns the average temperature of all cells in the battery in mC
uint32_t get_avg_temp();

// returns sum of all voltage tap measurements of pack, which is pack voltage
uint32_t get_pack_voltage();

// returns a specific module's voltage in mV, module number is 0 indexed
uint32_t get_module_voltage(uint8_t module_num);

// returns the pack current in mA, positive for charging, negative for discharging
int32_t get_pack_current(void);

// returns a specific module's temperature in mC, module number is 0 indexed
uint32_t get_module_temperature(uint8_t module_num);

// takes a segment number, returns TRUE if segment is OK (sending temperature information, no errors) else returns FALSE
bool get_temp_segment_status(uint8_t segment_num);

// takes a segment number, returns TRUE if segment is OK (sending voltage information, no errors) else returns FALSE
bool get_volt_segment_status(uint8_t segment_num);

// macros to find which module faulted (checks to see what bit in bitmap isn't set, returns its index. if all is good then it returns -1 ) 
#define get_mod_fault_num(mod_bitmap) (((~(mod_bitmap)) != 0) ? __builtin_ctz(~(mod_bitmap)) : -1)

// function to get the value of the faulted module (temperature or voltage)
extern uint32_t mod_fault_value;

static inline uint32_t get_mod_fault_value(void)
{
    return mod_fault_value;
}

/* ---- Watchdog Event Group ---- */
void Init_WDogTask();
extern EventGroupHandle_t xWDogEventGroup_handle;
#define TEMP_MONITOR_DONE   (1 << 0)
#define VOLT_MONITOR_DONE   (1 << 1)
#define WINDOW_TIMER_DONE   (1 << 2)
#define AMPERES_MONITOR_DONE (1 << 3)
#define CONTACTOR_MONITOR_DONE (1 << 4)
#define PRECHARGE_MONITOR_DONE (1 << 5)

#define ALL_TASKS_DONE (TEMP_MONITOR_DONE | VOLT_MONITOR_DONE | WINDOW_TIMER_DONE | AMPERES_MONITOR_DONE | CONTACTOR_MONITOR_DONE | PRECHARGE_MONITOR_DONE)

// latching intereger to keep track of which module faulted. bit 5 is used to latch
extern uint8_t mod_fault_num;

extern uint32_t exposed_temperature_watchdog_bitmap;
extern uint32_t exposed_volt_watchdog_bitmap;

// bit that is set in mod fault bitmap to indicate a module has faulted
#define MOD_FAULT_BITMAP_LATCH (1 << 5)

// that sets mod fault only if it hasn't been previously set
static inline void latch_mod_fault(uint8_t mod_fault_num_, uint32_t faulted_module_value) {
    // Sanity check: ensure the module number doesn't overflow into the latch bit
    if (mod_fault_num_ >= (NUM_BATTERY_MODULES)){
         return; 
    }

    // Suspend interrupts/scheduler to ensure atomicity
    taskENTER_CRITICAL(); 

    mod_fault_value = faulted_module_value; // store the value of the faulted module (temperature or voltage)
    
    // Check if the latch bit is NOT set
    if ((mod_fault_num & MOD_FAULT_BITMAP_LATCH) == 0) {
        // Use Bitwise OR (|) to combine the latch bit and the module number
         mod_fault_num = mod_fault_num_ | MOD_FAULT_BITMAP_LATCH;
    }
    
    // Resume interrupts/scheduler
    taskEXIT_CRITICAL(); 
}

// State bit event group to keep track of BPS states
extern EventGroupHandle_t xStateBits;
extern StaticEventGroup_t xStateBits_buffer;

#define get_state_bit(bit) ((xEventGroupGetBits(xStateBits) & (1U << bit)) >> bit)

#define clear_state_bit(bit) (xEventGroupClearBits(xStateBits, (1U << bit)))

#define set_state_bit(bit, state) ((state) ? (xEventGroupSetBits(xStateBits, (1U << bit))) : (clear_state_bit(bit))) 

#define STATE_BIT_SET (1)
#define STATE_BIT_RESET (0)

#define MAX_STATE_BITS (24)

typedef enum {

    // tracks if battery is charging or discharging
    DISCHARGING_BATT_STATE,
    CHARGING_BATT_STATE,

    // tracks if each task has completed at least one cycle at startup
    AMPERES_MONITOR_GOOD,
    VOLTAGE_MONITOR_GOOD,
    TEMPERATURE_MONITOR_GOOD,
    CONTACTOR_MONITOR_GOOD,

    // tracks if we have temp/voltage within charging thresholds
    TEMP_OK_FOR_CHARGING,
    VOLT_OK_FOR_CHARGING,

    NUM_STATE_BITS
} state_bits_t;

static_assert(NUM_STATE_BITS <= MAX_STATE_BITS, "Too many state bits!!");








