#pragma once

#include <stdint.h>

/* ================= CAN ID Macros ================= */

#define CAN_ID_BPS_STATUS 0x1
#define CAN_ID_BPS_VOLTAGE_AGGREGATE_ARR 0xB
#define CAN_ID_BPS_TEMPERATURE_AGGREGATE_ARR 0xC
#define CAN_ID_CHARGERINTERFACE_STATUS 0xE
#define CAN_ID_VCU_STATUS 0x18
#define CAN_ID_CONTROLS_STATUS 0x19
#define CAN_ID_BPS_PRECHARGE_VOLTAGES 0x20
#define CAN_ID_VCU_PRECHARGE_VOLTAGES 0x21
#define CAN_ID_PEDAL_STATUS 0x50
#define CAN_ID_DRIVER_INPUT_STATUS 0x60
#define CAN_ID_MPPT_A_POWERMEASUREMENTS 0x200
#define CAN_ID_MPPT_A_STATUS 0x201
#define CAN_ID_MPPT_A_SWEEPMEASUREMENTS 0x202
#define CAN_ID_MPPT_A_SETMODE 0x209
#define CAN_ID_MPPT_A_PERFORMSWEEP 0x210
#define CAN_ID_MPPT_A_SETOUTPUTVOLTAGELIMIT 0x211
#define CAN_ID_MPPT_A_SETOUTPUTCURRENTLIMIT 0x212
#define CAN_ID_MPPT_B_POWERMEASUREMENTS 0x220
#define CAN_ID_MPPT_B_STATUS 0x221
#define CAN_ID_MPPT_B_SWEEPMEASUREMENTS 0x222
#define CAN_ID_MPPT_B_SETMODE 0x229
#define CAN_ID_MPPT_B_PERFORMSWEEP 0x230
#define CAN_ID_MPPT_B_SETOUTPUTVOLTAGELIMIT 0x231
#define CAN_ID_MPPT_B_SETOUTPUTCURRENTLIMIT 0x232
#define CAN_ID_MPPT_C_POWERMEASUREMENTS 0x240
#define CAN_ID_MPPT_C_STATUS 0x241
#define CAN_ID_MPPT_C_SWEEPMEASUREMENTS 0x242
#define CAN_ID_MPPT_C_SETMODE 0x249
#define CAN_ID_MPPT_C_PERFORMSWEEP 0x250
#define CAN_ID_MPPT_C_SETOUTPUTVOLTAGELIMIT 0x251
#define CAN_ID_MPPT_C_SETOUTPUTCURRENTLIMIT 0x252
#define CAN_ID_SUPP_BATTERY_STATUS 0x300
#define CAN_ID_SUPP_CHARGING_STATUS 0x301
#define CAN_ID_PDU_STATUS_ARR 0x350
#define CAN_ID_PDU_SET_SWITCH_ARR 0x351
#define CAN_ID_PDU_SET_CURRENT_LIMIT_ARR 0x352
#define CAN_ID_PUMP_STATUS 0x500
#define CAN_ID_COOLANT_TEMPERATURE 0x501
#define CAN_ID_RADIATOR_FANSPEED 0x502
#define CAN_ID_LV_CARRIER_STATUS 0x600
#define CAN_ID_BRAKE_PRESSURE_1 0x650
#define CAN_ID_BRAKE_PRESSURE_2 0x651
#define CAN_ID_DISPLAY_STATUS 0x680
#define CAN_ID_TELEMETRY_STATUS 0x700
#define CAN_ID_BPS_TEMP_RAWV_AGGREGATE_ARR 0x750
#define CAN_ID_SUPP_MEASUREMENTS_RAWV 0x751
#define CAN_ID_SUPP_VICOR_MEASUREMENTS_RAWV 0x752
#define CAN_ID_PEDAL_BRAKE_RAWV 0x753
#define CAN_ID_PEDAL_ACCEL_RAWV 0x754

/* ================= CAN Length Macros ================= */

#define CAN_DLC_BPS_STATUS 8
#define CAN_DLC_BPS_VOLTAGE_AGGREGATE_ARR 6
#define CAN_DLC_BPS_TEMPERATURE_AGGREGATE_ARR 8
#define CAN_DLC_CHARGERINTERFACE_STATUS 5
#define CAN_DLC_VCU_STATUS 4
#define CAN_DLC_CONTROLS_STATUS 6
#define CAN_DLC_BPS_PRECHARGE_VOLTAGES 6
#define CAN_DLC_VCU_PRECHARGE_VOLTAGES 6
#define CAN_DLC_PEDAL_STATUS 6
#define CAN_DLC_DRIVER_INPUT_STATUS 2
#define CAN_DLC_MPPT_A_POWERMEASUREMENTS 8
#define CAN_DLC_MPPT_A_STATUS 5
#define CAN_DLC_MPPT_A_SWEEPMEASUREMENTS 6
#define CAN_DLC_MPPT_A_SETMODE 1
#define CAN_DLC_MPPT_A_PERFORMSWEEP 5
#define CAN_DLC_MPPT_A_SETOUTPUTVOLTAGELIMIT 2
#define CAN_DLC_MPPT_A_SETOUTPUTCURRENTLIMIT 2
#define CAN_DLC_MPPT_B_POWERMEASUREMENTS 8
#define CAN_DLC_MPPT_B_STATUS 5
#define CAN_DLC_MPPT_B_SWEEPMEASUREMENTS 6
#define CAN_DLC_MPPT_B_SETMODE 1
#define CAN_DLC_MPPT_B_PERFORMSWEEP 5
#define CAN_DLC_MPPT_B_SETOUTPUTVOLTAGELIMIT 2
#define CAN_DLC_MPPT_B_SETOUTPUTCURRENTLIMIT 2
#define CAN_DLC_MPPT_C_POWERMEASUREMENTS 8
#define CAN_DLC_MPPT_C_STATUS 5
#define CAN_DLC_MPPT_C_SWEEPMEASUREMENTS 6
#define CAN_DLC_MPPT_C_SETMODE 1
#define CAN_DLC_MPPT_C_PERFORMSWEEP 5
#define CAN_DLC_MPPT_C_SETOUTPUTVOLTAGELIMIT 2
#define CAN_DLC_MPPT_C_SETOUTPUTCURRENTLIMIT 2
#define CAN_DLC_SUPP_BATTERY_STATUS 6
#define CAN_DLC_SUPP_CHARGING_STATUS 6
#define CAN_DLC_PDU_STATUS_ARR 5
#define CAN_DLC_PDU_SET_SWITCH_ARR 1
#define CAN_DLC_PDU_SET_CURRENT_LIMIT_ARR 3
#define CAN_DLC_PUMP_STATUS 6
#define CAN_DLC_COOLANT_TEMPERATURE 4
#define CAN_DLC_RADIATOR_FANSPEED 8
#define CAN_DLC_LV_CARRIER_STATUS 1
#define CAN_DLC_BRAKE_PRESSURE_1 5
#define CAN_DLC_BRAKE_PRESSURE_2 5
#define CAN_DLC_DISPLAY_STATUS 2
#define CAN_DLC_TELEMETRY_STATUS 8
#define CAN_DLC_BPS_TEMP_RAWV_AGGREGATE_ARR 4
#define CAN_DLC_SUPP_MEASUREMENTS_RAWV 5
#define CAN_DLC_SUPP_VICOR_MEASUREMENTS_RAWV 5
#define CAN_DLC_PEDAL_BRAKE_RAWV 5
#define CAN_DLC_PEDAL_ACCEL_RAWV 5


/* ================= Value Table Enums ================= */

typedef enum {
    BPS_STATUS_BPS_FAULT_OK = 0,
    BPS_STATUS_BPS_FAULT_OVERVOLTAGE = 1,
    BPS_STATUS_BPS_FAULT_UNDERVOLTAGE = 2,
    BPS_STATUS_BPS_FAULT_REGEN = 3,
    BPS_STATUS_BPS_FAULT_OVERTEMPERATURE = 4,
    BPS_STATUS_BPS_FAULT_ELCON = 5,
    BPS_STATUS_BPS_FAULT_ARRAY_PRECHARGE_TIMEOUT = 6,
    BPS_STATUS_BPS_FAULT_INTERNAL_WATCHDOG = 7,
    BPS_STATUS_BPS_FAULT_SEGMENT_WATCHDOG = 8,
    BPS_STATUS_BPS_FAULT_HV_PLUS_CONTACTOR_SENSE = 9,
    BPS_STATUS_BPS_FAULT_HV_MINUS_CONTACTOR_SENSE = 10,
    BPS_STATUS_BPS_FAULT_ARRAY_CONTACTOR_SENSE = 11,
    BPS_STATUS_BPS_FAULT_ARRAY_PCHG_CONTACTOR_SENSE = 12,
    BPS_STATUS_BPS_FAULT_ESTOP_1 = 13,
    BPS_STATUS_BPS_FAULT_ESTOP_2 = 14,
    BPS_STATUS_BPS_FAULT_ESTOP_3 = 15,
    BPS_STATUS_BPS_FAULT_CHARGING_OVERCURRENT = 16,
    BPS_STATUS_BPS_FAULT_DISCHARGING_OVERCURRENT = 17,
    BPS_STATUS_BPS_FAULT_AMPERES_WATCHDOG = 18,
} bps_status_bps_fault_e;

