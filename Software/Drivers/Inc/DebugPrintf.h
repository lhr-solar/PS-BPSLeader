#pragma once

#include "UART.h"
#include "projdefs.h"
#include "stm32xx_hal.h"
#include "printf.h"

/** * @brief Initializes the hardware and software configuration for debug printing.
 * @note This configures the underlying UART peripheral and links it 
 * to the printf stream to enable 
 * console output over a serial connection.
 */
void debugPrintf_init();

/**
 * @brief Same as debugPrintf_init(), but on a chosen UART.
 * @param huart husart3 (PC10/PC11, AF7) or huart4 (PC10/PC11, AF5).
 *              Anything else calls Error_Handler().
 * @note Use huart4 when USART3 is needed for something else (e.g. the BQ).
 */
void debugPrintf_init_uart(UART_HandleTypeDef *huart);
