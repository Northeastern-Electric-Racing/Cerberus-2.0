#ifndef __U_PERIPHERALS_H
#define __U_PERIPHERALS_H

#include "main.h"

/* File for VCU's I2C/SPI peripherals. */

/* API */
int peripherals_init(void);                        /* Initializes I2C/SPI devices. */
int tempsensor_toggleHeater(bool enable);          /* Toggles the status of the temperature sensor's internal heater. */
int tempsensor_getTemperature(float *temperature); /* Gets the temp sensor's temperature reading. */
int tempsensor_getHumidity(float *humidity);       /* Gets the temp sensor's humidity reading. */

#endif /* u_peripherals.h */