typedef enum {
    BPS_STATUS_BPS_CHARGE_OK_NOT_OK = 0,
    BPS_STATUS_BPS_CHARGE_OK_OK = 1,
} bps_status_bps_charge_ok_e;

typedef enum {
    BPS_STATUS_BPS_REGEN_OK_NOT_OK = 0,
    BPS_STATUS_BPS_REGEN_OK_OK = 1,
} bps_status_bps_regen_ok_e;

typedef enum {
    BPS_STATUS_HV_PLUS_CONTACTOR_STATE_OPEN = 0,
    BPS_STATUS_HV_PLUS_CONTACTOR_STATE_CLOSED = 1,
} bps_status_hv_plus_contactor_state_e;

typedef enum {
    BPS_STATUS_HV_MINUS_CONTACTOR_STATE_OPEN = 0,
    BPS_STATUS_HV_MINUS_CONTACTOR_STATE_CLOSED = 1,
} bps_status_hv_minus_contactor_state_e;

typedef enum {
    BPS_STATUS_ARRAY_CONTACTOR_STATE_OPEN = 0,
    BPS_STATUS_ARRAY_CONTACTOR_STATE_CLOSED = 1,
} bps_status_array_contactor_state_e;

typedef enum {
    BPS_STATUS_ARRAY_PRECHARGE_CONTACTOR_STATE_OPEN = 0,
    BPS_STATUS_ARRAY_PRECHARGE_CONTACTOR_STATE_CLOSED = 1,
} bps_status_array_precharge_contactor_state_e;

typedef enum {
    BPS_STATUS_BPS_SEGMENT0_STATUS_OK = 0,
    BPS_STATUS_BPS_SEGMENT0_STATUS_FAULT = 1,
} bps_status_bps_segment0_status_e;

typedef enum {
    BPS_STATUS_BPS_SEGMENT1_STATUS_OK = 0,
    BPS_STATUS_BPS_SEGMENT1_STATUS_FAULT = 1,
} bps_status_bps_segment1_status_e;

typedef enum {
    BPS_STATUS_BPS_SEGMENT2_STATUS_OK = 0,
    BPS_STATUS_BPS_SEGMENT2_STATUS_FAULT = 1,
} bps_status_bps_segment2_status_e;

typedef enum {
    BPS_STATUS_BPS_SEGMENT3_STATUS_OK = 0,
    BPS_STATUS_BPS_SEGMENT3_STATUS_FAULT = 1,
} bps_status_bps_segment3_status_e;

typedef enum {
    BPS_STATUS_BPS_SEGMENT4_STATUS_OK = 0,
    BPS_STATUS_BPS_SEGMENT4_STATUS_FAULT = 1,
} bps_status_bps_segment4_status_e;

typedef enum {
    BPS_STATUS_BPS_SEGMENT5_STATUS_OK = 0,
    BPS_STATUS_BPS_SEGMENT5_STATUS_FAULT = 1,
} bps_status_bps_segment5_status_e;

typedef enum {
    BPS_STATUS_BPS_SEGMENT6_STATUS_OK = 0,
    BPS_STATUS_BPS_SEGMENT6_STATUS_FAULT = 1,
} bps_status_bps_segment6_status_e;

typedef enum {
    BPS_STATUS_BPS_SEGMENT7_STATUS_OK = 0,
    BPS_STATUS_BPS_SEGMENT7_STATUS_FAULT = 1,
} bps_status_bps_segment7_status_e;

typedef enum {
    BPS_VOLTAGE_AGGREGATE_ARR_BPS_VOLTAGE_TAP_FAULT_OK = 0,
    BPS_VOLTAGE_AGGREGATE_ARR_BPS_VOLTAGE_TAP_FAULT_BQ_I2C_READ_ERROR = 1,
    BPS_VOLTAGE_AGGREGATE_ARR_BPS_VOLTAGE_TAP_FAULT_OUT_OF_BOUNDS = 2,
    BPS_VOLTAGE_AGGREGATE_ARR_BPS_VOLTAGE_TAP_FAULT_OVER_VOLTAGE = 3,
    BPS_VOLTAGE_AGGREGATE_ARR_BPS_VOLTAGE_TAP_FAULT_UNDER_VOLTAGE = 4,
    BPS_VOLTAGE_AGGREGATE_ARR_BPS_VOLTAGE_TAP_FAULT_MESSAGE_WATCHDOG = 5,
} bps_voltage_aggregate_arr_bps_voltage_tap_fault_e;

typedef enum {
    BPS_TEMPERATURE_AGGREGATE_ARR_BPS_TEMPERATURE_TAP_FAULT_OK = 0,
    BPS_TEMPERATURE_AGGREGATE_ARR_BPS_TEMPERATURE_TAP_FAULT_OUT_OF_BOUNDS_SHORT_TO_GND_ = 1,
    BPS_TEMPERATURE_AGGREGATE_ARR_BPS_TEMPERATURE_TAP_FAULT_OUT_OF_BOUNDS_SHORT_TO_VCC_ = 2,
    BPS_TEMPERATURE_AGGREGATE_ARR_BPS_TEMPERATURE_TAP_FAULT_DISCONNECTED = 3,
    BPS_TEMPERATURE_AGGREGATE_ARR_BPS_TEMPERATURE_TAP_FAULT_OVER_TEMPERATURE = 4,
    BPS_TEMPERATURE_AGGREGATE_ARR_BPS_TEMPERATURE_TAP_FAULT_CHARGE_OVER_TEMPERATURE = 5,
    BPS_TEMPERATURE_AGGREGATE_ARR_BPS_TEMPERATURE_TAP_FAULT_UNDER_TEMPERATURE = 6,
    BPS_TEMPERATURE_AGGREGATE_ARR_BPS_TEMPERATURE_TAP_FAULT_CHARGE_UNDER_TEMPERATURE = 7,
    BPS_TEMPERATURE_AGGREGATE_ARR_BPS_TEMPERATURE_TAP_FAULT_MESSAGE_WATCHDOG = 8,
} bps_temperature_aggregate_arr_bps_temperature_tap_fault_e;

typedef enum {
    CHARGERINTERFACE_STATUS_ELCON_WATCHDOG_NOT_OK = 0,
    CHARGERINTERFACE_STATUS_ELCON_WATCHDOG_OK = 1,
} chargerinterface_status_elcon_watchdog_e;

typedef enum {
    CHARGERINTERFACE_STATUS_ELCON_CMD_TIMEOUT_NOT_OK = 0,
    CHARGERINTERFACE_STATUS_ELCON_CMD_TIMEOUT_OK = 1,
} chargerinterface_status_elcon_cmd_timeout_e;

typedef enum {
    CHARGERINTERFACE_STATUS_ELCON_START_STATE_NOT_OK = 0,
    CHARGERINTERFACE_STATUS_ELCON_START_STATE_OK = 1,
} chargerinterface_status_elcon_start_state_e;

typedef enum {
    CHARGERINTERFACE_STATUS_ELCON_INPUT_VOL_ERROR_OK = 0,
    CHARGERINTERFACE_STATUS_ELCON_INPUT_VOL_ERROR_FAULT = 1,
} chargerinterface_status_elcon_input_vol_error_e;

typedef enum {
    CHARGERINTERFACE_STATUS_ELCON_OVER_TEMPERATURE_OK = 0,
    CHARGERINTERFACE_STATUS_ELCON_OVER_TEMPERATURE_FAULT = 1,
} chargerinterface_status_elcon_over_temperature_e;

