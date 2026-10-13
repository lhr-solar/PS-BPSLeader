#include "bq.h"
#include <string.h>

static bq_init_struct_t s_cfg;
static CRC_HandleTypeDef s_hcrc;
static TIM_HandleTypeDef s_htim;
static bool s_initialised = false;

    
static bq_status_t gpio_clk_enable(GPIO_TypeDef *port)
{
    if (port == GPIOA)      { __HAL_RCC_GPIOA_CLK_ENABLE(); }
    else if (port == GPIOB) { __HAL_RCC_GPIOB_CLK_ENABLE(); }
    else if (port == GPIOC) { __HAL_RCC_GPIOC_CLK_ENABLE(); }
    else if (port == GPIOD) { __HAL_RCC_GPIOD_CLK_ENABLE(); }
    else if (port == GPIOE) { __HAL_RCC_GPIOE_CLK_ENABLE(); }
    else if (port == GPIOF) { __HAL_RCC_GPIOF_CLK_ENABLE(); }
    else if (port == GPIOG) { __HAL_RCC_GPIOG_CLK_ENABLE(); }
    else return BQ_ERR;

    return BQ_OK;
}

static bq_status_t wake_tim_clk_enable(TIM_TypeDef *tim)
{
    if (tim == TIM2)      { __HAL_RCC_TIM2_CLK_ENABLE(); }
    else if (tim == TIM3) { __HAL_RCC_TIM3_CLK_ENABLE(); }
    else if (tim == TIM4) { __HAL_RCC_TIM4_CLK_ENABLE(); }
    else if (tim == TIM5) { __HAL_RCC_TIM5_CLK_ENABLE(); }
    else return BQ_ERR;

    return BQ_OK;
}

/* Hardware CRC unit, set up for the BQ frame CRC: CRC-16-IBM */
static bq_status_t crc_init(void)
{
    __HAL_RCC_CRC_CLK_ENABLE();

    s_hcrc.Instance                     = CRC;
    s_hcrc.Init.DefaultPolynomialUse    = DEFAULT_POLYNOMIAL_DISABLE;
    s_hcrc.Init.GeneratingPolynomial    = 0x8005U;
    s_hcrc.Init.CRCLength               = CRC_POLYLENGTH_16B;
    s_hcrc.Init.DefaultInitValueUse     = DEFAULT_INIT_VALUE_DISABLE;
    s_hcrc.Init.InitValue               = 0xFFFFU;
    s_hcrc.Init.InputDataInversionMode  = CRC_INPUTDATA_INVERSION_BYTE;
    s_hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_ENABLE;
    s_hcrc.InputDataFormat              = CRC_INPUTDATA_FORMAT_BYTES;

    if (HAL_CRC_Init(&s_hcrc) != HAL_OK) {
        return BQ_ERR;
    }

    return BQ_OK;
}

/* CRC over len bytes. Running it across a whole frame including its CRC
 * gives 0 when the frame is intact (Section 7.3.2.1.1.2). */
static uint16_t bq_crc16(const uint8_t *buf, uint32_t len)
{
    /* With FORMAT_BYTES, len is in bytes despite the uint32_t* signature */
    return (uint16_t)HAL_CRC_Calculate(&s_hcrc, (uint32_t *)buf, len);
}

/* Drop any stale bytes (noise, the rest of a failed response) so the next
 * response frame starts at its INIT byte. */
static void bq_rx_flush(void)
{
    uint8_t discard;

    while (uart_recv(s_cfg.huart, &discard, 1U, 0U) == UART_OK) { }
}

/* WAKE ping generator. While a ping is sent, tx_pin is switched to the timer 
    One-pulse mode, PWM mode 2, active low */
