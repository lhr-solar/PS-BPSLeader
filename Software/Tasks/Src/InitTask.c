#include "common.h"
#include "BPS_Tasks.h"
#include "EMC2305_Driver.h"
#include "CANbus.h"
#include "StatusLEDs.h"
#include "Contactors.h"
#include "DebugPrintf.h"
#include "SHT45.h"

#define ALL_TASK_BITS ((1 << AMPERES_MONITOR_GOOD) | (1 << CONTACTOR_MONITOR_GOOD) | (1 << VOLTAGE_MONITOR_GOOD) | (1 << TEMPERATURE_MONITOR_GOOD))
#define STARTUP_DELAY_MS (1000)

// Task Stack Arrays
StackType_t Task_Temperature_Stack_Array[TASK_TEMPERATURE_MONITOR_STACK_SIZE];
StackType_t FaultHandler_Task_Stack[FAULT_HANDLER_TASK_STACK_SIZE];
StackType_t Precharge_Task_Stack[PRECHARGE_TASK_STACK_SIZE];
StackType_t Task_Voltage_Stack_Array[TASK_VOLTAGE_MONITOR_STACK_SIZE];
StackType_t Task_Amperes_Stack_Array[TASK_AMPERES_MONITOR_STACK_SIZE];
StackType_t Task_Petwdog_Stack_Array[TASK_PETWDOG_STACK_SIZE];
StackType_t Task_Contactor_Monitor_Stack[TASK_CONTACTOR_MONITORING_STACK_SIZE];
StackType_t Init_Task_Stack[TASK_INIT_STACK_SIZE];
StackType_t Task_Can_Forward_Stack[TASK_CAN_FORWARD_STACK_SIZE];
StackType_t Task_Fan_Controller_Stack[TASK_FAN_CONTROLLER_STACK_SIZE];
StackType_t Task_Can_Status_Stack[TASK_CAN_STATUS_STACK_SIZE];

// Task Buffer
StaticTask_t Init_Task_Buffer;
StaticTask_t FaultHandler_Task_Buffer;
StaticTask_t Task_Temperature_Buffer;
StaticTask_t Task_Voltage_Buffer;
StaticTask_t Task_Amperes_Buffer;
StaticTask_t Task_Petwdog_Buffer;
StaticTask_t Precharge_Task_Buffer;
StaticTask_t Task_Contactor_Monitor_Buffer;
StaticTask_t Task_Can_Forward_Buffer;
StaticTask_t Task_Fan_Controller_Buffer;
StaticTask_t Task_Can_Status_Buffer;

// Event Group
EventGroupHandle_t xWDogEventGroup_handle;

EventGroupHandle_t xStateBits;
StaticEventGroup_t xStateBits_buffer;

