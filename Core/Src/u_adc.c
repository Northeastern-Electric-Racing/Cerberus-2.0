#include <stdint.h>
#include "u_tx_debug.h"
#include "main.h"
#include "u_mutexes.h"
#include "u_adc.h"

/* ADC1 Config. */
/* For mux'd inputs: SELx=HIGH corresponds to the A input. SELx=LOW corresponds to the B input. */
typedef enum {
    /* ADC Input Channel Ranks. *
    /* The order these values MUST match how the ADC ranks are set up in CubeMX. */
    ADC1_CHANNEL0,  // SEL1
    ADC1_CHANNEL2,  // APPS_1_ADC
    ADC1_CHANNEL3,  // EF_DASH_ADC
    ADC1_CHANNEL5,  // SEL3
    ADC1_CHANNEL6,  // APPS_2_ADC
    ADC1_CHANNEL9,  // SEL4
    ADC1_CHANNEL10, // EF_FANBATT_ADC
    ADC1_CHANNEL12, // EF_PUMP1_ADC
    ADC1_CHANNEL13, // EF_PUMP2_ADC
    ADC1_CHANNEL15, // SEL2
    ADC1_CHANNEL18, // EF_MC_ADC

    /* Total number of channels/indexes for ADC1. */
    ADC1_SIZE,
} _adc1_t;
static uint16_t _adc1_buffer[ADC1_SIZE]; // Buffer for the ADC DMA readings (note: has to be uint16_t to correspond with the "Half Word" GPDMA setting in CubeMX).

/* ADC2 Config. */
typedef enum {
    /* The order of items in this enum MUST match how the ADC ranks are set up in CubeMX. */

    ADC2_CHANNEL2, // BSE_1_ADC
    ADC2_CHANNEL6, // BSE_2_ADC

    /* Total number of indexes for ADC2. */
    ADC2_SIZE
} _adc2_t;
static uint16_t _adc2_buffer[ADC2_SIZE]; // Buffer for the ADC DMA readings (note: has to be uint16_t to correspond with the "Half Word" GPDMA setting in CubeMX).

/* Multiplexer buffer. */
typedef enum {
    /* SEL1 */
    SEL1_HIGH, // BREAKLIGHT_ADC
    SEL1_LOW,  // BATTBOX_ADC

    /* SEL2 */
    SEL2_HIGH,  // LFIU_CURRENT_1
    SEL2_LOW,   // LIFU_CURRENT_2
    
    /* SEL3 */
    SEL3_HIGH,  // SPARE_FUSE_ADC
    SEL3_LOW,   // SHUTDOWN_ADC

    /* SEL4 */
    SEL4_HIGH,  // RADFAN_ADC
    SEL4_LOW,   // SPARE1_ADC

    /* Total number of indexes for the multiplexer buffer. */
    MUX_SIZE
} _mux_t;
static uint16_t _mux_buffer[MUX_SIZE] = { 0 };

/* Manage muxs and updates the mux buffer. */
typedef enum { HIGH, LOW } _mux_state_t;
int adc_switchMuxState(void) {
    static _mux_state_t mux_state = LOW;

    if(mux_state == LOW) {
        /* Mux is currently LOW, so switch to HIGH. */
        HAL_Gpio_WritePin(MUX_SEL1_GPIO_Port, MUX_SEL1_Pin, GPIO_PIN_SET);
        HAL_Gpio_WritePin(MUX_SEL2_GPIO_Port, MUX_SEL2_Pin, GPIO_PIN_SET);
        HAL_Gpio_WritePin(MUX_SEL3_GPIO_Port, MUX_SEL3_Pin, GPIO_PIN_SET);
        HAL_Gpio_WritePin(MUX_SEL4_GPIO_Port, MUX_SEL4_Pin, GPIO_PIN_SET);

        tx_thread_sleep(1); // Sleep for 1 tick so the mux settles.
        mux_state = HIGH;   // Update the mux state

        /* We are now in the HIGH state, so set the associated indexes in the buffer. */
        mutex_get(&mux_buffer_mutex);
        _mux_buffer[SEL1_HIGH] = _adc1_buffer[ADC1_INP0_SEL1];
        _mux_buffer[SEL2_HIGH] = _adc1_buffer[ADC1_INP15_SEL2];
        _mux_buffer[SEL3_HIGH] = _adc1_buffer[ADC1_INP5_SEL3];
        _mux_buffer[SEL4_HIGH] = _adc1_buffer[ADC1_INP9_SEL4];
        mutex_put(&mux_buffer_mutex);
    }
}

/* Start ADC DMA. */
int adc_init(void) {
    /* Start DMA for ADC1. */
    int status = HAL_ADC_Start_DMA(&hadc1, _adc1_buffer, ADC1_SIZE);
    if(status != HAL_OK) {
        DEBUG_PRINTLN("ERROR: Failed to start ADC DMA for ADC1 (Status: %d/%s).", status, hal_status_toString(status));
        return U_ERROR;
    }

    /* Start DMA for ADC2. */
    status = HAL_ADC_Start_DMA(&hadc2, _adc2_buffer, ADC2_SIZE);
    if(status != HAL_OK) {
        DEBUG_PRINTLN("ERROR: Failed to start ADC DMA for ADC2 (Status: %d/%s).", status, hal_status_toString(status));
        return U_ERROR;
    }
    return U_SUCCESS;
}

/* Get raw eFuse ADC Data. */
raw_efuse_adc_t adc_getEFuseData(void) {
    mutex_get(&adc1_mutex);
    mutex_get(&mux_buffer_mutex);

    raw_efuse_adc_t efuses;
    efuses.dashboard = _get_adc1_value(ADC1_EFUSE_DASHBOARD);
    efuses.brake = _get_adc1_value(ADC1_EFUSE_BRAKE);
    efuses.shutdown = _get_adc1_value(ADC1_EFUSE_SHUTDOWN);
    efuses.radfan = _get_adc1_value(ADC1_EFUSE_RADFAN);
    efuses.fanbatt = _get_adc1_value(ADC1_EFUSE_FANBATT);
    efuses.pump1 = _get_adc1_value(ADC1_EFUSE_PUMP1);
    efuses.pump2 = _get_adc1_value(ADC1_EFUSE_PUMP2);
    efuses.battbox = _get_adc1_value(ADC1_EFUSE_BATTBOX);
    efuses.mc = _get_adc1_value(ADC1_EFUSE_MC);

    mutex_put(&adc1_mutex);
    mutex_put(&mux_buffer_mutex);
    return efuses;
}

/* Get raw pedal sensor ADC Data. */
raw_pedal_adc_t adc_getPedalData(void) {
    mutex_get(&adc1_mutex);
    mutex_get(&adc2_mutex);

    raw_pedal_adc_t sensors;
    sensors.accel_1 = _get_adc1_value(ADC1_ACCEL_PEDAL_1);
    sensors.accel_2 = _get_adc1_value(ADC1_ACCEL_PEDAL_2);
    sensors.brake_1 = _get_adc2_value(ADC2_BRAKE_PEDAL_1);
    sensors.brake_2 = _get_adc2_value(ADC2_BRAKE_PEDAL_2);

    mutex_put(&adc1_mutex);
    mutex_put(&adc2_mutex);
    return sensors;
}