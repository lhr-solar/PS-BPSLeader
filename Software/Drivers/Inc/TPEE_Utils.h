#pragma once
#include "CANbus.h"

/** * @brief Disables all MPPTs by sending the set mode command to each MPPT with a mode of 0 (disabled).
 * @param delay_ms          Wait time for CAN message to be sent before returning
 * @return can_status_t     Returns CAN_OK if the command was successful, or CAN_ERR if there was an error.
 */
can_status_t disableAllMPPTs(TickType_t delay_ms);

/** * @brief Enables all MPPTs by sending the set mode command to each MPPT with a mode of 1 (enabled).
 * @param delay_ms          Wait time for CAN message to be sent before returning
 * @return can_status_t     Returns CAN_OK if the command was successful, or CAN_ERR if there was an error.
 */
can_status_t enableAllMPPTs(TickType_t delay_ms);
