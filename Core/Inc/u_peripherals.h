#ifndef __U_PERIPHERALS_H
#define __U_PERIPHERALS_H

#include "main.h"
#include "lsm6dso.h"

/* File for VCU's I2C/SPI peripherals. */

/* API */
int peripherals_init(void);                         /* Initializes I2C/SPI devices. */
int tempsensor_toggleHeater(bool enable);           /* Toggles the status of the temperature sensor's internal heater. */
int tempsensor_getTemperature(float *temperature);  /* Gets the temp sensor's temperature reading. */
int tempsensor_getHumidity(float *humidity);        /* Gets the temp sensor's humidity reading. */
int imu_getAccelerometerData(LSM6DSO_Axes_t *axes); /* Gets the accelerometer axes (x, y, and z). */
int imu_getGyroscopeData(LSM6DSO_Axes_t *axes);     /* Gets the gyroscope axes (x, y, and z). */

#endif /* u_peripherals.h */