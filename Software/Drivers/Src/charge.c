#include "charge.h"
#include "Contactors.h"
#include "TPEE_Utils.h"

// short, non-blocking-ish wait for the boost CAN sends
#define BOOST_CAN_DELAY_MS 5u

// charge_enabled: set true only by the precharge task (array precharged + in range).
static volatile bool g_charge_enabled = false;

// Tick of the most recent enabled->disabled transition. Anchors the minimum charge-disable dwell
// (charge_reenable_allowed, anti-oscillation).
static volatile TickType_t g_last_disable_tick = 0;

bool charge_is_enabled(void)
{
    return g_charge_enabled;
}

// Record the disable edge (enabled -> disabled). Both the re-enable dwell and the escalation grace
// window are measured from this instant.
static inline void charge_note_disable_edge(void)
{
    if (g_charge_enabled)
    {
        g_last_disable_tick = xTaskGetTickCount();
    }
}

void charge_set_enabled(bool enabled)
{
    if (!enabled)
    {
        // On the enabled->disabled edge (precharge leaving RUN, shutdown to IDLE, fault, etc.) kill
        // boost NOW instead of waiting for the 300 ms status-task re-assert -> deterministic,
        // instant boost-off. Only fires on the edge, so steady IDLE doesn't spam the bus.
        if (g_charge_enabled)
        {
            disableAllMPPTs(BOOST_CAN_DELAY_MS);
        }
        charge_note_disable_edge();
    }
    g_charge_enabled = enabled;
}

void charge_force_disable(void)
{
    charge_note_disable_edge();
    g_charge_enabled = false;
    disableAllMPPTs(BOOST_CAN_DELAY_MS);
}

bool charge_reenable_allowed(void)
{
    return Calculate_TimeDifference(xTaskGetTickCount(), g_last_disable_tick) >=
           pdMS_TO_TICKS(MIN_CHARGE_DISABLE_TIME_MS);
}

void array_shutdown(fault_state_t mode, bool soft)
{
    // contactor_set takes a raw-ms wait (it converts internally)
    TickType_t wait = (mode == EMERGENCY) ? 0 : CALLBACK_BLOCKING_TIME_MS;

    // boost disable first so the MPPTs stop pushing current
    disableAllMPPTs(BOOST_CAN_DELAY_MS);

    // Only when a soft shutdown is active do we sequence the open: wait FAULT_SHUTDOWN_MPPT_DELAY_MS
    // (100-150ms) for the MPPTs to actually wind down, then open array precharge, wait the
    // inter-contactor gap, then open the main array contactor -- so the contacts never break full
    // solar current. When soft is NOT active, fall through and open both at once (boost disable has
    // already been sent above).
    if (soft)
    {
        vTaskDelay(pdMS_TO_TICKS(FAULT_SHUTDOWN_MPPT_DELAY_MS));
    }

    contactor_set(ARRAY_PRE_CONTACTOR, CONTACTOR_OPEN, wait, mode);

    if (soft)
    {
        vTaskDelay(pdMS_TO_TICKS(FAULT_SHUTDOWN_INTERCONTACTOR_MS));
    }

    contactor_set(ARRAY_CONTACTOR, CONTACTOR_OPEN, wait, mode);
}