typedef enum {
    CHARGERINTERFACE_STATUS_ELCON_HW_FAULT_OK = 0,
    CHARGERINTERFACE_STATUS_ELCON_HW_FAULT_FAULT = 1,
} chargerinterface_status_elcon_hw_fault_e;

typedef enum {
    VCU_STATUS_VCU_FAULT_NO_FAULT = 0,
    VCU_STATUS_VCU_FAULT_MOTOR_CONTACTOR_SENSE = 1,
    VCU_STATUS_VCU_FAULT_MOTOR_PCHG_CONTACTOR_SENSE = 2,
    VCU_STATUS_VCU_FAULT_MOTOR_PRECHARGE_TIMEOUT = 3,
    VCU_STATUS_VCU_FAULT_MOTOR_CONTROLLER_FAULT = 4,
    VCU_STATUS_VCU_FAULT_MOTOR_HV_OVERVOLTAGE = 5,
    VCU_STATUS_VCU_FAULT_MOTOR_HV_UNDERVOLTAGE = 6,
} vcu_status_vcu_fault_e;

typedef enum {
    VCU_STATUS_MOTOR_CONTACTOR_STATE_OPEN = 0,
    VCU_STATUS_MOTOR_CONTACTOR_STATE_CLOSED = 1,
} vcu_status_motor_contactor_state_e;

typedef enum {
    VCU_STATUS_MOTOR_PRECHARGE_CONTACTOR_STATE_OPEN = 0,
    VCU_STATUS_MOTOR_PRECHARGE_CONTACTOR_STATE_CLOSED = 1,
} vcu_status_motor_precharge_contactor_state_e;

typedef enum {
    VCU_STATUS_MOTOR_READY_NOT_OK = 0,
    VCU_STATUS_MOTOR_READY_OK = 1,
} vcu_status_motor_ready_e;

typedef enum {
    VCU_STATUS_VCU_DRIVER_INPUT_OK_NOT_OK = 0,
    VCU_STATUS_VCU_DRIVER_INPUT_OK_OK = 1,
} vcu_status_vcu_driver_input_ok_e;

typedef enum {
    VCU_STATUS_VCU_PEDALS_OK_NOT_OK = 0,
    VCU_STATUS_VCU_PEDALS_OK_OK = 1,
} vcu_status_vcu_pedals_ok_e;

typedef enum {
    VCU_STATUS_VCU_REGEN_OK_NOT_OK = 0,
    VCU_STATUS_VCU_REGEN_OK_OK = 1,
} vcu_status_vcu_regen_ok_e;

typedef enum {
    VCU_STATUS_VCU_REGEN_ACTIVE_INACTIVE = 0,
    VCU_STATUS_VCU_REGEN_ACTIVE_ACTIVE = 1,
} vcu_status_vcu_regen_active_e;

typedef enum {
    VCU_STATUS_VCU_STEERING_ANGLE_OK_NOT_OK = 0,
    VCU_STATUS_VCU_STEERING_ANGLE_OK_OK = 1,
} vcu_status_vcu_steering_angle_ok_e;

typedef enum {
    VCU_STATUS_VCU_MOTORCOMMANDSOURCE_VCU = 0,
    VCU_STATUS_VCU_MOTORCOMMANDSOURCE_TRITIUM_EXTERNAL = 1,
} vcu_status_vcu_motorcommandsource_e;

typedef enum {
    VCU_STATUS_VCU_FSM_STATE_INIT = 0,
    VCU_STATUS_VCU_FSM_STATE_FORWARD_DRIVE = 1,
    VCU_STATUS_VCU_FSM_STATE_NEUTRAL_DRIVE = 2,
    VCU_STATUS_VCU_FSM_STATE_REVERSE_DRIVE = 3,
    VCU_STATUS_VCU_FSM_STATE_REGEN = 4,
    VCU_STATUS_VCU_FSM_STATE_CRUISE_CONTROL = 5,
    VCU_STATUS_VCU_FSM_STATE_DISABLED = 6,
    VCU_STATUS_VCU_FSM_STATE_VEHICLE_NOT_READY = 7,
} vcu_status_vcu_fsm_state_e;

typedef enum {
    VCU_STATUS_VCU_FSM_IN_PCHG_OK_FALSE = 0,
    VCU_STATUS_VCU_FSM_IN_PCHG_OK_TRUE = 1,
} vcu_status_vcu_fsm_in_pchg_ok_e;

typedef enum {
    VCU_STATUS_VCU_FSM_IN_BRAKE_FALSE = 0,
    VCU_STATUS_VCU_FSM_IN_BRAKE_TRUE = 1,
} vcu_status_vcu_fsm_in_brake_e;

typedef enum {
    VCU_STATUS_VCU_FSM_IN_FORWARD_FALSE = 0,
    VCU_STATUS_VCU_FSM_IN_FORWARD_TRUE = 1,
} vcu_status_vcu_fsm_in_forward_e;

typedef enum {
    VCU_STATUS_VCU_FSM_IN_NEUTRAL_FALSE = 0,
    VCU_STATUS_VCU_FSM_IN_NEUTRAL_TRUE = 1,
} vcu_status_vcu_fsm_in_neutral_e;

typedef enum {
    VCU_STATUS_VCU_FSM_IN_REVERSE_FALSE = 0,
    VCU_STATUS_VCU_FSM_IN_REVERSE_TRUE = 1,
} vcu_status_vcu_fsm_in_reverse_e;

typedef enum {
    VCU_STATUS_VCU_FSM_IN_REGEN_RDY_FALSE = 0,
    VCU_STATUS_VCU_FSM_IN_REGEN_RDY_TRUE = 1,
} vcu_status_vcu_fsm_in_regen_rdy_e;

typedef enum {
    VCU_STATUS_VCU_FSM_IN_REGEN_ENABLE_FALSE = 0,
    VCU_STATUS_VCU_FSM_IN_REGEN_ENABLE_TRUE = 1,
} vcu_status_vcu_fsm_in_regen_enable_e;

typedef enum {
    VCU_STATUS_VCU_FSM_IN_CRUISE_REQ_FALSE = 0,
    VCU_STATUS_VCU_FSM_IN_CRUISE_REQ_TRUE = 1,
} vcu_status_vcu_fsm_in_cruise_req_e;

typedef enum {
    VCU_STATUS_VCU_FSM_IN_REGEN_REQ_FALSE = 0,
    VCU_STATUS_VCU_FSM_IN_REGEN_REQ_TRUE = 1,
} vcu_status_vcu_fsm_in_regen_req_e;

typedef enum {
    CONTROLS_STATUS_CONTROLS_LEADER_FAULT_OK = 0,
    CONTROLS_STATUS_CONTROLS_LEADER_FAULT_BOSCH_LWS_WATCHDOG = 1,
    CONTROLS_STATUS_CONTROLS_LEADER_FAULT_BPS_WATCHDOG = 2,
    CONTROLS_STATUS_CONTROLS_LEADER_FAULT_INVALID_DRIVER_INPUTS = 3,
    CONTROLS_STATUS_CONTROLS_LEADER_FAULT_LIGHTING_BOARD_FAULT = 4,
    CONTROLS_STATUS_CONTROLS_LEADER_FAULT_LIGHTING_BOARD_WATCHDOG = 5,
} controls_status_controls_leader_fault_e;

typedef enum {
    CONTROLS_STATUS_LIGHTINGBOARD_FRONT_STATUS_OK = 0,
    CONTROLS_STATUS_LIGHTINGBOARD_FRONT_STATUS_ADDR_LED_UNDERCURRENT = 1,
    CONTROLS_STATUS_LIGHTINGBOARD_FRONT_STATUS_LED0_UNDERCURRENT = 2,
    CONTROLS_STATUS_LIGHTINGBOARD_FRONT_STATUS_LED1_UNDERCURRENT = 3,
    CONTROLS_STATUS_LIGHTINGBOARD_FRONT_STATUS_ADDR_LED_OVERCURRENT = 4,
    CONTROLS_STATUS_LIGHTINGBOARD_FRONT_STATUS_LED0_OVERCURRENT = 5,
    CONTROLS_STATUS_LIGHTINGBOARD_FRONT_STATUS_LED1_OVERCURRENT = 6,
    CONTROLS_STATUS_LIGHTINGBOARD_FRONT_STATUS_LIGHT_COMMAND_WATCHDOG = 7,
    CONTROLS_STATUS_LIGHTINGBOARD_FRONT_STATUS_WATCHDOG = 8,
} controls_status_lightingboard_front_status_e;

