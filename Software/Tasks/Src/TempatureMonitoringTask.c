// TODO: Volt temps have multiple faults, this does not discriminate. Add discrimination.
// TODO: CAN QueueSet solution

#include "common.h"
#include "BPS_Tasks.h"
#include "CANbus.h"
#include "StatusLEDs.h"
#include "BPSCAN_can_msgs.h"
#include "CarCAN_can_msgs.h"
#include "string.h"

#define TEMPERATURE_CAN_DELAY_MS 10u

#define TEMP_TAPS_PER_BOARD (NUM_TEMPERATURE_SENSORS / NUM_VOLTTEMP_BOARDS)

// a mask of all 1's to compare against the temp sensor watchdog bitmap to ensure every bit is set (all info received)
#define TEMP_TAPS_ALL_DATA 0xFFFFFFFF

// timeout for receiving tap information from CAN
#define TEMP_WATCHDOG_TIMEOUT_MS 1000

// determines if we should use charging or discharging temp threshold depending if we're charging or discharging (state bit set in amperes task)
#define get_temp_threshold() ((get_state_bit(CHARGING_BATT_STATE) == STATE_BIT_SET) ? OVERTEMP_THRESHOLD_CHARGING_MC : OVERTEMP_THRESHOLD_DISCHARGING_MC)

// get first five bits of temp can message, which is id
#define TEMP_ID_MASK 0x1F

// get bits 5:7 of temp can message, which is fault code
#define TEMP_FAULT_MASK 0x7

// defines max value a message frame ID can be before reseting to 0
#define FRAME_ID_MAX 255

// Printf period macros
#define TEMP_LOOP_PRINTF_DELAY_MS 2000
#define TEMP_PRINTF_COUNTER (TEMP_LOOP_PRINTF_DELAY_MS / TEMP_MONITOR_TASK_DELAY_MS)

#define OTEMP_FAULT_THRESHOLD 3 // number of consecutive otemp faults before latching module fault
_Static_assert(OTEMP_FAULT_THRESHOLD < 255, "OTEMP_FAULT_THRESHOLD must be less than 255 since the histogram is an array of uint8_t");

uint32_t exposed_temperature_watchdog_bitmap = TEMP_TAPS_ALL_DATA;

// array to hold struct packed can data
bps_temperature_aggregate_arr_t temp_can_data[NUM_TEMPERATURE_SENSORS] = {0};
bps_temp_rawv_aggregate_arr_t temp_can_data2[NUM_TEMPERATURE_SENSORS] = {0};

// watchdog bitmap
uint32_t temp_watchdog_bitmap = 0;

// bitmap to hold temp sensor watchdog, starts all bits set (good), corresponding bits are cleared if taps don't check in
uint32_t exposed_temp_watchdog_bitmap = TEMP_TAPS_ALL_DATA;


// array to store how often a module has consecutively otemp'd, indexed by module number
uint8_t temp_module_fault_histogram[NUM_TEMPERATURE_SENSORS] = {0};

// watchdog timer
static TimerHandle_t temperature_watchdog_timer;
static StaticTimer_t temp_timer_buffer;

// array that is indexed to get the CAN id for each volltemp
static const uint32_t temperature_can_ids[NUM_VOLTTEMP_BOARDS] = {
    CAN_ID_BPS_VT0_TEMPERATURE_ARR,
    CAN_ID_BPS_VT1_TEMPERATURE_ARR,
    CAN_ID_BPS_VT2_TEMPERATURE_ARR,
    CAN_ID_BPS_VT3_TEMPERATURE_ARR,
    CAN_ID_BPS_VT4_TEMPERATURE_ARR,
    CAN_ID_BPS_VT5_TEMPERATURE_ARR,
    CAN_ID_BPS_VT6_TEMPERATURE_ARR,
    CAN_ID_BPS_VT7_TEMPERATURE_ARR};

