#include "DebugPrintf.h"

// UART that debug printf owns. PC10/PC11 are only configured for this one.
static UART_HandleTypeDef *debug_huart = NULL;

void HAL_UART_MspGPIOInit(UART_HandleTypeDef *huart) {

    // Other UARTs (e.g. the BQ on USART3 PB10/PB11) configure their own pins
    if (huart != debug_huart) {
        return;
    }

    GPIO_InitTypeDef init = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();

    init.Pin = GPIO_PIN_10|GPIO_PIN_11;
    init.Mode = GPIO_MODE_AF_PP;
    init.Pull = GPIO_NOPULL;
    init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    init.Alternate = (huart->Instance == UART4) ? GPIO_AF5_UART4 : GPIO_AF7_USART3;
    HAL_GPIO_Init(GPIOC, &init);

}

void debugPrintf_init_uart(UART_HandleTypeDef *huart){

    if (huart != husart3 && huart != huart4) {
        Error_Handler();
    }

    if (huart == husart3) {
        RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
        PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART3;
        PeriphClkInit.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
        {
            Error_Handler();
        }
    }
    // UART4 kernel clock defaults to PCLK1 at reset

    debug_huart = huart;

    huart->Init.BaudRate = 115200;
    huart->Init.WordLength = UART_WORDLENGTH_8B;
    huart->Init.StopBits = UART_STOPBITS_1;
    huart->Init.Parity = UART_PARITY_NONE;
    huart->Init.Mode = UART_MODE_TX_RX;
    huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart->Init.OverSampling = UART_OVERSAMPLING_16;

    // printf_init() -> uart_init() -> HAL_UART_MspGPIOInit() sets up the pins
    printf_init(huart);
}

void debugPrintf_init(){
    debugPrintf_init_uart(husart3);
}