typedef enum {
    CONTROLS_STATUS_LIGHTINGBOARD_LEFT_STATUS_OK = 0,
    CONTROLS_STATUS_LIGHTINGBOARD_LEFT_STATUS_ADDR_LED_UNDERCURRENT = 1,
    CONTROLS_STATUS_LIGHTINGBOARD_LEFT_STATUS_LED0_UNDERCURRENT = 2,
    CONTROLS_STATUS_LIGHTINGBOARD_LEFT_STATUS_LED1_UNDERCURRENT = 3,
    CONTROLS_STATUS_LIGHTINGBOARD_LEFT_STATUS_ADDR_LED_OVERCURRENT = 4,
    CONTROLS_STATUS_LIGHTINGBOARD_LEFT_STATUS_LED0_OVERCURRENT = 5,
    CONTROLS_STATUS_LIGHTINGBOARD_LEFT_STATUS_LED1_OVERCURRENT = 6,
    CONTROLS_STATUS_LIGHTINGBOARD_LEFT_STATUS_LIGHT_COMMAND_WATCHDOG = 7,
    CONTROLS_STATUS_LIGHTINGBOARD_LEFT_STATUS_WATCHDOG = 8,
} controls_status_lightingboard_left_status_e;

typedef enum {
    CONTROLS_STATUS_LIGHTINGBOARD_RIGHT_STATUS_OK = 0,
    CONTROLS_STATUS_LIGHTINGBOARD_RIGHT_STATUS_ADDR_LED_UNDERCURRENT = 1,
    CONTROLS_STATUS_LIGHTINGBOARD_RIGHT_STATUS_LED0_UNDERCURRENT = 2,
    CONTROLS_STATUS_LIGHTINGBOARD_RIGHT_STATUS_LED1_UNDERCURRENT = 3,
    CONTROLS_STATUS_LIGHTINGBOARD_RIGHT_STATUS_ADDR_LED_OVERCURRENT = 4,
    CONTROLS_STATUS_LIGHTINGBOARD_RIGHT_STATUS_LED0_OVERCURRENT = 5,
    CONTROLS_STATUS_LIGHTINGBOARD_RIGHT_STATUS_LED1_OVERCURRENT = 6,
    CONTROLS_STATUS_LIGHTINGBOARD_RIGHT_STATUS_LIGHT_COMMAND_WATCHDOG = 7,
    CONTROLS_STATUS_LIGHTINGBOARD_RIGHT_STATUS_WATCHDOG = 8,
} controls_status_lightingboard_right_status_e;

typedef enum {
    CONTROLS_STATUS_LIGHTINGBOARD_REAR_STATUS_OK = 0,
    CONTROLS_STATUS_LIGHTINGBOARD_REAR_STATUS_ADDR_LED_UNDERCURRENT = 1,
    CONTROLS_STATUS_LIGHTINGBOARD_REAR_STATUS_LED0_UNDERCURRENT = 2,
    CONTROLS_STATUS_LIGHTINGBOARD_REAR_STATUS_LED1_UNDERCURRENT = 3,
    CONTROLS_STATUS_LIGHTINGBOARD_REAR_STATUS_ADDR_LED_OVERCURRENT = 4,
    CONTROLS_STATUS_LIGHTINGBOARD_REAR_STATUS_LED0_OVERCURRENT = 5,
    CONTROLS_STATUS_LIGHTINGBOARD_REAR_STATUS_LED1_OVERCURRENT = 6,
    CONTROLS_STATUS_LIGHTINGBOARD_REAR_STATUS_LIGHT_COMMAND_WATCHDOG = 7,
    CONTROLS_STATUS_LIGHTINGBOARD_REAR_STATUS_WATCHDOG = 8,
} controls_status_lightingboard_rear_status_e;

typedef enum {
    CONTROLS_STATUS_LIGHTINGBOARD_CANOPY_STATUS_OK = 0,
    CONTROLS_STATUS_LIGHTINGBOARD_CANOPY_STATUS_ADDR_LED_UNDERCURRENT = 1,
    CONTROLS_STATUS_LIGHTINGBOARD_CANOPY_STATUS_LED0_UNDERCURRENT = 2,
    CONTROLS_STATUS_LIGHTINGBOARD_CANOPY_STATUS_LED1_UNDERCURRENT = 3,
    CONTROLS_STATUS_LIGHTINGBOARD_CANOPY_STATUS_ADDR_LED_OVERCURRENT = 4,
    CONTROLS_STATUS_LIGHTINGBOARD_CANOPY_STATUS_LED0_OVERCURRENT = 5,
    CONTROLS_STATUS_LIGHTINGBOARD_CANOPY_STATUS_LED1_OVERCURRENT = 6,
    CONTROLS_STATUS_LIGHTINGBOARD_CANOPY_STATUS_LIGHT_COMMAND_WATCHDOG = 7,
    CONTROLS_STATUS_LIGHTINGBOARD_CANOPY_STATUS_WATCHDOG = 8,
} controls_status_lightingboard_canopy_status_e;

typedef enum {
    PEDAL_STATUS_ACCELPEDAL_MAIN_FAULT_OK = 0,
    PEDAL_STATUS_ACCELPEDAL_MAIN_FAULT_FAULT = 1,
} pedal_status_accelpedal_main_fault_e;

typedef enum {
    PEDAL_STATUS_ACCELPEDAL_REDUNDANT_FAULT_OK = 0,
    PEDAL_STATUS_ACCELPEDAL_REDUNDANT_FAULT_FAULT = 1,
} pedal_status_accelpedal_redundant_fault_e;

typedef enum {
    PEDAL_STATUS_BRAKEPEDAL_MAIN_FAULT_OK = 0,
    PEDAL_STATUS_BRAKEPEDAL_MAIN_FAULT_FAULT = 1,
} pedal_status_brakepedal_main_fault_e;

typedef enum {
    PEDAL_STATUS_BRAKEPEDAL_REDUNDANT_FAULT_OK = 0,
    PEDAL_STATUS_BRAKEPEDAL_REDUNDANT_FAULT_FAULT = 1,
} pedal_status_brakepedal_redundant_fault_e;

typedef enum {
    PEDAL_STATUS_BRAKE_PRESSURE_1_FAULT_OK = 0,
    PEDAL_STATUS_BRAKE_PRESSURE_1_FAULT_FAULT = 1,
} pedal_status_brake_pressure_1_fault_e;

typedef enum {
    PEDAL_STATUS_BRAKE_PRESSURE_2_FAULT_OK = 0,
    PEDAL_STATUS_BRAKE_PRESSURE_2_FAULT_FAULT = 1,
} pedal_status_brake_pressure_2_fault_e;

typedef enum {
    DRIVER_INPUT_STATUS_IGNITION_ARRAY__ = 0,
    DRIVER_INPUT_STATUS_IGNITION_ARRAY_SELECTED = 1,
} driver_input_status_ignition_array_e;

typedef enum {
    DRIVER_INPUT_STATUS_IGNITION_MOTOR__ = 0,
    DRIVER_INPUT_STATUS_IGNITION_MOTOR_SELECTED = 1,
} driver_input_status_ignition_motor_e;

typedef enum {
    DRIVER_INPUT_STATUS_IGNITION_OFF__ = 0,
    DRIVER_INPUT_STATUS_IGNITION_OFF_SELECTED = 1,
} driver_input_status_ignition_off_e;

typedef enum {
    DRIVER_INPUT_STATUS_CRUISE_ENABLE_DISABLED = 0,
    DRIVER_INPUT_STATUS_CRUISE_ENABLE_ENABLED = 1,
} driver_input_status_cruise_enable_e;

typedef enum {
    DRIVER_INPUT_STATUS_CRUISE_SET__ = 0,
    DRIVER_INPUT_STATUS_CRUISE_SET_PRESSED = 1,
} driver_input_status_cruise_set_e;

typedef enum {
    DRIVER_INPUT_STATUS_GEAR_FORWARD__ = 0,
    DRIVER_INPUT_STATUS_GEAR_FORWARD_SELECTED = 1,
} driver_input_status_gear_forward_e;

typedef enum {
    DRIVER_INPUT_STATUS_GEAR_NEUTRAL__ = 0,
    DRIVER_INPUT_STATUS_GEAR_NEUTRAL_SELECTED = 1,
} driver_input_status_gear_neutral_e;

typedef enum {
    DRIVER_INPUT_STATUS_GEAR_REVERSE__ = 0,
    DRIVER_INPUT_STATUS_GEAR_REVERSE_SELECTED = 1,
} driver_input_status_gear_reverse_e;

