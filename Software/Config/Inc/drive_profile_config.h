/** drive_profile_config.h
 * Config for the BPS "drive override" feature (CAN 0x67/0x69), the relaxed
 * "drive profile" protection setpoints that apply ONLY while the drive override
 * is active, and the (always-on) regen-allowed gating setpoints.
 *
 * Split out of config.h so all override/profile/regen tuning lives in one place.
 * Included by config.h, so every consumer of config.h sees these defines too.
 */

#pragma once

// BPS Command (CAN 0x67 / ack 0x667) -- per-signal HARD config gates. The command is a 1-byte
// bitfield from Controls (see CarCAN.dbc BPS_Command): bit0 BPS_Drive_Profile_Enable_Master,
// bit1 BPS_Regen_Allow, bit2 BPS_Adv_MPPT_Control, bit3 BPS_Soft_Shdn, bit4 BPS_Vsag_Compensation.
// For each signal the gate below decides whether the BPS listens to the CAN bit at all:
//   0 = hard-disabled: the CAN bit is IGNORED and the behavior is forced off.
//   1 = enabled: the behavior follows the received CAN bit.
// BPS_CMD_CONFIG_DRIVE_PROFILE is the master: if 0, EVERY signal is forced off regardless of its own
// gate or the CAN message (the drive profile, and thus all relaxed setpoints below, never engage).
#define BPS_CMD_CONFIG_DRIVE_PROFILE      1
#define BPS_CMD_CONFIG_REGEN_ALLOW        1
#define BPS_CMD_CONFIG_ADV_MPPT_CONTROL   1
#define BPS_CMD_CONFIG_SOFT_SHDN          1
#define BPS_CMD_CONFIG_VSAG_COMPENSATION  1

// Drive-profile sub-behavior (gated by the master): how cell overtemp is handled while active.
#define DRIVE_OVERRIDE_DISABLE_OVERTEMP     0   // 0: relax overtemp to OVERRIDE_OVERTEMP_* below; 1: blanket-suppress

// Voltage sag compensation tuning (applied only while BPS_Vsag_Compensation is active and discharging):
//   adj_uv_limit_mV = OVERRIDE_CELL_UNDERVOLTAGE_THRESHOLD_MV - (I_mA * R_mOhm * FoS%) / 100000
#define ESTIMATED_MODULE_RESISTANCE_MOHM    3  // estimated per-module resistance (milliohms)
#define VSAG_COMPENSATION_FOS_PERCENT       80 // factor of safety (percent)

//--------------------------------------------------------------------------------
// Drive profile setpoints: applied ONLY while the drive override (0x67) is active.
// They intentionally relax the normal limits in config.h so the car can keep running
// in an emergency / limp-home.
// ponytail: starting values only — CALIBRATE to the cell datasheet / risk tolerance.

// Cell overtemp limits while override active (normal: 55C charge / 70C discharge)
#define OVERRIDE_OVERTEMP_THRESHOLD_CHARGING_MC      60000   // 60 C
#define OVERRIDE_OVERTEMP_THRESHOLD_DISCHARGING_MC   80000   // 80 C

// Cell voltage limits while override active:
//   discharge -> undervoltage floor   (normal: 2600 mV)
//   charge    -> overvoltage ceiling  (normal: 4200 mV)
#define OVERRIDE_CELL_UNDERVOLTAGE_THRESHOLD_MV      2550    // 2.55 V (discharge floor)
#define OVERRIDE_CELL_OVERVOLTAGE_THRESHOLD_MV       4230    // 4.23 V (charge ceiling)

// Master "OK for charging" cutoff while the drive profile is active. This is the BPS-level backstop
// (disable charge + open the array) and is deliberately set ABOVE the advanced-MPPT CV target so it
// does NOT preempt the CV hold/taper -- it only fires if the CV loop fails to hold. Drive-profile
// voltage ladder: MPPT_TAPER_START_MV (4150, CC->CV) < MPPT_CV_TARGET_MV (4200, hold + current-
// terminate) < OVERRIDE_CELL_CHARGING_VOLTAGE_THRESHOLD_MV (4225, master cutoff) <
// OVERRIDE_CELL_OVERVOLTAGE_THRESHOLD_MV (4250, hard fault). The ordering is locked by _Static_asserts
// in MpptControlTask.c. ponytail: 4225 = midpoint of CV target and hard fault; calibrate to the CV
// loop's observed overshoot (tighten toward ~4215 if it holds tight, widen toward ~4235 if it rings).
#define OVERRIDE_CELL_CHARGING_VOLTAGE_THRESHOLD_MV  4205    // 4.205 V (backstop above the CV target)