static bq_status_t wake_tim_init(void)
{
    TIM_OC_InitTypeDef oc = {0};
    uint32_t tim_clk = HAL_RCC_GetPCLK1Freq();

    if (wake_tim_clk_enable(s_cfg.wake_tim) != BQ_OK) {
        return BQ_ERR;
    }

    /* APB1 timer kernel clock is 2x PCLK1 unless the APB1 prescaler is 1 */
    if ((RCC->CFGR & RCC_CFGR_PPRE1) != RCC_HCLK_DIV1) {
        tim_clk *= 2U;
    }
    if (tim_clk < BQ_WAKE_TIM_HZ || (tim_clk / BQ_WAKE_TIM_HZ) > 0x10000U) {
        return BQ_ERR;
    }

    s_htim.Instance               = s_cfg.wake_tim;
    s_htim.Init.Prescaler         = (tim_clk / BQ_WAKE_TIM_HZ) - 1U;
    s_htim.Init.CounterMode       = TIM_COUNTERMODE_UP;
    s_htim.Init.Period            = BQ_WAKE_PING_US;   /* ARR, with CCR = 1 gives BQ_WAKE_PING_US ticks low */
    s_htim.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    s_htim.Init.RepetitionCounter = 0U;
    s_htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (HAL_TIM_OnePulse_Init(&s_htim, TIM_OPMODE_SINGLE) != HAL_OK) {
        return BQ_ERR;
    }

    oc.OCMode     = TIM_OCMODE_PWM2;
    oc.Pulse      = 1U;     /* CCR: one tick of delay so CNT = 0 is the idle-high state */
    oc.OCPolarity = TIM_OCPOLARITY_LOW;
    oc.OCFastMode = TIM_OCFAST_DISABLE;

    if (HAL_TIM_PWM_ConfigChannel(&s_htim, &oc, s_cfg.wake_tim_channel) != HAL_OK) {
        return BQ_ERR;
    }

    /* Enable the channel output now, while tx_pin is still routed elsewhere,
     * so it is already driving high by the time the pin is switched to it. */
    TIM_CCxChannelCmd(s_htim.Instance, s_cfg.wake_tim_channel, TIM_CCx_ENABLE);

    return BQ_OK;
}