uint32_t get_module_temperature(uint8_t module_num) {

    // invalid module number, return 69420 to indicate error
    if (module_num >= NUM_TEMPERATURE_SENSORS) {
        return 69420;
    }
    return temp_can_data[module_num].BPS_Temperature_Tap_Data;
}  
// pass in pointer to raw data, packs struct into arr
static uint8_t temp_can_unpack(uint8_t *raw_temp_can_data, bps_temperature_aggregate_arr_t *temp_can_data, bps_temp_rawv_aggregate_arr_t *temp_can_data2)
{

    static uint8_t frame_id = 0;

    static TickType_t s_last_rx_times[NUM_TEMPERATURE_SENSORS] = {0};

    // if sensor not sending, skip. Watchdog will err eventually if it doesn't receive more data
    if (raw_temp_can_data == NULL)
        return 0;

    // bits [0:4]: Tap index (Byte 0)
    uint8_t tap_index = raw_temp_can_data[0] & TEMP_ID_MASK;

    if (tap_index >= NUM_TEMPERATURE_SENSORS)
        return 0;

    temp_can_data[tap_index].BPS_Tap_idx = tap_index;

    // bits [8:31]: Temp data (24 bits, Bytes 1-3)
    temp_can_data[tap_index].BPS_Temperature_Tap_Data = raw_temp_can_data[1] << 0;
    temp_can_data[tap_index].BPS_Temperature_Tap_Data |= raw_temp_can_data[2] << 8;
    temp_can_data[tap_index].BPS_Temperature_Tap_Data |= raw_temp_can_data[3] << 16;

    // bits [32:39]: Fault data (8 bits, Byte 4)
    temp_can_data[tap_index].BPS_Temperature_Tap_Fault = raw_temp_can_data[4];

    // bits [40:55]: Raw Voltage data (16 bits, Bytes 5-6)
    temp_can_data2[tap_index].BPS_Tap_idx = tap_index;
    temp_can_data2[tap_index].BPS_Temperature_Tap_RawV = raw_temp_can_data[5] << 0;
    temp_can_data2[tap_index].BPS_Temperature_Tap_RawV |= raw_temp_can_data[6] << 8;

    temp_can_data2[tap_index].FrameID_BPS_Temperature = frame_id;
    temp_can_data[tap_index].FrameID_BPS_Temperature = frame_id;

    TickType_t current_time = xTaskGetTickCount();

    // set the time since last receive
    if (s_last_rx_times[tap_index] == 0)
    {
        temp_can_data[tap_index].BPS_Temperature_Tap_Age = 0;
    }
    else
    {
        // Calculate the delta: Current Time - Last Arrival Time
        temp_can_data[tap_index].BPS_Temperature_Tap_Age = Calculate_TimeDifference(current_time, s_last_rx_times[tap_index]);
    }

    s_last_rx_times[tap_index] = current_time;

    portENTER_CRITICAL();
    // set corresponding bit in recv bitmap
    temp_watchdog_bitmap |= (1U << tap_index);
    portEXIT_CRITICAL();

    // update frame id to next value, wrap back to 0 if it hits FRAME_ID_MAX + 1
    frame_id = (frame_id + 1) % (FRAME_ID_MAX + 1);

    return 1;
}

// pack temp aggregate array message
static void temp_can_pack(bps_temperature_aggregate_arr_t temp_can_data, uint8_t *msgArr)
{

    if (msgArr == NULL)
    {
        return;
    }

    // bits [0:4]: Tap index (length 5, byte 0)
    msgArr[0] = (temp_can_data.BPS_Tap_idx & TEMP_ID_MASK); // Assuming TEMP_ID_MASK is 0x1F

    // bits [8:31]: Temp data (length 24, bytes 1-3)
    memcpy(&msgArr[1], &(temp_can_data.BPS_Temperature_Tap_Data), 3UL);

    // bits [32:39]: Fault data (length 8, byte 4)
    msgArr[4] = temp_can_data.BPS_Temperature_Tap_Fault;

    // bits [40:55]: Tap age (length 16, bytes 5-6)
    memcpy(&msgArr[5], &(temp_can_data.BPS_Temperature_Tap_Age), sizeof(uint16_t));

    // bits [56:63]: Frame ID (length 8, byte 7)
    msgArr[7] = temp_can_data.FrameID_BPS_Temperature;
}

// Pack rawV debug temp message
static void temp_can_pack2(bps_temp_rawv_aggregate_arr_t temp_can_data, uint8_t *msgArr)
{
    if (msgArr == NULL)
    {
        return;
    }

    // bits [0:4]: Tap index (length 5, byte 0)
    // 0x1F is the hex mask for 5 bits (0b00011111)
    msgArr[0] = (temp_can_data.BPS_Tap_idx & 0x1F);

    // bits [8:23]: Raw Voltage (length 16, bytes 1-2)
    memcpy(&msgArr[1], &(temp_can_data.BPS_Temperature_Tap_RawV), sizeof(uint16_t));

    // bits [24:31]: Frame ID (length 8, byte 3)
    msgArr[3] = temp_can_data.FrameID_BPS_Temperature;
}