// Overcurrent thresholds while the drive profile is active (relaxed from the normal config.h limits
// for limp-home). Selected via overrides_overcurrent_charge_mA() / _discharge_mA() when
// overrides_drive_profile_active(), same config + CAN-flag gate as the other drive-profile setpoints.
// ponytail: starting values -- CALIBRATE to the pack / contactor / wiring current ratings.
#define OVERRIDE_OVERCURRENT_DISCHARGE_THRESHOLD_mA  (75000)   // 75 A discharge (normal: 68 A)
#define OVERRIDE_OVERCURRENT_CHARGE_THRESHOLD_mA     (-54000)  // -54 A charge (normal: -38 A)

//--------------------------------------------------------------------------------
// Regen-allowed gating. Reported to the VCU via BPS_Regen_OK in the status message:
// regen is advertised as safe only while the drive override (0x67) is active AND the
// pack's max cell voltage AND max cell temperature are below these setpoints. The BPS
// does not actuate regen, it only advertises whether regen is safe; overcurrent still
// faults regardless.
#define REGEN_VOLTAGE_THRESHOLD_MV   4150    // 4.15 V: regen allowed below, disabled at/above
#define REGEN_TEMP_THRESHOLD_MC      55000   // 55 C: regen allowed below

// Regen re-enable hysteresis (mirror of the charge-enable hysteresis): once regen is disabled at the
// threshold above, it only re-enables after the max cell drops this far back below it, so a cell
// hovering at the limit can't flap BPS_Regen_OK. ponytail: same delta as charge for now; calibrate.
#define REGEN_REENABLE_VOLTAGE_HYSTERESIS_MV 100   // same as CHARGE_REENABLE_VOLTAGE_HYSTERESIS_MV for now
#define REGEN_REENABLE_TEMP_HYSTERESIS_MC    2000  // same as CHARGE_REENABLE_TEMP_HYSTERESIS_MC for now

//--------------------------------------------------------------------------------
// Advanced MPPT control. Built + active only when BPS_CMD_CONFIG_ADV_MPPT_CONTROL is 1 AND the
// BPS_Adv_MPPT_Control command signal is active (which itself requires the drive-profile master).
// Otherwise the BPS uses plain boost enable/disable. The control-law constants below are only used
// while it is active.

// Control law (keyed on the pack's MAX CELL, mV -- NOT pack/bus voltage, which says nothing about
// the fullest cell once cells imbalance). Below the taper point we let the MPPTs do full power-point
// tracking; above it we close a loop on the max cell, holding it near the CV target by steering the
// MPPT output-voltage ceiling (the TPEE reduces its own current as the bus nears the lowered ceiling).
//   max cell <  MPPT_TAPER_START_MV   -> full MPPT power tracking (mode 1); no overcharge risk this low
//   max cell >= MPPT_TAPER_START_MV   -> CV taper: hold max cell at ~MPPT_CV_TARGET_MV (mode 2)
// Charging then ENDS ON CURRENT (see termination block below), not the instant a cell first reaches
// target -- so imbalanced low cells keep filling while the high cell is pinned and balancing works.
#define MPPT_TAPER_START_MV   4150     // enter closed-loop CV taper at/above this max-cell voltage
#define MPPT_CV_TARGET_MV     4180     // max-cell setpoint the loop holds (never intentionally exceeded)
#define MPPT_CV_DEADBAND_MV   10       // hold band [target-deadband, target]; no ceiling nudge inside it
#define MPPT_CHARGE_RESTART_MV 4100    // after termination, resume harvesting once max cell relaxes <= this