typedef enum {
    DRIVER_INPUT_STATUS_HAZARD_PRESSED_OFF = 0,
    DRIVER_INPUT_STATUS_HAZARD_PRESSED_ON = 1,
} driver_input_status_hazard_pressed_e;

typedef enum {
    DRIVER_INPUT_STATUS_HORN_PRESSED_OFF = 0,
    DRIVER_INPUT_STATUS_HORN_PRESSED_ON = 1,
} driver_input_status_horn_pressed_e;

typedef enum {
    DRIVER_INPUT_STATUS_BLINKER_LEFT__ = 0,
    DRIVER_INPUT_STATUS_BLINKER_LEFT_SELECTED = 1,
} driver_input_status_blinker_left_e;

typedef enum {
    DRIVER_INPUT_STATUS_BLINKER_RIGHT__ = 0,
    DRIVER_INPUT_STATUS_BLINKER_RIGHT_SELECTED = 1,
} driver_input_status_blinker_right_e;

typedef enum {
    DRIVER_INPUT_STATUS_PUSHTOTALK_PRESSED__ = 0,
    DRIVER_INPUT_STATUS_PUSHTOTALK_PRESSED_PRESSED = 1,
} driver_input_status_pushtotalk_pressed_e;

typedef enum {
    DRIVER_INPUT_STATUS_REGEN_ACTIVATE__ = 0,
    DRIVER_INPUT_STATUS_REGEN_ACTIVATE_PRESSED = 1,
} driver_input_status_regen_activate_e;

typedef enum {
    DRIVER_INPUT_STATUS_REGEN_ENABLE_DISABLED = 0,
    DRIVER_INPUT_STATUS_REGEN_ENABLE_ENABLED = 1,
} driver_input_status_regen_enable_e;

typedef enum {
    MPPT_A_STATUS_MPPT_MODE_CONSTANT_INPUT_VOLTAGE = 0,
    MPPT_A_STATUS_MPPT_MODE_CONSTANT_INPUT_CURRENT = 1,
    MPPT_A_STATUS_MPPT_MODE_MINIMUM_INPUT_CURRENT = 2,
    MPPT_A_STATUS_MPPT_MODE_CONSTANT_OUTPUT_VOLTAGE = 3,
    MPPT_A_STATUS_MPPT_MODE_CONSTANT_OUTPUT_CURRENT = 4,
    MPPT_A_STATUS_MPPT_MODE_TEMPERATURE_DE_RATING = 5,
    MPPT_A_STATUS_MPPT_MODE_FAULT = 6,
} mppt_a_status_mppt_mode_e;

typedef enum {
    MPPT_A_STATUS_MPPT_FAULT_OK = 0,
    MPPT_A_STATUS_MPPT_FAULT_CONFIGURATION_ERROR = 1,
    MPPT_A_STATUS_MPPT_FAULT_INPUT_OVER_VOLTAGE = 2,
    MPPT_A_STATUS_MPPT_FAULT_OUTPUT_OVER_VOLTAGE = 3,
    MPPT_A_STATUS_MPPT_FAULT_OUTPUT_OVER_CURRENT = 4,
    MPPT_A_STATUS_MPPT_FAULT_INPUT_OVER_CURRENT = 5,
    MPPT_A_STATUS_MPPT_FAULT_INPUT_UNDER_CURRENT = 6,
    MPPT_A_STATUS_MPPT_FAULT_PHASE_OVER_CURRENT = 7,
    MPPT_A_STATUS_MPPT_FAULT_UNKNOWN_FAULT = 8,
} mppt_a_status_mppt_fault_e;

typedef enum {
    MPPT_A_STATUS_MPPT_ENABLED_DISABLED = 0,
    MPPT_A_STATUS_MPPT_ENABLED_ENABLED = 1,
    MPPT_A_STATUS_MPPT_ENABLED_ENABLED_CONSTANT_VOLTAGE = 2,
} mppt_a_status_mppt_enabled_e;

typedef enum {
    MPPT_A_SETMODE_MPPT_SET_ENABLE_DISABLED = 0,
    MPPT_A_SETMODE_MPPT_SET_ENABLE_ENABLED = 1,
    MPPT_A_SETMODE_MPPT_SET_ENABLE_ENABLED_CONSTANT_VOLTAGE = 2,
} mppt_a_setmode_mppt_set_enable_e;

typedef enum {
    MPPT_B_STATUS_MPPT_MODE_CONSTANT_INPUT_VOLTAGE = 0,
    MPPT_B_STATUS_MPPT_MODE_CONSTANT_INPUT_CURRENT = 1,
    MPPT_B_STATUS_MPPT_MODE_MINIMUM_INPUT_CURRENT = 2,
    MPPT_B_STATUS_MPPT_MODE_CONSTANT_OUTPUT_VOLTAGE = 3,
    MPPT_B_STATUS_MPPT_MODE_CONSTANT_OUTPUT_CURRENT = 4,
    MPPT_B_STATUS_MPPT_MODE_TEMPERATURE_DE_RATING = 5,
    MPPT_B_STATUS_MPPT_MODE_FAULT = 6,
} mppt_b_status_mppt_mode_e;

typedef enum {
    MPPT_B_STATUS_MPPT_FAULT_OK = 0,
    MPPT_B_STATUS_MPPT_FAULT_CONFIGURATION_ERROR = 1,
    MPPT_B_STATUS_MPPT_FAULT_INPUT_OVER_VOLTAGE = 2,
    MPPT_B_STATUS_MPPT_FAULT_OUTPUT_OVER_VOLTAGE = 3,
    MPPT_B_STATUS_MPPT_FAULT_OUTPUT_OVER_CURRENT = 4,
    MPPT_B_STATUS_MPPT_FAULT_INPUT_OVER_CURRENT = 5,
    MPPT_B_STATUS_MPPT_FAULT_INPUT_UNDER_CURRENT = 6,
    MPPT_B_STATUS_MPPT_FAULT_PHASE_OVER_CURRENT = 7,
    MPPT_B_STATUS_MPPT_FAULT_UNKNOWN_FAULT = 8,
} mppt_b_status_mppt_fault_e;

typedef enum {
    MPPT_B_STATUS_MPPT_ENABLED_DISABLED = 0,
    MPPT_B_STATUS_MPPT_ENABLED_ENABLED = 1,
    MPPT_B_STATUS_MPPT_ENABLED_ENABLED_CONSTANT_VOLTAGE = 2,
} mppt_b_status_mppt_enabled_e;

typedef enum {
    MPPT_B_SETMODE_MPPT_SET_ENABLE_DISABLED = 0,
    MPPT_B_SETMODE_MPPT_SET_ENABLE_ENABLED = 1,
    MPPT_B_SETMODE_MPPT_SET_ENABLE_ENABLED_CONSTANT_VOLTAGE = 2,
} mppt_b_setmode_mppt_set_enable_e;

typedef enum {
    MPPT_C_STATUS_MPPT_MODE_CONSTANT_INPUT_VOLTAGE = 0,
    MPPT_C_STATUS_MPPT_MODE_CONSTANT_INPUT_CURRENT = 1,
    MPPT_C_STATUS_MPPT_MODE_MINIMUM_INPUT_CURRENT = 2,
    MPPT_C_STATUS_MPPT_MODE_CONSTANT_OUTPUT_VOLTAGE = 3,
    MPPT_C_STATUS_MPPT_MODE_CONSTANT_OUTPUT_CURRENT = 4,
    MPPT_C_STATUS_MPPT_MODE_TEMPERATURE_DE_RATING = 5,
    MPPT_C_STATUS_MPPT_MODE_FAULT = 6,
} mppt_c_status_mppt_mode_e;

typedef enum {
    MPPT_C_STATUS_MPPT_FAULT_OK = 0,
    MPPT_C_STATUS_MPPT_FAULT_CONFIGURATION_ERROR = 1,
    MPPT_C_STATUS_MPPT_FAULT_INPUT_OVER_VOLTAGE = 2,
    MPPT_C_STATUS_MPPT_FAULT_OUTPUT_OVER_VOLTAGE = 3,
    MPPT_C_STATUS_MPPT_FAULT_OUTPUT_OVER_CURRENT = 4,
    MPPT_C_STATUS_MPPT_FAULT_INPUT_OVER_CURRENT = 5,
    MPPT_C_STATUS_MPPT_FAULT_INPUT_UNDER_CURRENT = 6,
    MPPT_C_STATUS_MPPT_FAULT_PHASE_OVER_CURRENT = 7,
    MPPT_C_STATUS_MPPT_FAULT_UNKNOWN_FAULT = 8,
} mppt_c_status_mppt_fault_e;

