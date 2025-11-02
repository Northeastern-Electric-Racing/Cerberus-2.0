#ifndef __U_ADC_H
#define __U_ADC_H

#include <stdint.h>

/* File for managing ADC order. */

/* API */
int adc_init(void);
int adc_switchMuxState(void); /* "Switches" the multiplexer (if it's currently HIGH, it will become LOW; if it's currently LOW, it will become HIGH). Then, updates the mux buffer accordingly. */

/* Get raw EFuse ADC Data. */
typedef struct {
    uint16_t dashboard;
    uint16_t brake;
    uint16_t shutdown;
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