// inits the peripherals for the bq chip
bq_status_t bq_init(const bq_init_struct_t init) {
    GPIO_InitTypeDef gpio = {0};

    if (init.huart == NULL || init.tx_rx_port == NULL || init.fault_port == NULL || init.wake_tim == NULL) {
        return BQ_ERR;
    }
    if (init.wake_tim_channel != TIM_CHANNEL_1 && init.wake_tim_channel != TIM_CHANNEL_2 &&
        init.wake_tim_channel != TIM_CHANNEL_3 && init.wake_tim_channel != TIM_CHANNEL_4) {
        return BQ_ERR;
    }
    /* Callers size frame arrays with BQ_MAX_RESPONSE_FRAMES, and bq_wake's stack wait scales with it */
    if (init.num_devices > BQ_MAX_DEVICES) {
        return BQ_ERR;
    }

    s_initialised = false;
    s_cfg = init;

    if (gpio_clk_enable(s_cfg.tx_rx_port) != BQ_OK || gpio_clk_enable(s_cfg.fault_port) != BQ_OK) {
        return BQ_ERR;
    }

    if (crc_init() != BQ_OK) {
        return BQ_ERR;
    }

    if (wake_tim_init() != BQ_OK) {
        return BQ_ERR;
    }

    /* TX starts as a GPIO driven high (idle). It's only handed to the UART
     * once the UART is enabled and idling high too, so the BQ's RX never sees
     * a low glitch that could read as a ping or COMM CLEAR */
    HAL_GPIO_WritePin(s_cfg.tx_rx_port, s_cfg.tx_pin, GPIO_PIN_SET);
    gpio.Pin   = s_cfg.tx_pin;
    gpio.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio.Pull  = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(s_cfg.tx_rx_port, &gpio);

    /* RX. Pull-up because the BQ's TX is high-impedance in SHUTDOWN - a
     * floating RX manufactures framing errors that look like real faults. */
    gpio.Pin       = s_cfg.rx_pin;
    gpio.Mode      = GPIO_MODE_AF_PP;
    gpio.Pull      = GPIO_PULLUP;
    gpio.Speed     = GPIO_SPEED_FREQ_HIGH;
    gpio.Alternate = s_cfg.uart_af;
    HAL_GPIO_Init(s_cfg.tx_rx_port, &gpio);

    /* NFAULT is an active-low output meant to be pulled up to CVDD externally */
    gpio.Pin       = s_cfg.fault_pin;
    gpio.Mode      = GPIO_MODE_INPUT;
    gpio.Pull      = GPIO_PULLUP;
    gpio.Speed     = GPIO_SPEED_FREQ_LOW;
    gpio.Alternate = 0U;
    HAL_GPIO_Init(s_cfg.fault_port, &gpio);

    s_cfg.huart->Init.BaudRate     = BQ_UART_BAUD;
    s_cfg.huart->Init.WordLength   = UART_WORDLENGTH_8B;
    s_cfg.huart->Init.StopBits     = UART_STOPBITS_1;
    s_cfg.huart->Init.Parity       = UART_PARITY_NONE;
    s_cfg.huart->Init.Mode         = UART_MODE_TX_RX;
    s_cfg.huart->Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    s_cfg.huart->Init.OverSampling = UART_OVERSAMPLING_16;

    /* With overrun detection on, an ORE aborts HAL's interrupt reception and
     * nothing in UART.c restarts it, so RX would go dead for good. A lost
     * byte already shows up as a CRC or timeout error on the frame. */
    s_cfg.huart->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_RXOVERRUNDISABLE_INIT;
    s_cfg.huart->AdvancedInit.OverrunDisable = UART_ADVFEATURE_OVERRUN_DISABLE;

    if (uart_init(s_cfg.huart) != UART_OK) {
        return BQ_ERR;
    }

    /* UART is enabled and idling high, hand TX over to it */
    gpio.Pin       = s_cfg.tx_pin;
    gpio.Mode      = GPIO_MODE_AF_PP;
    gpio.Pull      = GPIO_NOPULL;
    gpio.Speed     = GPIO_SPEED_FREQ_HIGH;
    gpio.Alternate = s_cfg.uart_af;
    HAL_GPIO_Init(s_cfg.tx_rx_port, &gpio);

    s_initialised = true;
    return BQ_OK;
}

// Build a command frame (INIT, [DEV ADR], REG ADR, DATA, CRC) from msg and send it.
// msg->data holds msg->len (1-8) DATA bytes.
static bq_status_t bq_send(const bq_msg_frame_t *msg)
{
    uint8_t frame[BQ_MAX_FRAME_LEN];
    uint16_t byte_num = 0U;

    // build init byte
    frame[byte_num] = (uint8_t)(0x80U | ((uint8_t)msg->req << 4) | (msg->len - 1U));
    byte_num++;

    // if this is a single r/w, add a device address
    if (msg->req == BQ_SINGLE_READ || msg->req == BQ_SINGLE_WRITE) {
        frame[byte_num] = msg->dev_addr;
        byte_num++;
    }

    // put in 2 byte reg address
    frame[byte_num] = (uint8_t)(msg->reg_addr >> 8);
    byte_num++;
    frame[byte_num] = (uint8_t)(msg->reg_addr & 0xFFU);
    byte_num++;

    // fill in the data
    for (uint8_t i = 0U; i < msg->len; i++) {
        frame[byte_num] = msg->data[i];
        byte_num++;
    }

    // add the crc
    uint16_t crc = bq_crc16(frame, byte_num);
    frame[byte_num] = (uint8_t)(crc & 0xFFU);    /* CRC LSB first */
    byte_num++;
    frame[byte_num] = (uint8_t)(crc >> 8);
    byte_num++;

    // run it
    if (uart_send(s_cfg.huart, frame, byte_num, pdMS_TO_TICKS(BQ_RESP_TIMEOUT_MS)) != UART_OK) {
        return BQ_ERR;
    }

    return BQ_OK;
}