// Closed-loop integrator on the MPPT output (bus) voltage ceiling. Pack/bus mV; converted to the
// MPPT's wire units via MPPT_VOLTAGE_LIMIT_SCALE_MV_PER_LSB below. Each control cycle
// (MPPT_CONTROL_TASK_DELAY_MS = 300 ms) the ceiling steps one notch toward holding the max cell at
// target: cell over target -> lower ceiling (less current), cell below the band -> raise it.
// ponytail: crude deadband-integral loop (fixed step, no gain term) -- enough to taper gracefully.
#define MPPT_VLIMIT_STEP_MV   100      // ceiling change per control cycle
#define MPPT_MAX_BOOST_MV     136000   // ceiling upper clamp. 32S ~134.4 V full + headroom (noise/IR/imbalance)
#define MPPT_MIN_BOOST_MV     130000   // ceiling lower clamp (charge current ~0 here for a full top cell)

// Current-based charge termination -- the correct way to end CC/CV (NOT "first cell hit target").
// Once the max cell has held in the CV band with the charge current tapered below the cutoff for the
// full dwell time, the top cell is full, balancing/polarization have settled, and remaining charge
// acceptance is negligible: MPPTs go off (then restart via the hysteresis above when the cell sags).
// NOTE: uses NET pack current (get_pack_current(), negative = charging) -- the only current the BPS
// senses. While driving, motor draw can mask the array current, so this is an approximation; the
// restart hysteresis makes it self-correcting. ponytail: add a dedicated array-current sense if exact
// charge-current termination ever matters.
#define MPPT_CHARGE_TERMINATE_CURRENT_MA 500    // charge current considered "tapered" below this (|mA|)
#define MPPT_CHARGE_TERMINATE_TIME_MS    60000  // ...held continuously this long -> charge complete

// Wire scaling of the SetOutputVoltageLimit int16 field, from the Open-SEC manual (Packet ID 11):
// 0.01 V/LSB => 10 mV per LSB. So MPPT_MAX_BOOST_MV (136000) is sent as 136000/10 = 13600.
// ponytail/VERIFY: confirm 0.01 V/LSB against the current Open-SEC datasheet before relying on the
// CV ceiling. A wrong scale silently mis-sets the constant-voltage limit (only used while the
// advanced MPPT control signal is active). NOT verified here -- no datasheet in repo.
#define MPPT_VOLTAGE_LIMIT_SCALE_MV_PER_LSB 10

//--------------------------------------------------------------------------------
// Sequenced "soft" shutdown timing (limits inductive overvoltage / contactor arcing). Reverse of
// startup (HV- , +50ms, HV+ , ignition, array contactor, precharge, array precharge, boost enable).
// Shutdown order: broadcast fault status -> boost disable -> wait MPPT_DELAY (MPPTs wind down) ->
// open array precharge contactor -> wait INTERCONTACTOR -> open array contactor -> wait HV_DELAY
// (motor board zeros torque + opens its contactors) -> open HV+ -> wait INTERCONTACTOR -> open HV-.
#define FAULT_SHUTDOWN_INTERCONTACTOR_MS   50   // "shortly after" gap within each contactor pair
#define FAULT_SHUTDOWN_MPPT_DELAY_MS       150  // wait after boost disable before opening array
#define FAULT_SHUTDOWN_HV_DELAY_MS         500  // wait after opening the array contactor before HV+
                                                // (lets the motor board zero torque + open its contactors)

// 3-state shutdown modes (independently selected per shutdown type below).
#define SHUTDOWN_MODE_NEVER                0    // hard: open contactors immediately
#define SHUTDOWN_MODE_ALWAYS               1    // soft: sequenced open
#define SHUTDOWN_MODE_OVERRIDE             2    // soft only while the BPS_Soft_Shdn command signal is active

// Emergency (hard fault) shutdown. In a soft emergency shutdown the array is ALWAYS
// soft-shut as part of the sequence (this does not depend on ARRAY_SOFT_SHUTDOWN_MODE).
#define EMERGENCY_SOFT_SHUTDOWN_MODE       SHUTDOWN_MODE_OVERRIDE

// Array shutdown used when CHARGING is disabled (cell over charge-voltage / over-temp). Governs ONLY
// the charge-disable / array drop, independently of EMERGENCY_SOFT_SHUTDOWN_MODE so the array
// wind-down can be tuned separately from the emergency/fault shutdown.
#define ARRAY_SOFT_SHUTDOWN_MODE           SHUTDOWN_MODE_OVERRIDE
