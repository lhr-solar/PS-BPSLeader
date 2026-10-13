#include "CANbus.h"
#include "config.h"

static uint32_t HAL_RCC_FDCAN_CLK_ENABLED=0;

FDCAN_HandleTypeDef* car_can = NULL;
FDCAN_HandleTypeDef* bps_can = NULL;

static bool is_initialized = false;

static void FDCAN_Init_TXHeader(FDCAN_TxHeaderTypeDef* tx_header, uint32_t ID, uint32_t dataLength) {

    tx_header->Identifier = ID;
    tx_header->IdType = FDCAN_STANDARD_ID;
    tx_header->TxFrameType = FDCAN_DATA_FRAME;
    tx_header->DataLength = dataLength;
    tx_header->ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    tx_header->BitRateSwitch = FDCAN_BRS_OFF;
    tx_header->FDFormat = FDCAN_CLASSIC_CAN;
    tx_header->TxEventFifoControl = FDCAN_STORE_TX_EVENTS;
    tx_header->MessageMarker = 0;
    
}

can_status_t car_can_send(uint32_t ID, uint8_t data[], uint32_t data_length, TickType_t delay_ms) {

    if ((car_can == NULL) || (!is_initialized)) return CAN_ERR;

    FDCAN_TxHeaderTypeDef tx_header;
    FDCAN_Init_TXHeader(&tx_header, ID, data_length);

    return can_fd_send(car_can, &tx_header, data, pdMS_TO_TICKS(delay_ms));
}

can_status_t bps_can_send(uint32_t ID, uint8_t data[], uint32_t data_length, TickType_t delay_ms) {

    if ((bps_can == NULL) || (!is_initialized)) return CAN_ERR;

    FDCAN_TxHeaderTypeDef tx_header;
    FDCAN_Init_TXHeader(&tx_header, ID, data_length);

    return can_fd_send(bps_can, &tx_header, data, pdMS_TO_TICKS(delay_ms));
}

can_status_t car_can_recv(uint32_t ID, uint8_t data[], uint32_t data_length, TickType_t delay_ms) {

    if ((car_can == NULL) || (!is_initialized)) return CAN_ERR;

    FDCAN_RxHeaderTypeDef rx_header = { 0 };

    return can_fd_recv(car_can, ID, &rx_header, data, pdMS_TO_TICKS(delay_ms));
}

can_status_t bps_can_recv(uint32_t ID, uint8_t data[], uint32_t data_length, TickType_t delay_ms) {

    if ((bps_can == NULL) || (!is_initialized)) return CAN_ERR;

    FDCAN_RxHeaderTypeDef rx_header = { 0 };

    return can_fd_recv(bps_can, ID, &rx_header, data, pdMS_TO_TICKS(delay_ms));
}

void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef* fdcanHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  if(fdcanHandle->Instance==FDCAN1)
  {
 
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;
    PeriphClkInit.FdcanClockSelection = RCC_FDCANCLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* FDCAN1 clock enable */
    HAL_RCC_FDCAN_CLK_ENABLED++;
    if(HAL_RCC_FDCAN_CLK_ENABLED==1){
      __HAL_RCC_FDCAN_CLK_ENABLE();
    }

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**FDCAN1 GPIO Configuration
    PA11     ------> FDCAN1_RX
    PA12     ------> FDCAN1_TX
    */ 
    GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_FDCAN1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* FDCAN1 interrupt Init */
    HAL_NVIC_SetPriority(FDCAN1_IT0_IRQn, FDCAN_NVIC_PRIO, 0);
    HAL_NVIC_EnableIRQ(FDCAN1_IT0_IRQn);
    HAL_NVIC_SetPriority(FDCAN1_IT1_IRQn, FDCAN_NVIC_PRIO, 0);
    HAL_NVIC_EnableIRQ(FDCAN1_IT1_IRQn);
  }

  else if(fdcanHandle->Instance==FDCAN3)
  {
  /** Initializes the peripherals clocks
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;
    PeriphClkInit.FdcanClockSelection = RCC_FDCANCLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* FDCAN3 clock enable */
    HAL_RCC_FDCAN_CLK_ENABLED++;
    if(HAL_RCC_FDCAN_CLK_ENABLED==1){
      __HAL_RCC_FDCAN_CLK_ENABLE();
    }

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**FDCAN3 GPIO Configuration
    PA8     ------> FDCAN3_RX
    PA15     ------> FDCAN3_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF11_FDCAN3;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* FDCAN3 interrupt Init */
    HAL_NVIC_SetPriority(FDCAN3_IT0_IRQn, FDCAN_NVIC_PRIO, 0);
    HAL_NVIC_EnableIRQ(FDCAN3_IT0_IRQn);
    HAL_NVIC_SetPriority(FDCAN3_IT1_IRQn, FDCAN_NVIC_PRIO, 0);
    HAL_NVIC_EnableIRQ(FDCAN3_IT1_IRQn);

  }
}