typedef enum {
    MPPT_C_STATUS_MPPT_ENABLED_DISABLED = 0,
    MPPT_C_STATUS_MPPT_ENABLED_ENABLED = 1,
    MPPT_C_STATUS_MPPT_ENABLED_ENABLED_CONSTANT_VOLTAGE = 2,
} mppt_c_status_mppt_enabled_e;

typedef enum {
    MPPT_C_SETMODE_MPPT_SET_ENABLE_DISABLED = 0,
    MPPT_C_SETMODE_MPPT_SET_ENABLE_ENABLED = 1,
    MPPT_C_SETMODE_MPPT_SET_ENABLE_ENABLED_CONSTANT_VOLTAGE = 2,
} mppt_c_setmode_mppt_set_enable_e;

typedef enum {
    SUPP_BATTERY_STATUS_SUPPLEMENTAL_BATTERY_FAULT_OK = 0,
    SUPP_BATTERY_STATUS_SUPPLEMENTAL_BATTERY_FAULT_UNDER_VOLTAGE = 1,
    SUPP_BATTERY_STATUS_SUPPLEMENTAL_BATTERY_FAULT_OVER_VOLTAGE = 2,
    SUPP_BATTERY_STATUS_SUPPLEMENTAL_BATTERY_FAULT_OVER_CURRENT_DISCHARGE_ = 3,
    SUPP_BATTERY_STATUS_SUPPLEMENTAL_BATTERY_FAULT_OVER_CURRENT_CHARGE_ = 4,
    SUPP_BATTERY_STATUS_SUPPLEMENTAL_BATTERY_FAULT_UNKNOWN_FAULT = 5,
} supp_battery_status_supplemental_battery_fault_e;

typedef enum {
    SUPP_CHARGING_STATUS_SUPPLEMENTAL_CHARGING_STATUS_CHARGE_DISABLED = 0,
    SUPP_CHARGING_STATUS_SUPPLEMENTAL_CHARGING_STATUS_CHARGE_COMPLETE = 1,
    SUPP_CHARGING_STATUS_SUPPLEMENTAL_CHARGING_STATUS_CHARGING_ = 2,
    SUPP_CHARGING_STATUS_SUPPLEMENTAL_CHARGING_STATUS_FAULT = 3,
} supp_charging_status_supplemental_charging_status_e;

typedef enum {
    PDU_STATUS_ARR_HSS_CHANNEL_IDX_VCU = 0,
    PDU_STATUS_ARR_HSS_CHANNEL_IDX_DASHBOARD = 1,
    PDU_STATUS_ARR_HSS_CHANNEL_IDX_HORN = 2,
    PDU_STATUS_ARR_HSS_CHANNEL_IDX_LIGHTS = 3,
    PDU_STATUS_ARR_HSS_CHANNEL_IDX_PI_DISPLAY = 4,
    PDU_STATUS_ARR_HSS_CHANNEL_IDX_PEDALS = 5,
    PDU_STATUS_ARR_HSS_CHANNEL_IDX_CAMERAS = 6,
    PDU_STATUS_ARR_HSS_CHANNEL_IDX_UNASSIGNED_0 = 7,
    PDU_STATUS_ARR_HSS_CHANNEL_IDX_BATTERY = 8,
    PDU_STATUS_ARR_HSS_CHANNEL_IDX_MPPTS = 9,
    PDU_STATUS_ARR_HSS_CHANNEL_IDX_MOTOR_CONTROLLER = 10,
    PDU_STATUS_ARR_HSS_CHANNEL_IDX_TELEMETRY_DATA_ACQ = 11,
    PDU_STATUS_ARR_HSS_CHANNEL_IDX_PUMP = 12,
    PDU_STATUS_ARR_HSS_CHANNEL_IDX_UNASSIGNED_1 = 13,
    PDU_STATUS_ARR_HSS_CHANNEL_IDX_UNASSIGNED_2 = 14,
    PDU_STATUS_ARR_HSS_CHANNEL_IDX_PDE_FANS = 15,
} pdu_status_arr_hss_channel_idx_e;

typedef enum {
    PDU_STATUS_ARR_HSS_FAULT_OK = 0,
    PDU_STATUS_ARR_HSS_FAULT_FAULT = 1,
} pdu_status_arr_hss_fault_e;

typedef enum {
    PDU_STATUS_ARR_HSS_ENABLED_DISABLED = 0,
    PDU_STATUS_ARR_HSS_ENABLED_ENABLED = 1,
} pdu_status_arr_hss_enabled_e;

typedef enum {
    PDU_SET_SWITCH_ARR_HSS_CHANNEL_IDX_VCU = 0,
    PDU_SET_SWITCH_ARR_HSS_CHANNEL_IDX_DASHBOARD = 1,
    PDU_SET_SWITCH_ARR_HSS_CHANNEL_IDX_HORN = 2,
    PDU_SET_SWITCH_ARR_HSS_CHANNEL_IDX_LIGHTS = 3,
    PDU_SET_SWITCH_ARR_HSS_CHANNEL_IDX_PI_DISPLAY = 4,
    PDU_SET_SWITCH_ARR_HSS_CHANNEL_IDX_PEDALS = 5,
    PDU_SET_SWITCH_ARR_HSS_CHANNEL_IDX_CAMERAS = 6,
    PDU_SET_SWITCH_ARR_HSS_CHANNEL_IDX_UNASSIGNED_0 = 7,
    PDU_SET_SWITCH_ARR_HSS_CHANNEL_IDX_BATTERY = 8,
    PDU_SET_SWITCH_ARR_HSS_CHANNEL_IDX_MPPTS = 9,
    PDU_SET_SWITCH_ARR_HSS_CHANNEL_IDX_MOTOR_CONTROLLER = 10,
    PDU_SET_SWITCH_ARR_HSS_CHANNEL_IDX_TELEMETRY_DATA_ACQ = 11,
    PDU_SET_SWITCH_ARR_HSS_CHANNEL_IDX_PUMP = 12,
    PDU_SET_SWITCH_ARR_HSS_CHANNEL_IDX_UNASSIGNED_1 = 13,
    PDU_SET_SWITCH_ARR_HSS_CHANNEL_IDX_UNASSIGNED_2 = 14,
    PDU_SET_SWITCH_ARR_HSS_CHANNEL_IDX_PDE_FANS = 15,
} pdu_set_switch_arr_hss_channel_idx_e;

typedef enum {
    PDU_SET_SWITCH_ARR_HSS_SETSWITCH_DO_NOTHING = 0,
    PDU_SET_SWITCH_ARR_HSS_SETSWITCH_TOGGLE_ON = 1,
    PDU_SET_SWITCH_ARR_HSS_SETSWITCH_TOGGLE_OFF = 2,
} pdu_set_switch_arr_hss_setswitch_e;

typedef enum {
    PDU_SET_CURRENT_LIMIT_ARR_HSS_CHANNEL_IDX_VCU = 0,
    PDU_SET_CURRENT_LIMIT_ARR_HSS_CHANNEL_IDX_DASHBOARD = 1,
    PDU_SET_CURRENT_LIMIT_ARR_HSS_CHANNEL_IDX_HORN = 2,
    PDU_SET_CURRENT_LIMIT_ARR_HSS_CHANNEL_IDX_LIGHTS = 3,
    PDU_SET_CURRENT_LIMIT_ARR_HSS_CHANNEL_IDX_PI_DISPLAY = 4,
    PDU_SET_CURRENT_LIMIT_ARR_HSS_CHANNEL_IDX_PEDALS = 5,
    PDU_SET_CURRENT_LIMIT_ARR_HSS_CHANNEL_IDX_CAMERAS = 6,
    PDU_SET_CURRENT_LIMIT_ARR_HSS_CHANNEL_IDX_UNASSIGNED_0 = 7,
    PDU_SET_CURRENT_LIMIT_ARR_HSS_CHANNEL_IDX_BATTERY = 8,
    PDU_SET_CURRENT_LIMIT_ARR_HSS_CHANNEL_IDX_MPPTS = 9,
    PDU_SET_CURRENT_LIMIT_ARR_HSS_CHANNEL_IDX_MOTOR_CONTROLLER = 10,
    PDU_SET_CURRENT_LIMIT_ARR_HSS_CHANNEL_IDX_TELEMETRY_DATA_ACQ = 11,
    PDU_SET_CURRENT_LIMIT_ARR_HSS_CHANNEL_IDX_PUMP = 12,
    PDU_SET_CURRENT_LIMIT_ARR_HSS_CHANNEL_IDX_UNASSIGNED_1 = 13,
    PDU_SET_CURRENT_LIMIT_ARR_HSS_CHANNEL_IDX_UNASSIGNED_2 = 14,
    PDU_SET_CURRENT_LIMIT_ARR_HSS_CHANNEL_IDX_PDE_FANS = 15,
} pdu_set_current_limit_arr_hss_channel_idx_e;