bq_status_t bq_write(const bq_msg_frame_t *msg)
{
    // genius checks
    if (!s_initialised || msg == NULL || msg->data == NULL || msg->len == 0U || msg->len > BQ_MAX_WRITE_LEN) {
        return BQ_ERR;
    }
    if (msg->req != BQ_SINGLE_WRITE && msg->req != BQ_STACK_WRITE &&
        msg->req != BQ_BROAD_WRITE  && msg->req != BQ_BROAD_WRITE_REV) {
        return BQ_ERR;
    }
    if (msg->req == BQ_SINGLE_WRITE && msg->dev_addr > BQ_MAX_DEV_ADDR) {
        return BQ_ERR;
    }

    return bq_send(msg);
}

bq_status_t bq_read(const bq_msg_frame_t *msg, bq_response_frame_t *frames, uint8_t max_frames)
{
    static uint8_t resp[BQ_MAX_FRAME_LEN];
    uint8_t num_frames;

    // high IQ checks
    if (!s_initialised || msg == NULL || frames == NULL || msg->len == 0U || msg->len > BQ_MAX_DATA_LEN) {
        return BQ_ERR;
    }
    if (msg->req != BQ_SINGLE_READ && msg->req != BQ_STACK_READ && msg->req != BQ_BROAD_READ) {
        return BQ_ERR;
    }
    if (msg->req == BQ_SINGLE_READ && msg->dev_addr > BQ_MAX_DEV_ADDR) {
        return BQ_ERR;
    }

    const uint16_t resp_len = (uint16_t)msg->len + 6U;

    // one frame per responding device
    switch (msg->req) {
        case BQ_SINGLE_READ:
            num_frames = 1U;
            break;
        case BQ_STACK_READ:
            num_frames = s_cfg.num_devices;
            break;
        case BQ_BROAD_READ:
            num_frames = s_cfg.num_devices + 1U;
            break;
        default: return BQ_ERR;
    }
    // check before sending, so a too-small array never leaves unread replies on the line
    if (num_frames == 0U || num_frames > max_frames) {
        return BQ_ERR;
    }

    /* Read command DATA byte is the number of bytes to return, minus one */
    const uint8_t count = msg->len - 1U;
    const bq_msg_frame_t cmd = {
        .req      = msg->req,
        .dev_addr = msg->dev_addr,
        .reg_addr = msg->reg_addr,
        .data     = &count,
        .len      = 1U,
    };
    bq_rx_flush();
    bq_status_t status = bq_send(&cmd);
    
    if (status != BQ_OK) {
        return status;
    }

    // gather every response frame, form either 1 (1) or more than one (more)
    for (uint8_t dev = 0U; dev < num_frames; dev++) {

        // get this cycle's frame
        if (uart_recv(s_cfg.huart, resp, resp_len, pdMS_TO_TICKS(BQ_RESP_TIMEOUT_MS)) != UART_OK) {
            return BQ_ERR;
        }

        // check CRC
        if (bq_crc16(resp, resp_len) != 0U) {
            return BQ_ERR;
        }   

        // check response frame for correctness
        if (resp[0] != count) {
            return BQ_ERR;
        }
        if (msg->req == BQ_SINGLE_READ && resp[1] != msg->dev_addr) {
            return BQ_ERR;
        }
        if (resp[2] != (uint8_t)(msg->reg_addr >> 8) || resp[3] != (uint8_t)(msg->reg_addr & 0xFFU)) {
            return BQ_ERR;
        }

        // keep who answered and what they sent
        frames[dev].dev_addr = resp[1];
        memcpy(frames[dev].data, &resp[4], msg->len);
    }

    return BQ_OK;
}