// gets all can data from each tap from a passed in volttemp board, unpacks it and puts it into array
static void can_recv_all_taps(uint32_t can_id_index, bps_temperature_aggregate_arr_t temp_can_data[], bps_temp_rawv_aggregate_arr_t temp_can_data2[])
{
    // can recieve for all 4 temperature taps for each temptemp board
    for (uint8_t i = 0; i < TEMP_TAPS_PER_BOARD; i++)
    {
        uint8_t raw_databuffer[CAN_DLC_BPS_VT0_TEMPERATURE_ARR] = {0};
        // if can fails then message will not be unpacked and the watchdog will trip
        if (bps_can_recv(temperature_can_ids[can_id_index], raw_databuffer, CAN_DLC_BPS_VT0_TEMPERATURE_ARR, TEMPERATURE_CAN_DELAY_MS) == CAN_OK)
        {
            // Unpacking temp CAN messages in aggregate temp array
            temp_can_unpack(raw_databuffer, temp_can_data, temp_can_data2);
        }
    }
}

// watchdog function that runs when the timer times out
static void vTemperatureWatchdogCallback(TimerHandle_t temp_timer)
{
    taskENTER_CRITICAL();
    // check if every tap has sent temp information since last timer timeout.
    if (temp_watchdog_bitmap != TEMP_TAPS_ALL_DATA)
    {
        // if one hasn't setn, save bitmap to know which one(s) didn't check in, then set fault bit
        exposed_temperature_watchdog_bitmap = temp_watchdog_bitmap;
        latch_mod_fault(get_mod_fault_num(exposed_temperature_watchdog_bitmap), 0); // Store 0 as the faulted module value since temperature isn't being stored here
        set_faultBit(VOLTTEMP_WATCHDOG_FAULT);
    }
    // reset bitmap
    temp_watchdog_bitmap = 0;
    taskEXIT_CRITICAL();
}

// returns the average temperature of all taps (used in can status)
uint32_t get_avg_temp()
{

    int32_t temp_sum = 0;

    for (uint8_t module_num = 0; module_num < NUM_BATTERY_MODULES; module_num++)
    {
        temp_sum += temp_can_data[module_num].BPS_Temperature_Tap_Data;
    }

    return (temp_sum / NUM_BATTERY_MODULES);
}

// checks status of a volttem segment
bool get_temp_segment_status(uint8_t segment_num)
{

    // confirm temperature readings are coming in
    if (((exposed_temp_watchdog_bitmap >> (segment_num * MODULES_PER_SEGMENT)) & 0xF) != 0xF)
    {
        return false;
    }

    // make sure no faults
    for (uint8_t tap_num = segment_num * MODULES_PER_SEGMENT; tap_num < (segment_num + 1) * MODULES_PER_SEGMENT; tap_num++)
    {

        if (temp_can_data[tap_num].BPS_Temperature_Tap_Fault != BPS_TEMPERATURE_AGGREGATE_ARR_BPS_TEMPERATURE_TAP_FAULT_OK || temp_can_data[tap_num].BPS_Temperature_Tap_Data > get_temp_threshold())
        {
            return false;
        }
    }
    return true;
}