typedef enum {
    PUMP_STATUS_PUMP_FAULT_OK = 0,
    PUMP_STATUS_PUMP_FAULT_FAULT = 1,
} pump_status_pump_fault_e;

typedef enum {
    LV_CARRIER_STATUS_LTC4421_HVDCDC_SELECTED__ = 0,
    LV_CARRIER_STATUS_LTC4421_HVDCDC_SELECTED_SELECTED = 1,
} lv_carrier_status_ltc4421_hvdcdc_selected_e;

typedef enum {
    LV_CARRIER_STATUS_LTC4421_HVDCDC_FAULT_OK = 0,
    LV_CARRIER_STATUS_LTC4421_HVDCDC_FAULT_FAULT = 1,
} lv_carrier_status_ltc4421_hvdcdc_fault_e;

typedef enum {
    LV_CARRIER_STATUS_LTC4421_HVDCDC_VALID_NOT_OK = 0,
    LV_CARRIER_STATUS_LTC4421_HVDCDC_VALID_OK = 1,
} lv_carrier_status_ltc4421_hvdcdc_valid_e;

typedef enum {
    LV_CARRIER_STATUS_LTC4421_SUPPBATT_SELECTED__ = 0,
    LV_CARRIER_STATUS_LTC4421_SUPPBATT_SELECTED_SELECTED = 1,
} lv_carrier_status_ltc4421_suppbatt_selected_e;

typedef enum {
    LV_CARRIER_STATUS_LTC4421_SUPPBATT_FAULT_OK = 0,
    LV_CARRIER_STATUS_LTC4421_SUPPBATT_FAULT_FAULT = 1,
} lv_carrier_status_ltc4421_suppbatt_fault_e;

typedef enum {
    LV_CARRIER_STATUS_LTC4421_SUPPBATT_VALID_NOT_OK = 0,
    LV_CARRIER_STATUS_LTC4421_SUPPBATT_VALID_OK = 1,
} lv_carrier_status_ltc4421_suppbatt_valid_e;

typedef enum {
    LV_CARRIER_STATUS_LV_EN_SUPPLEMENTALBATTERY_DISABLED = 0,
    LV_CARRIER_STATUS_LV_EN_SUPPLEMENTALBATTERY_ENABLED = 1,
} lv_carrier_status_lv_en_supplementalbattery_e;

typedef enum {
    LV_CARRIER_STATUS_LV_EN_POWERSUPPLY_DISABLED = 0,
    LV_CARRIER_STATUS_LV_EN_POWERSUPPLY_ENABLED = 1,
} lv_carrier_status_lv_en_powersupply_e;

typedef enum {
    DISPLAY_STATUS_CAMERA_STATUS_BACKUP_NOT_OK = 0,
    DISPLAY_STATUS_CAMERA_STATUS_BACKUP_OK = 1,
} display_status_camera_status_backup_e;

typedef enum {
    DISPLAY_STATUS_CAMERA_STATUS_LEFT_NOT_OK = 0,
    DISPLAY_STATUS_CAMERA_STATUS_LEFT_OK = 1,
} display_status_camera_status_left_e;

typedef enum {
    DISPLAY_STATUS_CAMERA_STATUS_RIGHT_NOT_OK = 0,
    DISPLAY_STATUS_CAMERA_STATUS_RIGHT_OK = 1,
} display_status_camera_status_right_e;

/* ================= Message Structs ================= */

typedef struct {
    uint8_t BPS_Fault;
    uint8_t BPS_Charge_OK;
    uint8_t BPS_Regen_OK;
    uint8_t HV_Plus_Contactor_State;
    uint8_t HV_Minus_Contactor_State;
    uint8_t Array_Contactor_State;
    uint8_t Array_Precharge_Contactor_State;
    uint32_t Main_Battery_Voltage;
    int16_t Main_Battery_Avg_Temperature;
    uint8_t BPS_Segment0_Status;
    uint8_t BPS_Segment1_Status;
    uint8_t BPS_Segment2_Status;
    uint8_t BPS_Segment3_Status;
    uint8_t BPS_Segment4_Status;
    uint8_t BPS_Segment5_Status;
    uint8_t BPS_Segment6_Status;
    uint8_t BPS_Segment7_Status;
} bps_status_t;

typedef struct {
    uint8_t BPS_Tap_idx;
    uint16_t BPS_Voltage_Tap_Data;
    uint8_t BPS_Voltage_Tap_Fault;
    uint16_t BPS_Voltage_Tap_Age;
} bps_voltage_aggregate_arr_t;

typedef struct {
    uint8_t BPS_Tap_idx;
    int32_t BPS_Temperature_Tap_Data;
    uint8_t BPS_Temperature_Tap_Fault;
    uint16_t BPS_Temperature_Tap_Age;
    uint8_t FrameID_BPS_Temperature;
} bps_temperature_aggregate_arr_t;

typedef struct {
    uint16_t Elcon_VOL_OUT;
    uint16_t Elcon_CUR_OUT;
    uint8_t Elcon_Watchdog;
    uint8_t Elcon_CMD_TIMEOUT;
    uint8_t Elcon_START_STATE;
    uint8_t Elcon_INPUT_VOL_ERROR;
    uint8_t Elcon_OVER_TEMPERATURE;
    uint8_t Elcon_HW_FAULT;
} chargerinterface_status_t;

typedef struct {
    uint8_t VCU_Fault;
    uint8_t Motor_Contactor_State;
    uint8_t Motor_Precharge_Contactor_State;
    uint8_t Motor_Ready;
    uint8_t VCU_Driver_Input_OK;
    uint8_t VCU_Pedals_OK;
    uint8_t VCU_Regen_OK;
    uint8_t VCU_Regen_Active;
    uint8_t VCU_Steering_Angle_OK;
    uint8_t VCU_MotorCommandSource;
    uint8_t VCU_FSM_State;
    uint8_t VCU_FSM_IN_PCHG_OK;
    uint8_t VCU_FSM_IN_BRAKE;
    uint8_t VCU_FSM_IN_FORWARD;
    uint8_t VCU_FSM_IN_NEUTRAL;
    uint8_t VCU_FSM_IN_REVERSE;
    uint8_t VCU_FSM_IN_REGEN_RDY;
    uint8_t VCU_FSM_IN_REGEN_ENABLE;
    uint8_t VCU_FSM_IN_CRUISE_REQ;
    uint8_t VCU_FSM_IN_REGEN_REQ;
} vcu_status_t;

typedef struct {
    uint8_t Controls_Leader_Fault;
    uint8_t LightingBoard_Front_Status;
    uint8_t LightingBoard_Left_Status;
    uint8_t LightingBoard_Right_Status;
    uint8_t LightingBoard_Rear_Status;
    uint8_t LightingBoard_Canopy_Status;
} controls_status_t;

typedef struct {
    uint32_t BPS_Precharge_Battery_Voltage;
    uint32_t BPS_Precharge_Array_Voltage;
} bps_precharge_voltages_t;

typedef struct {
    uint32_t VCU_Precharge_Battery_Voltage;
    uint32_t VCU_Precharge_Motor_Voltage;
} vcu_precharge_voltages_t;

typedef struct {
    uint8_t AccelPedal_Main_Pos;
    uint8_t AccelPedal_Redundant_Pos;
    uint8_t BrakePedal_Main_Pos;
    uint8_t BrakePedal_Redundant_Pos;
    uint8_t AccelPedal_Main_Fault;
    uint8_t AccelPedal_Redundant_Fault;
    uint8_t BrakePedal_Main_Fault;
    uint8_t BrakePedal_Redundant_Fault;
    uint8_t Brake_Pressure_1_Fault;
    uint8_t Brake_Pressure_2_Fault;
    uint8_t FrameID_Pedals;
} pedal_status_t;

