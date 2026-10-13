#pragma once

#include "common.h"
#include "Contactors.h"
#include "StatusLEDs.h"
#include "ADC_Driver.h"
#include "DebugPrintf.h"
#include "BPS_Tasks.h"
#include "CarCAN_can_msgs.h"
#include "CANbus.h"
#include "TPEE_Utils.h"


// Fixed-point scaling for mV to V conversion
#define mV_TO_V_SCALAR 1000

// 900/1000 = 0.900, 800/1000 = 0.800
// TODO: Test and increase hysteresis threshold closer to 90%
#define PRECHARGE_THRESHOLD_90 900
#define PRECHARGE_THRESHOLD_80 800 // NOTE: The second threshold exists to account for hysteresis, so that we don't drop out of the run state after successfully precharging just because of a small ADC reading change

// Allowed difference between array and battery voltage
#define VOLTAGE_TOLERANCE_NUMERATOR 22
#define VOLTAGE_TOLERANCE_DENOMINATOR 20 // array voltage can at most 10% higher than battery voltage

#define PRECHARGE_TIMEOUT_MS 400 // Precharge time to 90% -> 0.9 = 1 - e^(-t/RC), so t = -RC * ln(1-0.9) = 2.3*RC. For our case, R = 110 Ohms and C = 1 mF -> t = 253 ms.
#define ADC_TIMEOUT_MS 20

extern StaticTask_t Precharge_Task_Buffer;
extern StackType_t Precharge_Task_Stack[PRECHARGE_TASK_STACK_SIZE];

/* handle for the Precharge task, defined in PrechargeTask.c */
extern TaskHandle_t hprecharge_task;

typedef enum
{
    PRECHARGE_STATE_INITIAL = 0, // Begin precharge sequence, start by closing main array contactor and starting a timer to check for precharge timeout
    PRECHARGE_STATE_PRECHARGING, // Precharge sequence started successfully, close contactor and check hysteresis
    PRECHARGE_STATE_RUN,         // Precharge got through hysteresis, now continuously polling ADC
    PRECHARGE_STATE_IDLE,        // Precharge sequence is waiting for array ignition state (do nothing)
    PRECHARGE_STATE_FAULT        // We had a fault, this state exits precharge from anywhere gracefully
} Precharge_State_t;

/**
 * @brief Precharge task initialization function, initializes ADC, printf, and contactors
 * @param None
 * @retval None
 */
void Init_PrechargeTask();

/**
 * @brief Checks the repeated fault conditions for precharge sequence, if any fault condition is met, will call fault handler and not return
 * @param Array_Voltage most recent array voltage reading in mV
 * @param Battery_Voltage most recent battery voltage reading in mV
 * @param State current state of precharge system, to prevent undervoltage fault while in idle
 * @retval None
 */
void Fault_Checker(uint32_t Array_Voltage, uint32_t Battery_Voltage, Precharge_State_t State);

/**
 * @brief Precharge task main execution function, implements precharge pseudo state machine and fault handling
 * @param None
 * @retval None
 */
void Task_Precharge();