void Task_Temperature_Monitor()
{

    uint32_t temp_printf_debug_counter = 0;

    // Make timer for watchdog
    temperature_watchdog_timer = xTimerCreateStatic(
        "Temp Watchdog",                         /* Name of the timer */
        pdMS_TO_TICKS(TEMP_WATCHDOG_TIMEOUT_MS), /* Timer period in ticks */
        pdTRUE,                                  /* auto-reload */
        (void *)0,                               /* Timer ID */
        vTemperatureWatchdogCallback,            /* Callback function */
        &temp_timer_buffer                       /* Buffer to hold timer data */
    );

    // start watchdog timer
    xTimerStart(temperature_watchdog_timer, 0);

    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1)
    {
        temp_printf_debug_counter++;

        // Delays
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(TEMP_MONITOR_TASK_DELAY_MS));

        // loops through each can ID
        for (uint8_t can_id_index = 0; can_id_index < NUM_VOLTTEMP_BOARDS; can_id_index++)
        {

            // recieve data from all taps from each id
            can_recv_all_taps(can_id_index, temp_can_data, temp_can_data2);
        }

        // msg buff is packed with the temp aggregate message, msgBuff 2 is the packed with rawV debug message. Parity between is insured by frame id
        uint8_t msgBuff[CAN_DLC_BPS_TEMPERATURE_AGGREGATE_ARR] = {0};
        uint8_t msgBuff2[CAN_DLC_BPS_TEMP_RAWV_AGGREGATE_ARR] = {0};

        // variable used to keep track of maximum temperature for this specific cycle
        uint32_t max_temp = 0;

        // keeps track if all temps pass checks in order to close contactors
        bool all_temp_good = true;

        // preforms all checks on temperature data, sets relevant flags
        for (uint8_t i = 0; i < NUM_TEMPERATURE_SENSORS; i++)
        {

            // finds the max temperature reading to use for bounds checking
            if (temp_can_data[i].BPS_Temperature_Tap_Data > max_temp)
            {
                max_temp = temp_can_data[i].BPS_Temperature_Tap_Data;
            }

            // check if max temp is greater than threshold (changes depending on if battery is charging or discharging)
            if (temp_can_data[i].BPS_Temperature_Tap_Data > get_temp_threshold())
            {
                if (get_state_bit(CHARGING_BATT_STATE) == STATE_BIT_SET)
                {
                    temp_can_data[i].BPS_Temperature_Tap_Fault = BPS_TEMPERATURE_AGGREGATE_ARR_BPS_TEMPERATURE_TAP_FAULT_CHARGE_OVER_TEMPERATURE;
                }
                else
                {
                    temp_can_data[i].BPS_Temperature_Tap_Fault = BPS_TEMPERATURE_AGGREGATE_ARR_BPS_TEMPERATURE_TAP_FAULT_OVER_TEMPERATURE;
                }

                // if this module is otemp'd increment that module's fault count
                temp_module_fault_histogram[temp_can_data[i].BPS_Tap_idx]++;


                // only fault the BPS if a singular module has consecutively otemp'd a certain number of times to filter single abnormal readings from actual faults
                if(temp_module_fault_histogram[temp_can_data[i].BPS_Tap_idx] >= OTEMP_FAULT_THRESHOLD)
                {
                    printf("Entering Cell Over Temperature Fault for Tap %d: %ldmC\r\n", temp_can_data[i].BPS_Tap_idx, temp_can_data[i].BPS_Temperature_Tap_Data);
                    // latch this module as one who faulted, set fault bit, and set flag indicating we are not good to close contactors
                    latch_mod_fault(temp_can_data[i].BPS_Tap_idx, temp_can_data[i].BPS_Temperature_Tap_Data);
                    set_faultBit(CELL_OVERTEMP_FAULT);
                    all_temp_good = false;
                }
            }
            else
            {
                // if temp is good, reset fault histogram for this module
                temp_module_fault_histogram[temp_can_data[i].BPS_Tap_idx] = 0;
            }

            // pack temp aggregate array and rawV debug message
            temp_can_pack(temp_can_data[i], msgBuff);
            temp_can_pack2(temp_can_data2[i], msgBuff2);

            // forward these two messages to carcan
            car_can_send(CAN_ID_BPS_TEMPERATURE_AGGREGATE_ARR, msgBuff, CAN_DLC_BPS_TEMPERATURE_AGGREGATE_ARR, pdMS_TO_TICKS(TEMPERATURE_CAN_DELAY_MS));
            car_can_send(CAN_ID_BPS_TEMP_RAWV_AGGREGATE_ARR, msgBuff2, CAN_DLC_BPS_TEMP_RAWV_AGGREGATE_ARR, pdMS_TO_TICKS(TEMPERATURE_CAN_DELAY_MS));
        }

        if (temp_printf_debug_counter >= TEMP_PRINTF_COUNTER)
        {
            printf("================================\r\n");
            printf("Temperature values: {");
            for (int j = 0; j < NUM_BATTERY_MODULES; j++)
            {
                printf("%d: %lu.%03lu C, ", j, temp_can_data[j].BPS_Temperature_Tap_Data / 1000, temp_can_data[j].BPS_Temperature_Tap_Data % 1000);
            }
            printf("}\r\n");
            printf("================================\r\n");

            temp_printf_debug_counter = 0;
        }

        // if max temp is below charging threshold, and the state bit isn't already set, set state bit
        if ((max_temp < CELL_CHARGING_TEMP_THRESHOLD_MC) && (get_state_bit(TEMP_OK_FOR_CHARGING) != STATE_BIT_SET))
        {
            set_state_bit(TEMP_OK_FOR_CHARGING, STATE_BIT_SET);
        }
        // else if max temp is at or above charging threshold, and the state bit isn't already cleared, clear state bit
        else if ((max_temp >= CELL_CHARGING_TEMP_THRESHOLD_MC) && (get_state_bit(TEMP_OK_FOR_CHARGING) != STATE_BIT_RESET))
        {
            set_state_bit(TEMP_OK_FOR_CHARGING, STATE_BIT_RESET);
        }

        // if all temperature values are within range, and the state bit isn't already set, set state bit indicating we're good to close contactors
        if (all_temp_good && (get_state_bit(TEMPERATURE_MONITOR_GOOD) != STATE_BIT_SET))
        {
            set_state_bit(TEMPERATURE_MONITOR_GOOD, STATE_BIT_SET);
        }

        // Set event group bit for independent watchdog
        xEventGroupSetBits(xWDogEventGroup_handle, /* The event group being updated. */
                           TEMP_MONITOR_DONE);     /* The bits being set. */
    }
}
