#ifndef __U_ADC_H
#define __U_ADC_H

#include <stdint.h>

/* File for managing ADC order. */

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

/* ADC2 Config. */
typedef enum {
    /* The order of items in this enum MUST match how the ADC ranks are set up in CubeMX. */

    /* Brake Pedal Sensors. */
    ADC2_BRAKE_PEDAL_1, /* Sensor 1 for the Brake Pedal. */
    ADC2_BRAKE_PEDAL_2, /* Sensor 2 for the Brake Pedal. */

    /* Total number of indexes for ADC2. */
    ADC2_SIZE
} adc2_t;

/* API */
int adc_init(void);

/* Get raw EFuse ADC Data. */
typedef struct {
    uint16_t dashboard;
    uint16_t brake;
    uint16_t shutdown;
    uint16_t lv;
    uint16_t radfan;
    uint16_t fanbatt;
    uint16_t pump1;
    uint16_t pump2;
    uint16_t battbox;
    uint16_t mc;
} raw_efuse_adc_t; // Struct to store the data.
raw_efuse_adc_t adc_getEFuseData(void);

/* Get raw pedal sensor ADC data. */
typedef struct {
    uint16_t accel_1;
    uint16_t accel_2;
    uint16_t brake_1;
    uint16_t brake_2;
} raw_pedal_adc_t; // Struct to store the data.
raw_pedal_adc_t adc_getPedalData(void);

#endif /* u_adc.h */