static can_status_t BPS_CAN_Init(void)
{   

    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();

    bps_can = hfdcan1;
    bps_can->Instance = FDCAN1;
    bps_can->Init.ClockDivider = FDCAN_CLOCK_DIV1;
    bps_can->Init.FrameFormat = FDCAN_FRAME_CLASSIC;
    bps_can->Init.Mode = FDCAN_MODE_NORMAL;
    bps_can->Init.AutoRetransmission = ENABLE;
    bps_can->Init.TransmitPause = DISABLE;
    bps_can->Init.ProtocolException = DISABLE;
    bps_can->Init.NominalPrescaler = 20;
    bps_can->Init.NominalSyncJumpWidth = 1;
    bps_can->Init.NominalTimeSeg1 = 13;
    bps_can->Init.NominalTimeSeg2 = 2;
    bps_can->Init.DataPrescaler = 1;
    bps_can->Init.DataSyncJumpWidth = 1;
    bps_can->Init.DataTimeSeg1 = 1;
    bps_can->Init.DataTimeSeg2 = 1;
    bps_can->Init.StdFiltersNbr = 1;
    bps_can->Init.ExtFiltersNbr = 0;
    bps_can->Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;

    // FDCAN1 Filter Config
    FDCAN_FilterTypeDef sFilterConfig1;
    sFilterConfig1.IdType = FDCAN_STANDARD_ID;
    sFilterConfig1.FilterIndex = 0;
    sFilterConfig1.FilterType = FDCAN_FILTER_MASK;
    sFilterConfig1.FilterConfig = FDCAN_FILTER_TO_RXFIFO0; // directs frames to FIFO0
    sFilterConfig1.FilterID1 = 0x000;
    sFilterConfig1.FilterID2 = 0x000;

    if(can_fd_init(bps_can, &sFilterConfig1) != CAN_OK){
        return CAN_ERR;
    }

    if(can_fd_start(bps_can) != CAN_OK){
        return CAN_ERR;
    }

    return CAN_OK;

}

static can_status_t CAR_CAN_Init(void)
{

    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();

    car_can = hfdcan3;
    car_can->Instance = FDCAN3;
    car_can->Init.ClockDivider = FDCAN_CLOCK_DIV1;
    car_can->Init.FrameFormat = FDCAN_FRAME_CLASSIC;
    car_can->Init.Mode = FDCAN_MODE_NORMAL;
    car_can->Init.AutoRetransmission = ENABLE;
    car_can->Init.TransmitPause = DISABLE;
    car_can->Init.ProtocolException = DISABLE;
    car_can->Init.NominalPrescaler = 20;
    car_can->Init.NominalSyncJumpWidth = 1;
    car_can->Init.NominalTimeSeg1 = 13;
    car_can->Init.NominalTimeSeg2 = 2;
    car_can->Init.DataPrescaler = 1;
    car_can->Init.DataSyncJumpWidth = 1;
    car_can->Init.DataTimeSeg1 = 1;
    car_can->Init.DataTimeSeg2 = 1;
    car_can->Init.StdFiltersNbr = 1;
    car_can->Init.ExtFiltersNbr = 0;
    car_can->Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;


    // FDCAN3 Filter Config
    FDCAN_FilterTypeDef sFilterConfig1;
    sFilterConfig1.IdType = FDCAN_STANDARD_ID;
    sFilterConfig1.FilterIndex = 0;
    sFilterConfig1.FilterType = FDCAN_FILTER_MASK;
    sFilterConfig1.FilterConfig = FDCAN_FILTER_TO_RXFIFO0; // directs frames to FIFO0
    sFilterConfig1.FilterID1 = 0x000;
    sFilterConfig1.FilterID2 = 0x000;

    if(can_fd_init(car_can, &sFilterConfig1) != CAN_OK){
        return CAN_ERR;
    }

    if(can_fd_start(car_can) != CAN_OK){
        return CAN_ERR;
    }

    return CAN_OK;
    
}


can_status_t CAN_Init() {
    if (BPS_CAN_Init() != CAN_OK) return CAN_ERR;
    if (CAR_CAN_Init() != CAN_OK) return CAN_ERR;
    is_initialized = true;
    return CAN_OK;
}
