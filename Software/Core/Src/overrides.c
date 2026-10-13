#include "overrides.h"
#include "common.h"

// 32 modules x 2 bits fit in the 8-byte override payloads
#define OVERRIDE_PAYLOAD_BYTES 8

// Single-writer (CAN status task) state; readers are monitor tasks.
// g_bps_command: raw BPS_Command (0x67) byte as last received. Bit layout mirrors CarCAN.dbc.
static volatile uint8_t g_bps_command = 0;
static volatile uint8_t g_module_override[NUM_BATTERY_MODULES] = {0};

// Bit positions within the BPS_Command (0x67) byte (CarCAN.dbc BPS_Command signal start bits).
#define BPS_CMD_BIT_DRIVE_PROFILE  0
#define BPS_CMD_BIT_REGEN_ALLOW    1
#define BPS_CMD_BIT_ADV_MPPT       2
#define BPS_CMD_BIT_SOFT_SHDN      3
#define BPS_CMD_BIT_VSAG           4

static inline bool bps_cmd_bit(uint8_t bit)
{
    return ((g_bps_command >> bit) & 0x1u) != 0u;
}

void overrides_set_command(uint8_t cmd_byte)
{
    g_bps_command = cmd_byte;
}

// Master gate: drive profile is active only if the config allows it AND the CAN master bit is set.
// When inactive, every other command signal below is forced off.
bool overrides_drive_profile_active(void)
{
    return (BPS_CMD_CONFIG_DRIVE_PROFILE != 0) && bps_cmd_bit(BPS_CMD_BIT_DRIVE_PROFILE);
}

// Per-signal effective state: master active AND the signal's config gate AND its CAN bit.
bool overrides_regen_allowed(void)
{
    return overrides_drive_profile_active() && (BPS_CMD_CONFIG_REGEN_ALLOW != 0) && bps_cmd_bit(BPS_CMD_BIT_REGEN_ALLOW);
}

bool overrides_adv_mppt_enabled(void)
{
    return overrides_drive_profile_active() && (BPS_CMD_CONFIG_ADV_MPPT_CONTROL != 0) && bps_cmd_bit(BPS_CMD_BIT_ADV_MPPT);
}

bool overrides_soft_shutdown_enabled(void)
{
    return overrides_drive_profile_active() && (BPS_CMD_CONFIG_SOFT_SHDN != 0) && bps_cmd_bit(BPS_CMD_BIT_SOFT_SHDN);
}

bool overrides_vsag_enabled(void)
{
    return overrides_drive_profile_active() && (BPS_CMD_CONFIG_VSAG_COMPENSATION != 0) && bps_cmd_bit(BPS_CMD_BIT_VSAG);
}

void overrides_set_module_raw(const uint8_t *data8)
{
    if (data8 == NULL)
    {
        return;
    }

    taskENTER_CRITICAL();
    for (uint8_t m = 0; m < NUM_BATTERY_MODULES; m++)
    {
        g_module_override[m] = (data8[m >> 2] >> ((m & 0x3u) * 2u)) & 0x3u;
    }
    taskEXIT_CRITICAL();
}

uint8_t overrides_get_module(uint8_t module_num)
{
    if (module_num >= NUM_BATTERY_MODULES)
    {
        return MODULE_OVERRIDE_NORMAL;
    }
    return g_module_override[module_num];
}

void overrides_pack_command_ack(uint8_t *data8)
{
    if (data8 == NULL)
    {
        return;
    }

    // Reflect the EFFECTIVE state (post config gating) so the sender sees what the BPS will honor.
    data8[0] = 0;
    data8[0] |= (uint8_t)(overrides_drive_profile_active()  ? 1u : 0u) << BPS_CMD_BIT_DRIVE_PROFILE;
    data8[0] |= (uint8_t)(overrides_regen_allowed()         ? 1u : 0u) << BPS_CMD_BIT_REGEN_ALLOW;
    data8[0] |= (uint8_t)(overrides_adv_mppt_enabled()      ? 1u : 0u) << BPS_CMD_BIT_ADV_MPPT;
    data8[0] |= (uint8_t)(overrides_soft_shutdown_enabled() ? 1u : 0u) << BPS_CMD_BIT_SOFT_SHDN;
    data8[0] |= (uint8_t)(overrides_vsag_enabled()          ? 1u : 0u) << BPS_CMD_BIT_VSAG;
}

void overrides_pack_module_ack(uint8_t *data8)
{
    if (data8 == NULL)
    {
        return;
    }

    for (uint8_t i = 0; i < OVERRIDE_PAYLOAD_BYTES; i++)
    {
        data8[i] = 0;
    }

    taskENTER_CRITICAL();
    for (uint8_t m = 0; m < NUM_BATTERY_MODULES; m++)
    {
        data8[m >> 2] |= (g_module_override[m] & 0x3u) << ((m & 0x3u) * 2u);
    }
    taskEXIT_CRITICAL();
}

