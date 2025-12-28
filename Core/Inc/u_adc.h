#ifndef __U_ADC_H
#define __U_ADC_H

#include <stdint.h>
#include "u_pedals.h"
#include "u_efuses.h"

/* File for managing ADC order. */

/* API */
int adc_init(void);
int adc_switchMuxState(void); /* "Switches" the multiplexer (if it's currently HIGH, it will become LOW; if it's currently LOW, it will become HIGH). Then, updates the mux buffer accordingly. */

/* Get raw EFuse ADC Data. */
typedef struct { uint16_t data[NUM_EFUSES]; } raw_efuse_adc_t; // Struct to store the data.
int adc_getEFuseData(raw_efuse_adc_t* buffer);

/* Get raw pedal sensor ADC data. */
typedef struct { uint16_t data[NUM_PEDALS]; } raw_pedal_adc_t; // Struct to store the data.
int adc_getPedalData(raw_pedal_adc_t* buffer);

/* Get LFIU sensor ADC data. */
typedef struct { uint16_t data[NUM_LFIU]; } raw_lfiu_adc_t; // Struct to store the data.
int adc_getLfiuData(raw_lfiu_adc_t* buffer);

/* Get LV_BATT Voltage ADC data. */
typedef struct { 
    uint16_t raw; 
    float voltage; 
} lvread_adc_t; // Struct to store the data.
int adc_getLVData(lvread_adc_t* buffer);

#endif /* u_adc.h */