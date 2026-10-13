#include "stm32g4xx_hal.h"

/*
 * BQ79616 wake test.
 *
 * Sends one WAKE ping: PC10 (MCU TX -> BQ79616 RX) held low for 2.5 ms,
 * then released high. No UART needed; the ping is just a GPIO pulse.
 *
 * Wiring: PC10 -> EVM RX, MCU GND -> EVM GND.
 * Runs on the 16 MHz HSI reset clock; no clock config required.
 *
 * In GDB: `p wake_sent` reads 1 once the ping has gone out.
 */

#define WAKE_PORT       GPIOC
#define WAKE_PIN        GPIO_PIN_10
#define WAKE_PING_US    2500U          /* SLVAE86B: 2.5 ms active-low */

volatile uint32_t wake_sent = 0;

/* Weak so it won't clash if Embedded-Sharepoint already defines one */
__attribute__((weak)) void Error_Handler(void)
{
    __disable_irq();
    while (1) { }
}

static void delay_us(uint32_t us)
{
    uint32_t start  = DWT->CYCCNT;
    uint32_t cycles = us * (SystemCoreClock / 1000000U);
    while ((DWT->CYCCNT - start) < cycles) { }
}

int main(void)
{
    GPIO_InitTypeDef init;

    HAL_Init();

    /* DWT cycle counter for the microsecond delay */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    /* PC10 as push-pull output, preloaded HIGH so it never glitches low */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    HAL_GPIO_WritePin(WAKE_PORT, WAKE_PIN, GPIO_PIN_SET);

    init.Pin       = WAKE_PIN;
    init.Mode      = GPIO_MODE_OUTPUT_PP;
    init.Pull      = GPIO_NOPULL;
    init.Speed     = GPIO_SPEED_FREQ_LOW;
    init.Alternate = 0;
    HAL_GPIO_Init(WAKE_PORT, &init);

    delay_us(10000U);                                      /* let the line sit idle-high */

    HAL_GPIO_WritePin(WAKE_PORT, WAKE_PIN, GPIO_PIN_RESET); /* WAKE ping */
    delay_us(WAKE_PING_US);
    HAL_GPIO_WritePin(WAKE_PORT, WAKE_PIN, GPIO_PIN_SET);

    wake_sent = 1;

    while (1) { }                                          /* hold RX idle-high */
    return 0;
}