// Drive profile globally disables overtemp when configured to do so
static inline bool drive_disables_overtemp(void)
{
    return (DRIVE_OVERRIDE_DISABLE_OVERTEMP != 0) && overrides_drive_profile_active();
}

bool override_suppress_overvoltage(uint8_t module_num)
{
    uint8_t o = overrides_get_module(module_num);
    return (o == MODULE_OVERRIDE_VOLTAGE) || (o == MODULE_OVERRIDE_ALL);
}

bool override_suppress_undervoltage(uint8_t module_num)
{
    uint8_t o = overrides_get_module(module_num);
    return (o == MODULE_OVERRIDE_VOLTAGE) || (o == MODULE_OVERRIDE_ALL);
}

bool override_suppress_overtemp(uint8_t module_num)
{
    uint8_t o = overrides_get_module(module_num);
    return drive_disables_overtemp() || (o == MODULE_OVERRIDE_TEMP) || (o == MODULE_OVERRIDE_ALL);
}

// Whole-domain suppression for a module: VOLTAGE/ALL silences EVERY voltage fault, TEMP/ALL EVERY
// temperature fault -- including the hardware (BQ-chip / sensor) faults, not just the value faults.
// Note: the temp HW fault deliberately does NOT honor drive_disables_overtemp -- relaxing high-temp
// limits for limp-home should not also mask a physically broken thermistor; only an explicit module
// TEMP/ALL override does.
bool override_suppress_voltage(uint8_t module_num)
{
    uint8_t o = overrides_get_module(module_num);
    return (o == MODULE_OVERRIDE_VOLTAGE) || (o == MODULE_OVERRIDE_ALL);
}

bool override_suppress_temp(uint8_t module_num)
{
    uint8_t o = overrides_get_module(module_num);
    return (o == MODULE_OVERRIDE_TEMP) || (o == MODULE_OVERRIDE_ALL);
}

int32_t overrides_overtemp_limit_mC(bool charging)
{
    if (overrides_drive_profile_active())
    {
        return charging ? OVERRIDE_OVERTEMP_THRESHOLD_CHARGING_MC
                        : OVERRIDE_OVERTEMP_THRESHOLD_DISCHARGING_MC;
    }
    return charging ? OVERTEMP_THRESHOLD_CHARGING_MC
                    : OVERTEMP_THRESHOLD_DISCHARGING_MC;
}

int32_t overrides_overvoltage_limit_mV(void)
{
    return overrides_drive_profile_active() ? OVERRIDE_CELL_OVERVOLTAGE_THRESHOLD_MV
                                            : CELL_OVERVOLTAGE_THRESHOLD_MV;
}

int32_t overrides_charge_limit_voltage_mV(void)
{
    return overrides_drive_profile_active() ? OVERRIDE_CELL_CHARGING_VOLTAGE_THRESHOLD_MV
                                            : CELL_CHARGING_VOLTAGE_THRESHOLD_MV;
}

int32_t overrides_overcurrent_charge_mA(void)
{
    return overrides_drive_profile_active() ? OVERRIDE_OVERCURRENT_CHARGE_THRESHOLD_mA
                                            : OVERCURRENT_CHARGE_THRESHOLD_mA;
}

int32_t overrides_overcurrent_discharge_mA(void)
{
    return overrides_drive_profile_active() ? OVERRIDE_OVERCURRENT_DISCHARGE_THRESHOLD_mA
                                            : OVERCURRENT_DISCHARGE_THRESHOLD_mA;
}

int32_t overrides_adjusted_uv_limit_mV(int32_t pack_current_mA)
{
    // Base UV floor: relaxed setpoint while the drive profile is active, else normal.
    int32_t limit = overrides_drive_profile_active() ? OVERRIDE_CELL_UNDERVOLTAGE_THRESHOLD_MV
                                                     : CELL_UNDERVOLTAGE_THRESHOLD_MV;

    // Only sag-compensate while the vsag-compensation command signal is active and discharging.
    if (overrides_vsag_enabled() && (pack_current_mA > 0))
    {
        // adj = uv - I(mA) * R(mOhm) * FoS(%) / 100000   (64-bit math avoids overflow)
        int64_t drop = ((int64_t)pack_current_mA * ESTIMATED_MODULE_RESISTANCE_MOHM *
                        VSAG_COMPENSATION_FOS_PERCENT) /
                       100000;
        limit -= (int32_t)drop;
        if (limit < 0)
        {
            limit = 0;
        }
    }

    return limit;
}

bool shutdown_soft_active(uint8_t mode)
{
    switch (mode)
    {
    case SHUTDOWN_MODE_ALWAYS:
        return true;
    case SHUTDOWN_MODE_OVERRIDE:
        return overrides_soft_shutdown_enabled();
    case SHUTDOWN_MODE_NEVER:
    default:
        return false;
    }
}
