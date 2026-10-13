/** charge.h
 * Charge-enable state + array (MPPT boost) shutdown control.
 *
 * charge_enabled is the single "charging is allowed / boost on" flag. It is owned by
 * the precharge task (set true only once the array is precharged and cell voltage/temp
 * are within the charge limits with no active fault). Monitor tasks can force it off
 * immediately when a charge limit is crossed.
 *
 * When charge is disabled we soft-shut the array (boost disable -> delayed array open).
 * Re-enabling charge is rate-limited (charge_reenable_allowed) and gated by the monitor-task
 * voltage/temp hysteresis so charge enable cannot oscillate. MPPT boost is driven by MpptControlTask
 * from charge_is_enabled(). The "still charging into an over-temp pack" escalation lives in the
 * temperature monitor task (temp-based, not charge-enable based).
 */

#pragma once

#include "common.h" // fault_state_t, TickType_t
#include <stdbool.h>
#include <stdint.h>

// ---- charge-enabled flag (drives MPPT boost + BPS_Charge_OK) ----
bool charge_is_enabled(void);
// Set the charge-enable flag. Disabling on a true->false edge also sends an immediate boost-disable
// to the MPPTs, so boost-off is instant on shutdown / leaving RUN (not just re-asserted at the 300 ms
// status cadence).
void charge_set_enabled(bool enabled);

// Immediately disable charging: clears the flag and sends boost disable to the MPPTs now
// (used by the monitor tasks the instant a charge limit is crossed).
void charge_force_disable(void);

// Anti-oscillation: returns true once MIN_CHARGE_DISABLE_TIME_MS has elapsed since charging was last
// disabled. The precharge task requires this before recovering charge so it can't rapidly flip.
bool charge_reenable_allowed(void);

// Soft/hard-open the array: boost disable, then open the array + array-precharge
// contactors. When soft, waits FAULT_SHUTDOWN_MPPT_DELAY_MS for MPPT wind-down before
// opening. mode = EMERGENCY for the fault path (bypasses mutex/sense), NORMAL otherwise.
void array_shutdown(fault_state_t mode, bool soft);
