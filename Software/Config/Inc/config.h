/** config.h
 * Config file to hold any aliases/constants referenced by multiple files
 * Not specific to a single board/unit
 */

#pragma once

//--------------------------------------------------------------------------------
// Battery Pack layout
#ifndef NUM_BATTERY_MODULES
#define NUM_BATTERY_MODULES             32      // Number of battery modules
#endif

#ifndef NUM_TEMPERATURE_SENSORS
#define NUM_TEMPERATURE_SENSORS         NUM_BATTERY_MODULES      // Number of temperature sensors
#endif

#ifndef NUM_VOLTAGE_SENSORS
#define NUM_VOLTAGE_SENSORS             NUM_BATTERY_MODULES      // Number of voltage sensors
#endif

#define MODULES_PER_SEGMENT (NUM_BATTERY_MODULES / NUM_VOLTTEMP_BOARDS)

#define NUM_VOLTTEMP_BOARDS              8       // Number of volt temp boards

// volttemp segment voltage 
#define CELL_OVERVOLTAGE_THRESHOLD_MV 4200 // 4.2 V
#define CELL_UNDERVOLTAGE_THRESHOLD_MV 2500 // 2.5 V

// precharge macros
#define PACK_OVERVOLTAGE_THRESHOLD_MV (CELL_OVERVOLTAGE_THRESHOLD_MV*NUM_VOLTAGE_SENSORS) // 134.4 V
#define PACK_UNDERVOLTAGE_THRESHOLD_MV (CELL_UNDERVOLTAGE_THRESHOLD_MV*NUM_VOLTAGE_SENSORS) //  83.2 V

// battery segment temp voltage
#define OVERTEMP_THRESHOLD_CHARGING_MC 55000 // 55 C
#define OVERTEMP_THRESHOLD_DISCHARGING_MC 70000 // 70 C

// Charging Thresholds
#define CELL_CHARGING_VOLTAGE_THRESHOLD_MV 4150   // 4.15 V
#define CELL_CHARGING_TEMP_THRESHOLD_MC 50000 // 50 C

// current threshold to determine if battery is charging (negative number is charging, positive is discharging)
#define CHARGING_THRESHOLD_MA (-50) // -50 mA

// Charge-enable anti-oscillation. Charge disables the instant a cell reaches the charge voltage/temp
// limit, but RE-enabling requires the max cell to first fall a hysteresis band BELOW the limit, so a
// cell sitting at the limit can't flip charge on/off ("charge complete -> resume" / "cooled ->
// resume"). The minimum-disable dwell is a belt-and-suspenders so charge enable can't rapidly flip
// even if the hysteresis band is crossed quickly.
#define CHARGE_REENABLE_VOLTAGE_HYSTERESIS_MV 100   // re-enable 0.1 V below the charge-voltage cutoff
#define CHARGE_REENABLE_TEMP_HYSTERESIS_MC    2000  // re-enable 2 C below the charge-temp cutoff
#define MIN_CHARGE_DISABLE_TIME_MS            5000  // min time charge stays disabled before re-enable. ponytail: calibrate


// How many bad voltage reads are tolerable when switching states and closing contactors 
#define PRECHARGE_UNDERVOLTAGE_DEBOUNCE_LIMIT 2

// interrupt priorites
#define SHT45_IRQ_PRIO (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 4)
#define EMC2305_IRQ_PRIO (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 2)
#define FDCAN_NVIC_PRIO (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 3)
#define ADC_IRQ_PRIO (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1)

// Amperes Overcurrent Setpoints: 70A discharge, 40A charge
#define OVERCURRENT_DISCHARGE_THRESHOLD_mA  (68000)
#define OVERCURRENT_CHARGE_THRESHOLD_mA     (-38000)


#define PRE(s)  "\r    "s"  "   // \r removes the filepath and 'note: '#pragma message:...' parts
#define STR(x)  #x
#define XSTR(x) STR(x)

