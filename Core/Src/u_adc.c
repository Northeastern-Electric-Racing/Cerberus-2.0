#include <stdint.h>
#include "main.h"
#include "u_mutexes.h"
#include "u_general.h"
#include "u_adc.h"

/* ADC1 Config. */
typedef enum {
    /* The order of items in this enum MUST match how the ADC ranks are set up in CubeMX. */

    /* EFuses. */
    ADC1_EFUSE_DASHBOARD,
    ADC1_EFUSE_BRAKE,
    ADC1_EFUSE_SHUTDOWN,
    ADC1_EFUSE_LV,
    ADC1_EFUSE_RADFAN,
    ADC1_EFUSE_FANBATT,
    ADC1_EFUSE_PUMP1,
    ADC1_EFUSE_PUMP2,
    ADC1_EFUSE_BATTBOX,
    ADC1_EFUSE_MC,

    /* Acceleration Pedal Sensors. */
    ADC1_ACCEL_PEDAL_1, /* Sensor 1 for the Acceleration Pedal. */
    ADC1_ACCEL_PEDAL_2, /* Sensor 2 for the Acceleration Pedal. */
    
    /* Total number of indexes for ADC1. */
    ADC1_SIZE
} adc1_t;
static uint32_t _adc1_buffer[ADC1_SIZE]; // Buffer for the ADC DMA readings.

/* ADC2 Config. */
typedef enum {
    /* The order of items in this enum MUST match how the ADC ranks are set up in CubeMX. */

    /* Brake Pedal Sensors. */
    ADC2_BRAKE_PEDAL_1, /* Sensor 1 for the Brake Pedal. */
    ADC2_BRAKE_PEDAL_2, /* Sensor 2 for the Brake Pedal. */

    /* Total number of indexes for ADC2. */
    ADC2_SIZE
} adc2_t;
static uint32_t _adc2_buffer[ADC2_SIZE]; // Buffer for the ADC DMA readings.

uint16_t _get_adc1_value(adc1_t index) {
    uint16_t temp;
    mutex_get(&adc1_mutex);
    temp = (uint16_t)(_adc1_buffer[index]);
    mutex_put(&adc1_mutex);
    return temp;
}

uint16_t _get_adc2_value(adc2_t index) {
    uint16_t temp;
    mutex_get(&adc2_mutex);
    temp = (uint16_t)(_adc2_buffer[index]);
    mutex_put(&adc2_mutex);
    return temp;
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
    raw_efuse_adc_t efuses;
    efuses.dashboard = _get_adc1_value(ADC1_EFUSE_DASHBOARD);
    efuses.brake = _get_adc1_value(ADC1_EFUSE_BRAKE);
    efuses.shutdown = _get_adc1_value(ADC1_EFUSE_SHUTDOWN);
    efuses.lv = _get_adc1_value(ADC1_EFUSE_LV);
    efuses.radfan = _get_adc1_value(ADC1_EFUSE_RADFAN);
    efuses.fanbatt = _get_adc1_value(ADC1_EFUSE_FANBATT);
    efuses.pump1 = _get_adc1_value(ADC1_EFUSE_PUMP1);
    efuses.pump2 = _get_adc1_value(ADC1_EFUSE_PUMP2);
    efuses.battbox = _get_adc1_value(ADC1_EFUSE_BATTBOX);
    efuses.mc = _get_adc1_value(ADC1_EFUSE_MC);
    return efuses;
}

/* Get raw pedal sensor ADC Data. */
raw_pedal_adc_t adc_getPedalData(void) {
    raw_pedal_adc_t sensors;
    sensors.accel_1 = _get_adc1_value(ADC1_ACCEL_PEDAL_1);
    sensors.accel_2 = _get_adc1_value(ADC1_ACCEL_PEDAL_2);
    sensors.brake_1 = _get_adc2_value(ADC2_BRAKE_PEDAL_1);
    sensors.brake_2 = _get_adc2_value(ADC2_BRAKE_PEDAL_2);
    return sensors;
}