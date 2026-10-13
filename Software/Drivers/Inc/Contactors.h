#pragma once
#include "common.h"

/* Timing Definitions */

/** * @brief Time to wait for the physical contactor to settle before reading feedback. 
 */
#define CONTACTOR_SENSE_DELAY_TICKS      pdMS_TO_TICKS(1000)  

/** * @brief Maximum blocking time in milliseconds for contactor callbacks. 
 */
#define CALLBACK_BLOCKING_TIME_MS        20

/**
 * @brief Represents the logical and physical state of a contactor.
 */
typedef enum {
    CONTACTOR_OPEN = 0,     /**< Circuit is disconnected */
    CONTACTOR_CLOSED,       /**< Circuit is connected */
    CONTACTOR_ERROR,        /**< Error getting contactor state (used for function return) */
    CONTACTOR_OK            /**< Contactor action completed successfully (used for function return) */
} contactor_state_t;

/**
 * @brief Represents the possible states of the ESTOP buttons.
 */
typedef enum {
    ESTOP_OK = 0,
    ESTOP1_FAULT,
    ESTOP2_FAULT,
    ESTOP3_FAULT
} estop_status_t;

/**
 * @brief Identification for specific contactors within the HV system.
 */
typedef enum {
    HV_PLUS_CONTACTOR = 0,      /**< Main Positive Bus */
    HV_MINUS_CONTACTOR,         /**< Main Negative Bus */
    ARRAY_CONTACTOR,            /**< Solar/Battery Array Main */
    ARRAY_PRE_CONTACTOR,        /**< Pre-charge circuit for the Array */
    NUM_CONTACTORS              /**< Total count helper */
} contactor_num_t;

extern const char* CONTACTOR_NAMES[NUM_CONTACTORS];

/**
 * @brief Contactor hardware abstraction object.
 */
typedef struct {
    contactor_state_t state;        /**< Current commanded state */
    GpioPin_t sense_pin;            /**< Digital input for auxiliary feedback loop */
    GpioPin_t control_pin;          /**< Digital output to coil driver/relay */
    bool callback_faulted;          /**< Whether or not this Contactor had a callback fault */
    
    /* RTOS Resources */
    TimerHandle_t senseTimer;       /**< Handle for non-blocking state verification */
    StaticTimer_t senseTimerBuffer; /**< Memory buffer for static timer allocation */
} contactor_t;

/** * @brief Hardware/RTOS initialization for all contactors, timers, and mutexes. 
 */
void contactor_init(void);

/** * @brief Reads the physical sense pin for a specific contactor.
 * * @param contactor_num The specific contactor to read from the hardware.
 * @return contactor_state_t Returns CONTACTOR_CLOSED if connected, CONTACTOR_OPEN if disconnected, or CONTACTOR_ERROR.
 */
contactor_state_t contactor_get(contactor_num_t contactor_num);

/**
 * @brief Verifies if the physical state of a specific contactor matches its expected software state. 
 * * @param contactor_num The specific contactor to check (e.g., Array, Motor, Precharge).
 * * @return contactor_state_t Returns CONTACTOR_OK if they match, CONTACTOR_ERR if they don't
 */
contactor_state_t contactor_verify(contactor_num_t contactor_num);

/** * @brief Commands a contactor state change with safety verification.
 * * @param contactor_num The specific contactor to switch.
 * @param state         The desired state (CONTACTOR_OPEN or CONTACTOR_CLOSED).
 * @param wait_ms       Wait time for sense delay before returning/checking.
 * @param emergency     If set to a fault state, executes immediately and bypasses standard safety delays.
 * @return contactor_state_t  Returns CONTACTOR_OK if command was accepted, CONTACTOR_ERR on hardware or RTOS failure.
 */
contactor_state_t contactor_set(contactor_num_t contactor_num, contactor_state_t state, TickType_t wait_ms, fault_state_t emergency);

/**
 * @brief Checks if that Contactor had a contactor callback fault
 * * @param contactor_num The specific contactor to check (e.g., Array, Motor, Precharge).
 * * @return true if the Contactor had a callback fault, false if not
 */
bool contactor_get_faulted_status(contactor_num_t contactor_num);

/** * @brief Determines if any ESTOPs are tripped
 * @return estop_status_t Returns ESTOP_OK if no estops are tripped, else it returns a code for each estop
 */
estop_status_t contactor_estop_checker();

/** * @brief RTOS Timer callback for contactor state verification.
 * @note This function is called when the sense timer for the contactor times out. 
 * It confirms that the physical contactor state changed successfully and matches the expected value.
 * * @param senseTimer Handle of the timer that expired.
 */
/* static void vContactorCallback( TimerHandle_t senseTimer ); */

/** * @brief Opens all contactors immediately in case of a system fault.
 * @note TODO: Check if contactors are initialized. If not, initialize them before commanding.
 */
void emergency_open_contactors(void);