// WAKE ping: hold the BQ's RX low for BQ_WAKE_PING_US, then release it high (Section 6.4.3.1)
bq_status_t bq_wake(void) {
    GPIO_InitTypeDef gpio = {0};

    if (!s_initialised) {
        return BQ_ERR;
    }

    /* Don't cut off a frame. HAL only returns gState to READY at transmit
     * complete, i.e. after the last stop bit has left the pin. */
    if (s_cfg.huart->gState != HAL_UART_STATE_READY) {
        return BQ_ERR;
    }

    // Hand tx_pin to the timer
    gpio.Pin       = s_cfg.tx_pin;
    gpio.Mode      = GPIO_MODE_AF_PP;
    gpio.Pull      = GPIO_NOPULL;
    gpio.Speed     = GPIO_SPEED_FREQ_HIGH;
    gpio.Alternate = s_cfg.wake_tim_af;
    HAL_GPIO_Init(s_cfg.tx_rx_port, &gpio);

    /* Fire. From here the timer times the ping on its own and clears CEN
     * when it is done. */
    s_htim.Instance->CR1 |= TIM_CR1_CEN;

    vTaskDelay(pdMS_TO_TICKS(((BQ_WAKE_PING_US + 999U) / 1000U) + BQ_WAKE_SETTLE_MS));

    /* Hand tx_pin back to the UART. On the error path this also ends the low,
     * since the UART idles high. */
    gpio.Alternate = s_cfg.uart_af;
    HAL_GPIO_Init(s_cfg.tx_rx_port, &gpio);


    const uint8_t send_wake = BQ_REG_CONTROL1_SEND_WAKE;
    const bq_msg_frame_t wake_stack = {
        .req      = BQ_SINGLE_WRITE,
        .dev_addr = 0U,                 // the BQ79600 bridge
        .reg_addr = BQ_REG_CONTROL1,
        .data     = &send_wake,
        .len      = 1U,
    };
    if (bq_write(&wake_stack) != BQ_OK) {
        return BQ_ERR;
    }

    // Each BQ79616 takes up to tSU(WAKE_SHUT) to wake and forward the tone to the next one
    vTaskDelay(pdMS_TO_TICKS((uint32_t)s_cfg.num_devices * BQ_WAKE_PER_DEV_MS) + 1U);

    return BQ_OK;
}

// One-byte write
static bq_status_t bq_write_byte(bq_req_t req, uint8_t dev_addr, uint16_t reg_addr, uint8_t value)
{
    const bq_msg_frame_t msg = {
        .req      = req,
        .dev_addr = dev_addr,
        .reg_addr = reg_addr,
        .data     = &value,
        .len      = 1U,
    };
    return bq_write(&msg);
}

// Auto-addressing
bq_status_t bq_auto_address(void)
{
    static bq_response_frame_t frames[BQ_MAX_RESPONSE_FRAMES];
    const uint8_t n = s_cfg.num_devices;
    bq_msg_frame_t msg;

    if (!s_initialised || n == 0U) {
        return BQ_ERR;
    }

    for (uint16_t reg = BQ_REG_OTP_ECC_DATAIN1; reg <= BQ_REG_OTP_ECC_DATAIN8; reg++) {
        if (bq_write_byte(BQ_STACK_WRITE, 0U, reg, 0x00U) != BQ_OK) {
            return BQ_ERR;
        }
    }

    // every device keeps the next DIR0_ADDR it sees and doesn't forward it 
    if (bq_write_byte(BQ_BROAD_WRITE, 0U, BQ_REG_CONTROL1, BQ_REG_CONTROL1_ADDR_WR) != BQ_OK) {
        return BQ_ERR;
    }

    // 0 lands on the bridge, 1..n on the BQ79616s in chain order 
    for (uint8_t addr = 0U; addr <= n; addr++) {
        if (bq_write_byte(BQ_BROAD_WRITE, 0U, BQ_REG_DIR0_ADDR, addr) != BQ_OK) {
            return BQ_ERR;
        } 
    }

    // every BQ79616 is a stack device. The bridge is the base by default
    if (bq_write_byte(BQ_BROAD_WRITE, 0U, BQ_REG_COMM_CTRL, BQ_REG_COMM_CTRL_STACK_DEV) != BQ_OK) {
        return BQ_ERR;
    }

    // The last one is also top of stack, so it starts the response chain */
    if (bq_write_byte(BQ_SINGLE_WRITE, n, BQ_REG_COMM_CTRL,
                      BQ_REG_COMM_CTRL_STACK_DEV | BQ_REG_COMM_CTRL_TOP_STACK) != BQ_OK) {
        return BQ_ERR;
    }

    /* sync the DLLs in the read direction. 8 separate reads. The replies can be
     * garbled while the DLLs lock, so their status is ignored; bq_read() flushes whatever
     * is left over before its next command. */
    msg = (bq_msg_frame_t){ .req = BQ_STACK_READ, .len = 1U };
    for (uint16_t reg = BQ_REG_OTP_ECC_DATAIN1; reg <= BQ_REG_OTP_ECC_DATAIN8; reg++) {
        msg.reg_addr = reg;
        (void)bq_read(&msg, frames, BQ_MAX_RESPONSE_FRAMES);
    }

    // each BQ79616 reports the address it took, top of stack first */
    msg.reg_addr = BQ_REG_DIR0_ADDR;
    if (bq_read(&msg, frames, BQ_MAX_RESPONSE_FRAMES) != BQ_OK) {
        return BQ_ERR;
    }
    for (uint8_t i = 0U; i < n; i++) {
        const uint8_t expected = n - i;
        if (frames[i].dev_addr != expected || (frames[i].data[0] & BQ_MAX_DEV_ADDR) != expected) {
            return BQ_ERR;
        }
    }

    //  bridge still answers as device 0, with DEV_CONF1 at its reset value */
    msg = (bq_msg_frame_t){
        .req      = BQ_SINGLE_READ,
        .dev_addr = 0U,
        .reg_addr = BQ_REG_DEV_CONF1,
        .len      = 1U,
    };
    if (bq_read(&msg, frames, BQ_MAX_RESPONSE_FRAMES) != BQ_OK || frames[0].data[0] != BQ_REG_DEV_CONF1_RESET) {
        return BQ_ERR;
    }

    return BQ_OK;
}

