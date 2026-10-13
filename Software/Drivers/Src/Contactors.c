
#include "Contactors.h"

static SemaphoreHandle_t contactorsMutex = NULL;
static StaticSemaphore_t contactorsMutexBuffer;

bool contactor_is_initialized = false;

const char* CONTACTOR_NAMES[NUM_CONTACTORS] = {
    "HV Plus Contactor",
    "HV Minus Contactor",
    "Array Contactor",
    "Array Precharge Contactor"
};

// array to hold the contactor structs
static contactor_t contactors[NUM_CONTACTORS];

// get physical state
contactor_state_t contactor_get(contactor_num_t contactor_num)
{

    // check that contactor exists
    if ((contactor_num < 0) || (contactor_num >= NUM_CONTACTORS))
    {
        return CONTACTOR_ERROR;
    }

    contactor_t *contactor = &contactors[contactor_num];
    return (HAL_GPIO_ReadPin(contactor->sense_pin.port, contactor->sense_pin.pin) ? CONTACTOR_CLOSED : CONTACTOR_OPEN);
}

// determines if the contactor physical state matches the expected state
contactor_state_t contactor_verify (contactor_num_t contactor_num)
{

    // check that contactor exists
    if ((contactor_num < 0) || (contactor_num >= NUM_CONTACTORS))
    {
        return CONTACTOR_ERROR;
    }

    contactor_t *contactor = &contactors[contactor_num];

    // if timer is active contactor could be in either state
    if (xTimerIsTimerActive(contactor->senseTimer) == pdTRUE) {
        return CONTACTOR_OK;
    }

    return (contactor_get(contactor_num) == contactor->state) ? CONTACTOR_OK : CONTACTOR_ERROR;
}

static void vContactorCallback(TimerHandle_t senseTimer)
{

    contactor_num_t contactor_num = (contactor_num_t)pvTimerGetTimerID(senseTimer);
    contactor_t *contactor = &contactors[contactor_num];

    if (contactor->state != contactor_get(contactor_num))
    {
        contactor->callback_faulted = true;
        set_faultBit(CONTACTOR_CALLBACK_FAULT);
    }
}

estop_status_t contactor_estop_checker()
{

    if (HAL_GPIO_ReadPin(ESTOP1_PORT, ESTOP1_PIN) == GPIO_PIN_SET)
    {
        return ESTOP1_FAULT;
    }
    else if (HAL_GPIO_ReadPin(ESTOP2_PORT, ESTOP2_PIN) == GPIO_PIN_SET)
    {
        return ESTOP2_FAULT;
    }
    else if (HAL_GPIO_ReadPin(ESTOP3_PORT, ESTOP3_PIN) == GPIO_PIN_SET)
    {
        return ESTOP3_FAULT;
    }
    else {
        return ESTOP_OK;
    }
}

bool contactor_get_faulted_status(contactor_num_t contactor_num)
{
    if (!contactor_is_initialized)
        return false;
    // check that contactor exists
    if ((contactor_num < 0) || (contactor_num >= NUM_CONTACTORS))
    {
        return false;
    }

    contactor_t *contactor = &contactors[contactor_num];
    return contactor->callback_faulted;
}

// sets contactor, updates state value, then starts timer to check expected state matches actual state.
contactor_state_t contactor_set(contactor_num_t contactor_num, contactor_state_t state, TickType_t wait_ms, fault_state_t fault_state)
{

    // check that contactor exists
    if ((contactor_num < 0) || (contactor_num >= NUM_CONTACTORS))
    {
        return CONTACTOR_ERROR;
    }

    if ((contactorsMutex == NULL) && (fault_state != EMERGENCY))
        return CONTACTOR_ERROR;

    contactor_t *contactor = &contactors[contactor_num];

    // if its emergency, dont bother with semaphore
    if ((fault_state != EMERGENCY) && xSemaphoreTake(contactorsMutex, pdMS_TO_TICKS(wait_ms)) == pdFALSE)
    {
        return CONTACTOR_ERROR;
    };

    // critical section:
    HAL_GPIO_WritePin(contactor->control_pin.port, contactor->control_pin.pin, ((state == CONTACTOR_CLOSED) ? GPIO_PIN_SET : GPIO_PIN_RESET));
    contactor->state = state;

    /* start timer to check if the state of the contactor makes expected state, the exit critical section. Timer resets
    when the contactor is set to another value, so no possible error with expected value changing from when timer is called
    the 0 param in xTimerStart indicates to start the timer immediately, not wait any ticks*/
    if ((fault_state != EMERGENCY))
    {
        xTimerStart(contactor->senseTimer, 0);
        xSemaphoreGive(contactorsMutex);
    }

    return CONTACTOR_OK;
}

