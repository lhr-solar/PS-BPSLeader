#include "common.h"
#include "CANbus.h"
#include "BPS_Tasks.h"

#define CAN_RX_FORWARD_QUEUE_SIZE 50
#define CAN_FORWARD_WAIT_TICKS pdMS_TO_TICKS(10)

#define FAULT_COUNT_ERROR_THRESHOLD 5

static StaticQueue_t canRxForwardQueueBuffer;
static uint8_t canRxForwardQueueStorage[CAN_RX_FORWARD_QUEUE_SIZE * sizeof(can_rx_payload_t)];
static QueueHandle_t canRxForwardQueue;

 
void can_fd_rx_callback_hook(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs, can_rx_payload_t recv_payload)
{

    static volatile uint8_t amperes_msg_counter = 0; 

    BaseType_t higherPriorityTaskWoken = pdFALSE;

    if (hfdcan->Instance == BPS_CAN_CHANNEL)
    {
        if (recv_payload.header.Identifier == CAN_ID_BPS_PACK_CURRENT) {
            
            if (canRxForwardQueue != NULL && amperes_msg_counter == 0)
            {
                xQueueSendCircularBufferFromISR(  
                    canRxForwardQueue,
                    &recv_payload,
                    &higherPriorityTaskWoken,
                    sizeof(can_rx_payload_t));
            }
            amperes_msg_counter = (amperes_msg_counter + 1) % 3;
        }
          
    }

    portYIELD_FROM_ISR(higherPriorityTaskWoken);
}

// NOTE: any CAN forwards that require more processing than simply forwarding (i.e. packing the voltage aggregate msg) will be done in their respective tasks
void Task_CanRxForward()
{

    // canbus MUST be initialized by now
    can_rx_payload_t payload;

    uint32_t ID, data_length;
    uint8_t *data;

    canRxForwardQueue = xQueueCreateStatic(
        CAN_RX_FORWARD_QUEUE_SIZE,
        sizeof(can_rx_payload_t),
        canRxForwardQueueStorage,
        &canRxForwardQueueBuffer);

    if (canRxForwardQueue == NULL)
    {
        set_faultBit(BPS_CAN_ERROR);
        vTaskDelete(NULL);
    }

    while (1)
    {
        if (xQueueReceive(canRxForwardQueue, &payload, portMAX_DELAY) == pdTRUE)
        {
            ID = payload.header.Identifier;            
            data_length = payload.header.DataLength;
            data = payload.data;

            car_can_send(ID, data, data_length, CAN_FORWARD_WAIT_TICKS);
            taskYIELD();
        } 
    }
}
