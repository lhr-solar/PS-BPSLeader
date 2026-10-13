#pragma once

#include "common.h"
#include "StatusLEDs.h"
#include "UART.h"
#include "DebugPrintf.h"
#include "bq.h"

// BQ79616 EVM bench test task configuration
#define BQ_TEST_TASK_STACK_SIZE     (configMINIMAL_STACK_SIZE * 8)
#define BQ_TEST_TASK_PRIO           (tskIDLE_PRIORITY + 3)

#define BQ_TEST_MAX_DEVICES         2U      // EVMs in the daisy chain (T3/T4.2)
#define BQ_TEST_NUM_DEVICES         1U      // BQ79616 EVMs behind the BQ79600 bridge right now
#define BQ_TEST_NUM_CELLS           8U      // 8S, test plan T1
#define BQ_TEST_NUM_GPIO            8U

// T1.1: voltages applied to cells 1-8 (V), and how far a reading may be off (mV)
#define BQ_TEST_CELL_EXPECTED_V     {3.2f, 3.4f, 3.5f, 3.6f, 3.7f, 3.8f, 4.0f, 4.1f}
#define BQ_TEST_CELL_TOL_MV         10.0f

// T2: NTC divider TSREF -> pull-up -> GPIOn -> NTC -> AVSS. Check these against the EVM BOM.
#define BQ_TEST_NTC_R_PULLUP        10000.0f
#define BQ_TEST_NTC_R25             10000.0f
#define BQ_TEST_NTC_BETA            3380.0f
// VTSREF 4.975-5.025 V widened by the +/-30 mV load regulation spec (datasheet 5.5)
#define BQ_TEST_TSREF_MIN_V         4.945f
#define BQ_TEST_TSREF_MAX_V         5.055f
#define BQ_TEST_AMBIENT_TOL_C       3.0f    // T2.2: each channel vs ambient
#define BQ_TEST_HEAT_RISE_C         5.0f    // T2.3: heated channel must rise at least this much
#define BQ_TEST_HEAT_OTHERS_C       2.0f    // T2.3: other channels may move at most this much

// T2.4 / T4: polling cadence
#define BQ_TEST_POLL_PERIOD_MS      100U
#define BQ_TEST_POLL_CYCLES         100U
#define BQ_TEST_PRE_TIMEOUT_POLL_MS 3000U   // T4: poll this long before going silent
#define BQ_TEST_NO_TRIP_MIN_MS      30000U  // T4.4: poll at least this long

// T4: COMM_TIMEOUT_CONF[CTL_TIME] code (1 = 100 ms, 2 = 2 s, 3 = 10 s, 4 = 1 min)
#define BQ_TEST_CTL_CODE_DEFAULT    2U
#define BQ_TEST_CTL_CODE_MAX        4U
#define BQ_TEST_MARKER              0xA5U   // scratch value that only survives if a device is never reset

// T5: manual balancing. NO_ADJ_CB = 1 at reset, so channels must not be adjacent.
#define BQ_TEST_BAL_MASK_DEFAULT    0x55U   // cells 1, 3, 5, 7
#define BQ_TEST_BAL_TIMER_CODE      0x04U   // CB_CELLn_CTRL[TIME] = 300 s
#define BQ_TEST_BAL_TIMER_S         300U
#define BQ_TEST_PAUSE_HOLD_MS       15000U  // T5.3/T5.4: BAL_TIME has 5 s resolution
#define BQ_TEST_SETTLE_SAMPLES      5U      // T5.6: samples averaged per settle delay
#define BQ_TEST_SETTLE_ERR_MV       1.0f    // T5.6: error that counts as back to baseline

// Bring-up steps, recorded in bq_test_fail_step so a failure can be read in GDB
typedef enum {
    BQ_TEST_NONE = 0,               /**< No failure (yet) */
    BQ_TEST_WAKE_BEFORE_INIT,       /**< bq_wake() did not reject an uninitialised driver */
    BQ_TEST_INIT_RETURN,            /**< bq_init() returned BQ_ERR */
    BQ_TEST_INIT_PERIPH,            /**< USART3 / PB10 / PB11 not configured as expected */
    BQ_TEST_WAKE_RETURN,            /**< bq_wake() returned BQ_ERR */
    BQ_TEST_WAKE_PIN_RESTORE,       /**< TX pin not handed back to the UART after the wake ping */
} bq_test_step_t;

// Checks bq_init() and bq_wake(), then runs the test plan (test.txt) from a serial menu on UART4
void Task_BQ_Test(void *pvParameters);