/* Stack-write one byte to every BQ79616, then stack-read it back and check every device took it */
static bq_status_t bq_stack_write_verify(uint16_t reg_addr, uint8_t value)
{
    static bq_response_frame_t frames[BQ_MAX_RESPONSE_FRAMES];
    const bq_msg_frame_t read = {
        .req      = BQ_STACK_READ,
        .reg_addr = reg_addr,
        .len      = 1U,
    };

    if (bq_write_byte(BQ_STACK_WRITE, 0U, reg_addr, value) != BQ_OK) {
        return BQ_ERR;
    }
    if (bq_read(&read, frames, BQ_MAX_RESPONSE_FRAMES) != BQ_OK) {
        return BQ_ERR;
    }
    for (uint8_t i = 0U; i < s_cfg.num_devices; i++) {
        if (frames[i].data[0] != value) {
            return BQ_ERR;
        }
    }

    return BQ_OK;
}

bq_status_t bq_config(void)
{
    if (!s_initialised || s_cfg.num_devices == 0U) {
        return BQ_ERR;
    }

    // sets # of cells in series
    if (bq_stack_write_verify(BQ_REG_ACTIVE_CELL, BQ_REG_ACTIVE_CELL_NUM(BQ_NUM_CELLS)) != BQ_OK) {
        return BQ_ERR;
    }
    // wait to let main AFE to settle
    vTaskDelay(pdMS_TO_TICKS(BQ_AFE_SETTLE_MS) + 1U);

    /* Long comm timeout: after 1 min without valid communication, go to SHUTDOWN.
     * CTS_TIME = 0 leaves the short timeout off.*/
    if (bq_stack_write_verify(BQ_REG_COMM_TIMEOUT_CONF,
                              BQ_REG_COMM_TIMEOUT_CONF_CTL_ACT_SHUTDOWN | BQ_REG_COMM_TIMEOUT_CONF_CTL_TIME_1MIN) != BQ_OK) {
        return BQ_ERR;
    }

    /* GPIO1-8 as ADC-only thermistor inputs: measured by the main ADC, but the OT/UT comparators
     * stay off, so a missing thermistor can't raise a fault. Each register holds two GPIOs; the
     * upper bits (GPIO_CONF1's FAULT_IN_EN and SPI_EN, spares elsewhere) stay 0.*/
    for (uint16_t reg = BQ_REG_GPIO_CONF1; reg <= BQ_REG_GPIO_CONF4; reg++) {
        if (bq_stack_write_verify(reg, BQ_REG_GPIO_CONF_PAIR(BQ_GPIO_MODE_ADC_ONLY, BQ_GPIO_MODE_ADC_ONLY)) != BQ_OK) {
            return BQ_ERR;
        }
    }

    /* TSREF is the 5 V bias at the top of each thermistor divider; it's off at reset. SEND_HW_RESET,
     * the only other bit in CONTROL2, stays 0 */
    if (bq_stack_write_verify(BQ_REG_CONTROL2, BQ_REG_CONTROL2_TSREF_EN) != BQ_OK) {
        return BQ_ERR;
    }
    vTaskDelay(pdMS_TO_TICKS(BQ_TSREF_SETTLE_MS) + 1U);

    return BQ_OK;
}
// feed in a pointer to an array of cell voltages 
bq_status_t bq_read_cell_voltages(uint16_t cell_voltages_mv[BQ_MAX_DEVICES][BQ_NUM_CELLS])
{
    static bq_response_frame_t frames[BQ_MAX_RESPONSE_FRAMES];
    const uint8_t n = s_cfg.num_devices;
    bq_msg_frame_t msg;

    // bq_init() capped num_devices at BQ_MAX_DEVICES, so every device has a row
    if (!s_initialised || cell_voltages_mv == NULL || n == 0U) {
        return BQ_ERR;
    }

    /* One-shot main ADC run on every BQ79616: 8 round robins, then it stops and holds the
     * results. Writing MAIN_GO clears DRDY_MAIN_ADC, so the results below are always from this
     * run */
    if (bq_write_byte(BQ_STACK_WRITE, 0U, BQ_REG_ADC_CTRL1,
                      BQ_REG_ADC_CTRL1_MAIN_MODE_SINGLE | BQ_REG_ADC_CTRL1_MAIN_GO) != BQ_OK) {
        return BQ_ERR;
    }

    /* Wait out the whole single run (8 x tMAIN_ADC_CYCLE). A result that still never converted
     * shows up as 0x8000 below. +1 tick: vTaskDelay(N) can return early. */
    vTaskDelay(pdMS_TO_TICKS(BQ_ADC_SINGLE_RUN_MS) + 1U);

    /* One stack read from VCELL<BQ_NUM_CELLS>_HI up to VCELL1_LO. The registers run from the
     * highest cell down, 2 bytes each, big-endian two's complement. */
    msg = (bq_msg_frame_t){
        .req      = BQ_STACK_READ,
        .reg_addr = BQ_REG_VCELL_HI(BQ_NUM_CELLS),
        .len      = 2U * BQ_NUM_CELLS,
    };
    if (bq_read(&msg, frames, BQ_MAX_RESPONSE_FRAMES) != BQ_OK) {
        return BQ_ERR;
    }

    for (uint8_t i = 0U; i < n; i++) {
        const uint8_t dev = frames[i].dev_addr;     // top of stack comes first
        if (dev == 0U || dev > n) {
            return BQ_ERR;
        }
        for (uint8_t cell = 1U; cell <= BQ_NUM_CELLS; cell++) {
            const uint8_t *hi_lo = &frames[i].data[2U * (BQ_NUM_CELLS - cell)];
            const int16_t raw = (int16_t)(((uint16_t)hi_lo[0] << 8) | hi_lo[1]);
            if (raw == BQ_ADC_RESULT_INVALID) {
                return BQ_ERR;
            }
            /* raw * 190.73 uV in mV, rounded: |raw| <= 32767, so raw * 19073 fits in int32 */
            const int32_t mv = ((int32_t)raw * BQ_VCELL_UV_PER_LSB_X100 + 50000) / 100000;
            cell_voltages_mv[dev - 1U][cell - 1U] = (mv < 0) ? 0U : (uint16_t)mv;
        }
    }

    return BQ_OK;
}