void Task_Init()
{

    // vTaskDelay(pdMS_TO_TICKS(STARTUP_DELAY_MS));

    if (faultHandler_init() == 0)
    {
        Error_Handler();
    }

    xStateBits = xEventGroupCreateStatic(&xStateBits_buffer);

    if (xStateBits == NULL) {
        Error_Handler();
    }

    LEDs_init();

    debugPrintf_init();

    CAN_Init();

    contactor_init();

    SHT45_init();

    EMC2305_I2C_init();
    EMC2305_Driver_init();

    Init_WDogTask();

    printf("Initialized...\n\r");

    xTaskCreateStatic(
        Task_FaultHandler,             // Task function
        "FaultHandler",                // Name of the task (for debugging)
        FAULT_HANDLER_TASK_STACK_SIZE, // Stack size in words
        (void *)NULL,                  // Task input parameter
        TASK_FAULT_HANDLER_PRIO,       // Task priority
        FaultHandler_Task_Stack,       // Task handle
        &FaultHandler_Task_Buffer      // Static task buffer (optional)
    );

    xTaskCreateStatic(
        Task_Amperes_Monitor,            /* The function that implements the task. */
        "Amperes Monitor Task",          /* Text name for the task. */
        TASK_AMPERES_MONITOR_STACK_SIZE, /* The size (in words) of the stack that should be created for the task. */
        (void *)NULL,                    /* Paramter passed into the task. */
        TASK_AMPERES_MONITOR_PRIO,       /* Task Prioriy. */
        Task_Amperes_Stack_Array,        /* Stack array. */
        &Task_Amperes_Buffer             /* Buffer for static allocation. */
    );

    xTaskCreateStatic(
        Task_Fan_Controller,            /* The function that implements the task. */
        "Fan Controller Task",          /* Text name for the task. */
        TASK_FAN_CONTROLLER_STACK_SIZE, /* The size (in words) of the stack that should be created for the task. */
        (void *)NULL,                   /* Paramter passed into the task. */
        TASK_FAN_CONTROLLER_PRIO,       /* Task Prioriy. */
        Task_Fan_Controller_Stack,      /* Stack array. */
        &Task_Fan_Controller_Buffer     /* Buffer for static allocation. */
    );

    xTaskCreateStatic(
        Task_CanRxForward,           /* The function that implements the task. */
        "CAN Forward Task",          /* Text name for the task. */
        TASK_CAN_FORWARD_STACK_SIZE, /* The size (in words) of the stack that should be created for the task. */
        (void *)NULL,                /* Paramter passed into the task. */
        TASK_CAN_FORWARD_PRIO,       /* Task Prioriy. */
        Task_Can_Forward_Stack,      /* Stack array. */
        &Task_Can_Forward_Buffer     /* Buffer for static allocation. */
    );

    xTaskCreateStatic(
        Task_Temperature_Monitor,            /* The function that implements the task. */
        "Temperature Monitor Task",          /* Text name for the task. */
        TASK_TEMPERATURE_MONITOR_STACK_SIZE, /* The size (in words) of the stack that should be created for the task. */
        (void *)NULL,                        /* Paramter passed into the task. */
        TASK_TEMPERATURE_MONITOR_PRIO,       /* Task Prioriy. */
        Task_Temperature_Stack_Array,        /* Stack array. */
        &Task_Temperature_Buffer             /* Buffer for static allocation. */
    );

    xTaskCreateStatic(
        Task_Contactor_Monitor,               /* The function that implements the task. */
        "Cotnactor State-checking Task",      /* Text name for the task. */
        TASK_CONTACTOR_MONITORING_STACK_SIZE, /* The size (in words) of the stack that should be created for the task. */
        (void *)NULL,                         /* Paramter passed into the task. */
        TASK_CONTACTOR_MONITOR_PRIO,          /* Task Prioriy. */
        Task_Contactor_Monitor_Stack,         /* Stack array. */
        &Task_Contactor_Monitor_Buffer        /* Buffer for static allocation. */
    );

    xTaskCreateStatic(
        Task_Voltage_Monitor,            /* The function that implements the task. */
        "Voltage Monitor Task",          /* Text name for the task. */
        TASK_VOLTAGE_MONITOR_STACK_SIZE, /* The size (in words) of the stack that should be created for the task. */
        (void *)NULL,                    /* Paramter passed into the task. */
        TASK_VOLTAGE_MONITOR_PRIO,       /* Task Prioriy. */
        Task_Voltage_Stack_Array,        /* Stack array. */
        &Task_Voltage_Buffer             /* Buffer for static allocation. */
    );

    xTaskCreateStatic(
        Task_Can_Status,            /* The function that implements the task. */
        "CAN Status Sending Task",  /* Text name for the task. */
        TASK_CAN_STATUS_STACK_SIZE, /* The size (in words) of the stack that should be created for the task. */
        (void *)NULL,               /* Paramter passed into the task. */
        TASK_CAN_STATUS_PRIO,       /* Task Prioriy. */
        Task_Can_Status_Stack,   /* Stack array. */
        &Task_Can_Status_Buffer        /* Buffer for static allocation. */
    );

        xTaskCreateStatic(
        Task_Precharge,             /* The function that implements the task. */
        "Precharging Task",         /* Text name for the task. */
        PRECHARGE_TASK_STACK_SIZE,  /* The size (in words) of the stack that should be created for the task. */
        (void *)NULL,               /* Paramter passed into the task. */
        TASK_PRECHARGE_PRIO,        /* Task Prioriy. */
        Precharge_Task_Stack,       /* Stack array. */
        &Precharge_Task_Buffer      /* Buffer for static allocation. */
    );

    // xTaskCreateStatic(
    //     Task_PetWatchdog,                   /* The function that implements the task. */
    //     "PetWatchdog",                      /* Text name for the task. */
    //     TASK_PETWDOG_STACK_SIZE,            /* The size (in words) of the stack that should be created for the task. */
    //     (void*)NULL,                        /* Paramter passed into the task. */
    //     TASK_PETWDOG_PRIO,                  /* Task Prioriy. */
    //     Task_Petwdog_Stack_Array,           /* Stack array. */
    //     &Task_Petwdog_Buffer                /* Buffer for static allocation. */
    // );

    // Wait till all tasks check in
    xEventGroupWaitBits(
        xStateBits,    // The event group handle
        ALL_TASK_BITS, // The bits to wait for (your 'xTaskBits')
        pdFALSE,       // do not Clear the bits after they are met
        pdTRUE,        // xWaitForAllBits: pdTRUE = AND (Wait for ALL)
        portMAX_DELAY  // wait forever
    );


    // All tasks have checked in, ensure nothing faulted on startup before closing contactors
    if (is_fault_set(NUM_FAULTS) == false)
    {
        contactor_set(HV_PLUS_CONTACTOR, CONTACTOR_CLOSED, 10, NORMAL);
        contactor_set(HV_MINUS_CONTACTOR, CONTACTOR_CLOSED, 10, NORMAL);
    }

    // Task deletes itself after all other taks are init'd
    vTaskDelete(NULL);
}