void contactor_init()
{

    if (contactor_is_initialized)
        return;

    // Enable clock for GPIO A/B/C
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    // initializing the pindef into contactor structs
    contactors[HV_PLUS_CONTACTOR] = (contactor_t){
        .control_pin = {HV_PLUS_CONTROL_PORT, HV_PLUS_CONTROL_PIN},
        .sense_pin = {HV_PLUS_SENSE_PORT, HV_PLUS_SENSE_PIN}};

    contactors[HV_MINUS_CONTACTOR] = (contactor_t){
        .control_pin = {HV_MINUS_CONTROL_PORT, HV_MINUS_CONTROL_PIN},
        .sense_pin = {HV_MINUS_SENSE_PORT, HV_MINUS_SENSE_PIN}};

    contactors[ARRAY_CONTACTOR] = (contactor_t){
        .control_pin = {ARRAY_CONTROL_PORT, ARRAY_CONTROL_PIN},
        .sense_pin = {ARRAY_SENSE_PORT, ARRAY_SENSE_PIN}};

    contactors[ARRAY_PRE_CONTACTOR] = (contactor_t){
        .control_pin = {ARRAY_PRE_CONTROL_PORT, ARRAY_PRE_CONTROL_PIN},
        .sense_pin = {ARRAY_PRE_SENSE_PORT, ARRAY_PRE_SENSE_PIN}};

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // init estops
    GPIO_InitStruct.Pin = ESTOP1_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(ESTOP1_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = ESTOP2_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(ESTOP2_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = ESTOP3_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(ESTOP3_PORT, &GPIO_InitStruct);

    // initialize ESTOPS

    // loop to intialize contactor GPIO and timers
    for (uint8_t contactor_num = 0; contactor_num < NUM_CONTACTORS; contactor_num++)
    {

        contactor_t *contactor = &contactors[contactor_num];

        // set output level
        HAL_GPIO_WritePin(contactor->control_pin.port, contactor->control_pin.pin, GPIO_PIN_RESET);

        // init contactor control pins
        GPIO_InitStruct.Pin = contactor->control_pin.pin;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(contactor->control_pin.port, &GPIO_InitStruct);

        // init contactor sense pins
        GPIO_InitStruct.Pin = contactor->sense_pin.pin;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(contactor->sense_pin.port, &GPIO_InitStruct);

        contactor->state = contactor_get(contactor_num);

        contactor->callback_faulted = false;

        // making timers and putting them into contactor structs
        contactor->senseTimer = xTimerCreateStatic(
            CONTACTOR_NAMES[contactor_num],  /* Name of the timer */
            CONTACTOR_SENSE_DELAY_TICKS,     /* Timer period in ticks */
            pdFALSE,                         /* Don't auto-reload */
            (void *)(uint32_t)contactor_num, /* Timer ID */
            vContactorCallback,              /* Callback function */
            &(contactor->senseTimerBuffer)   /* Buffer to hold timer data */
        );
    }

    // INITIALIZE MUTEX
    contactorsMutex = xSemaphoreCreateMutexStatic(&contactorsMutexBuffer);

    contactor_is_initialized = true;
}

// Opens all contactors in case of fault
void emergency_open_contactors(void)
{

    contactor_init();

    // pdTICKS_TO_MS
    for (uint8_t contactor_num = 0; contactor_num < NUM_CONTACTORS; contactor_num++)
    {
        contactor_set(contactor_num, CONTACTOR_OPEN, pdTICKS_TO_MS(portMAX_DELAY), EMERGENCY);
    }
}
