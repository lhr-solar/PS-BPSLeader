/** overrides.h
 * Central state for the BPS Command + module override CAN features.
 *
 * - BPS Command      (CAN 0x67 BPS_Command):         1-byte bitfield. bit0 drive-profile master
 *     (gates the rest + the relaxed drive-profile setpoints), bit1 regen-allow, bit2 advanced-MPPT,
 *     bit3 soft-shutdown, bit4 vsag-compensation. Each is also hard-gated in config (BPS_CMD_CONFIG_*).
 * - Module overrides (CAN 0x69 BPS_Module_Override): per-module 2-bit code.
 *
 * State defaults to 0 (no command) and only changes when a CAN message is
 * received. A single writer (CAN status task) updates it; monitor tasks read it.
 * All accessors are lightweight and non-blocking (safe from any task).
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

// Per-module override codes (matches CarCAN BPS_Module_Override value table)
typedef enum {
    MODULE_OVERRIDE_NORMAL  = 0,
    MODULE_OVERRIDE_VOLTAGE = 1,
    MODULE_OVERRIDE_TEMP    = 2,
    MODULE_OVERRIDE_ALL     = 3,
} module_override_t;

// ---- BPS Command (0x67) ----
// Store the raw 1-byte command bitfield as received (bit layout: see CarCAN.dbc BPS_Command).
void overrides_set_command(uint8_t cmd_byte);

// Effective signal states. Each is true only if its config gate allows it, the drive-profile master
// is active (config gate + CAN bit0), AND the signal's own CAN bit is set. If the drive profile is
// inactive, all of these are false.
bool overrides_drive_profile_active(void);   // master gate; also enables the relaxed drive-profile setpoints
bool overrides_regen_allowed(void);
bool overrides_adv_mppt_enabled(void);
bool overrides_soft_shutdown_enabled(void);
bool overrides_vsag_enabled(void);

// ---- Module overrides (0x69) ----
// data8 is the raw 8-byte payload (32 modules x 2 bits, little-endian).
void overrides_set_module_raw(const uint8_t *data8);
uint8_t overrides_get_module(uint8_t module_num);

// ---- Ack packing (0x667 / 0x669) ----
// The command ack reflects the EFFECTIVE state (what the BPS will honor after config gating).
void overrides_pack_command_ack(uint8_t *data8);
void overrides_pack_module_ack(uint8_t *data8);

// ---- Fault suppression (combines drive + module overrides) ----
// Value (threshold) faults:
bool override_suppress_overvoltage(uint8_t module_num);
bool override_suppress_undervoltage(uint8_t module_num);
bool override_suppress_overtemp(uint8_t module_num);
// Whole-domain (value AND hardware) faults for a module -- used to gate the BQ-chip / sensor HW
// faults so a VOLTAGE/TEMP/ALL module override covers them too, not just the value faults.
bool override_suppress_voltage(uint8_t module_num);
bool override_suppress_temp(uint8_t module_num);

// ---- Voltage sag compensation ----
// Returns the (possibly lowered) per-cell undervoltage limit in mV for the given
// pack current (mA, positive = discharging). The base limit is the relaxed override
// (discharge) setpoint while the drive override is active, else the normal limit; sag
// compensation is then applied only while overriding and discharging.
int32_t overrides_adjusted_uv_limit_mV(int32_t pack_current_mA);

// ---- Drive-profile thresholds ----
// Each returns the relaxed setpoint while the drive profile is active (overrides_drive_profile_active),
// otherwise the normal config.h limit.
int32_t overrides_overtemp_limit_mC(bool charging);   // charging selects the charge vs discharge setpoint
int32_t overrides_overvoltage_limit_mV(void);         // per-cell overvoltage ceiling (charge)
int32_t overrides_charge_limit_voltage_mV(void);      // "OK for charging" cutoff voltage
int32_t overrides_overcurrent_charge_mA(void);        // pack charge overcurrent fault threshold (negative)
int32_t overrides_overcurrent_discharge_mA(void);     // pack discharge overcurrent fault threshold (positive)

// ---- Shutdown-mode resolver ----
// Given a SHUTDOWN_MODE_* value, returns whether a soft (sequenced) shutdown should be
// used now: ALWAYS -> true, NEVER -> false, OVERRIDE -> only while drive override active.
bool shutdown_soft_active(uint8_t mode);
