#ifndef __U_PERIPHERALS_H
#define __U_PERIPHERALS_H

#include "main.h"
#include "lsm6dsv_reg.h"

/* File for VCU's I2C/SPI peripherals. */

typedef struct {
    float x;
    float y;
    float z;
} vector3_t;

/* API */
int peripherals_init(void);                         /* Initializes I2C/SPI devices. */
int tempsensor_toggleHeater(bool enable);           /* Toggles the status of the temperature sensor's internal heater. */
int tempsensor_getTemperature(float *temperature);  /* Gets the temp sensor's temperature reading. */
int tempsensor_getHumidity(float *humidity);        /* Gets the temp sensor's humidity reading. */
vector3_t imu_getAcceleration(void);                /* Gets the IMU's acceleration reading. */
vector3_t imu_getAngularRate(void);                 /* Gets the IMU's angular rate reading. */

#endif /* u_peripherals.h */