typedef struct {
    uint8_t Ignition_Array;
    uint8_t Ignition_Motor;
    uint8_t Ignition_Off;
    uint8_t Cruise_Enable;
    uint8_t Cruise_Set;
    uint8_t Gear_Forward;
    uint8_t Gear_Neutral;
    uint8_t Gear_Reverse;
    uint8_t Hazard_Pressed;
    uint8_t Horn_Pressed;
    uint8_t Blinker_Left;
    uint8_t Blinker_Right;
    uint8_t PushToTalk_Pressed;
    uint8_t Regen_Activate;
    uint8_t Regen_Enable;
} driver_input_status_t;

typedef struct {
    int16_t MPPT_Input_Voltage;
    int16_t MPPT_Input_Current;
    int16_t MPPT_Output_Voltage;
    int16_t MPPT_Output_Current;
} mppt_a_powermeasurements_t;

typedef struct {
    uint8_t MPPT_Mode;
    uint8_t MPPT_Fault;
    int8_t MPPT_Enabled;
    int8_t MPPT_AmbientTemperature;
    int8_t MPPT_HeatsinkTemperature;
} mppt_a_status_t;

typedef struct {
    uint8_t MPPT_Sweep_Data_cnt;
    int16_t MPPT_Sweep_Data_Current;
    int16_t MPPT_Sweep_Data_Voltage;
} mppt_a_sweepmeasurements_t;

typedef struct {
    uint8_t MPPT_Set_Enable;
} mppt_a_setmode_t;

typedef struct {
    int16_t MPPT_Set_Sweep_Start;
    int16_t MPPT_Set_Sweep_End;
    uint8_t MPPT_Set_Sweep_Size;
} mppt_a_performsweep_t;

typedef struct {
    int16_t MPPT_SetOutputVoltageLimit;
} mppt_a_setoutputvoltagelimit_t;

typedef struct {
    int16_t MPPT_SetOutputCurrentLimit;
} mppt_a_setoutputcurrentlimit_t;

typedef struct {
    int16_t MPPT_Input_Voltage;
    int16_t MPPT_Input_Current;
    int16_t MPPT_Output_Voltage;
    int16_t MPPT_Output_Current;
} mppt_b_powermeasurements_t;

typedef struct {
    uint8_t MPPT_Mode;
    uint8_t MPPT_Fault;
    int8_t MPPT_Enabled;
    int8_t MPPT_AmbientTemperature;
    int8_t MPPT_HeatsinkTemperature;
} mppt_b_status_t;

typedef struct {
    uint8_t MPPT_Sweep_Data_cnt;
    int16_t MPPT_Sweep_Data_Current;
    int16_t MPPT_Sweep_Data_Voltage;
} mppt_b_sweepmeasurements_t;

typedef struct {
    uint8_t MPPT_Set_Enable;
} mppt_b_setmode_t;

typedef struct {
    int16_t MPPT_Set_Sweep_Start;
    int16_t MPPT_Set_Sweep_End;
    uint8_t MPPT_Set_Sweep_Size;
} mppt_b_performsweep_t;

typedef struct {
    int16_t MPPT_SetOutputVoltageLimit;
} mppt_b_setoutputvoltagelimit_t;

typedef struct {
    int16_t MPPT_SetOutputCurrentLimit;
} mppt_b_setoutputcurrentlimit_t;

typedef struct {
    int16_t MPPT_Input_Voltage;
    int16_t MPPT_Input_Current;
    int16_t MPPT_Output_Voltage;
    int16_t MPPT_Output_Current;
} mppt_c_powermeasurements_t;

typedef struct {
    uint8_t MPPT_Mode;
    uint8_t MPPT_Fault;
    int8_t MPPT_Enabled;
    int8_t MPPT_AmbientTemperature;
    int8_t MPPT_HeatsinkTemperature;
} mppt_c_status_t;

typedef struct {
    uint8_t MPPT_Sweep_Data_cnt;
    int16_t MPPT_Sweep_Data_Current;
    int16_t MPPT_Sweep_Data_Voltage;
} mppt_c_sweepmeasurements_t;

typedef struct {
    uint8_t MPPT_Set_Enable;
} mppt_c_setmode_t;

typedef struct {
    int16_t MPPT_Set_Sweep_Start;
    int16_t MPPT_Set_Sweep_End;
    uint8_t MPPT_Set_Sweep_Size;
} mppt_c_performsweep_t;

typedef struct {
    int16_t MPPT_SetOutputVoltageLimit;
} mppt_c_setoutputvoltagelimit_t;

typedef struct {
    int16_t MPPT_SetOutputCurrentLimit;
} mppt_c_setoutputcurrentlimit_t;

typedef struct {
    uint8_t Supplemental_Battery_Fault;
    uint16_t Supplemental_Battery_Voltage;
    int16_t Supplemental_Battery_Current;
    uint8_t FrameID_Supp;
} supp_battery_status_t;

typedef struct {
    uint8_t Supplemental_Charging_Status;
    uint16_t Supplemental_Vicor_Voltage;
    int16_t Supplemental_Vicor_Current;
    uint8_t FrameID_Supp_Vicor;
} supp_charging_status_t;

typedef struct {
    uint8_t HSS_Channel_idx;
    uint8_t HSS_Fault;
    uint8_t HSS_Enabled;
    uint16_t HSS_Measured_Voltage;
    uint16_t HSS_Measured_Current;
} pdu_status_arr_t;

typedef struct {
    uint8_t HSS_Channel_idx;
    uint8_t HSS_SetSwitch;
} pdu_set_switch_arr_t;

typedef struct {
    uint8_t HSS_Channel_idx;
    uint16_t HSS_Current_Limit;
} pdu_set_current_limit_arr_t;

typedef struct {
    uint8_t Pump_DutyCycle;
    uint8_t Pump_Fault;
    uint16_t FlowRate_1;
    uint16_t FlowRate_2;
} pump_status_t;

typedef struct {
    int16_t Coolant_Temperature_1;
    int16_t Coolant_Temperature_2;
} coolant_temperature_t;

typedef struct {
    uint16_t Radiator_Fan_Speed_Measurement_1;
    uint16_t Radiator_Fan_Speed_Measurement_2;
    uint16_t Radiator_Fan_Speed_Target_1;
    uint16_t Radiator_Fan_Speed_Target_2;
} radiator_fanspeed_t;

typedef struct {
    uint8_t LTC4421_HVDCDC_Selected;
    uint8_t LTC4421_HVDCDC_Fault;
    uint8_t LTC4421_HVDCDC_Valid;
    uint8_t LTC4421_SuppBatt_Selected;
    uint8_t LTC4421_SuppBatt_Fault;
    uint8_t LTC4421_SuppBatt_Valid;
    uint8_t LV_EN_SupplementalBattery;
    uint8_t LV_EN_PowerSupply;
} lv_carrier_status_t;

typedef struct {
    uint16_t Brake_Pressure;
    uint16_t Brake_Pressure_RawV;
    uint8_t FrameID_Pedals;
} brake_pressure_1_t;

typedef struct {
    uint16_t Brake_Pressure;
    uint16_t Brake_Pressure_RawV;
    uint8_t FrameID_Pedals;
} brake_pressure_2_t;

typedef struct {
    uint8_t Display_FrameRate;
    uint8_t Camera_Status_Backup;
    uint8_t Camera_Status_Left;
    uint8_t Camera_Status_Right;
} display_status_t;

typedef struct {
} telemetry_status_t;

typedef struct {
    uint8_t BPS_Tap_idx;
    uint16_t BPS_Temperature_Tap_RawV;
    uint8_t FrameID_BPS_Temperature;
} bps_temp_rawv_aggregate_arr_t;

typedef struct {
    uint16_t Supp_Battery_Voltage_RawV;
    int16_t Supp_Battery_Current_RawV;
    uint8_t FrameID_Supp;
} supp_measurements_rawv_t;

typedef struct {
    uint16_t Supp_Vicor_Voltage_RawV;
    int16_t Supp_Vicor_Current_RawV;
    uint8_t FrameID_Supp_Vicor;
} supp_vicor_measurements_rawv_t;

typedef struct {
    uint16_t BrakePedal_Main_RawV;
    uint16_t BrakePedal_Redundant_RawV;
    uint8_t FrameID_Pedals;
} pedal_brake_rawv_t;

typedef struct {
    uint16_t AccelPedal_Main_RawV;
    uint16_t AccelPedal_Redundant_RawV;
    uint8_t FrameID_Pedals;
} pedal_accel_rawv_t;