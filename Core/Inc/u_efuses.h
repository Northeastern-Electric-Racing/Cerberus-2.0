#ifndef __U_EFUSES_H
#define __U_EFUSES_H

#include <stdint.h>
#include <stdbool.h>
#include "main.h"

typedef enum {
    EFUSE_DASHBOARD,
    EFUSE_BRAKE,
    EFUSE_SHUTDOWN,
    EFUSE_LV,
    EFUSE_RADFAN,
    EFUSE_FANBATT,
    EFUSE_PUMP1,
    EFUSE_PUMP2,
    EFUSE_BATTBOX,
    EFUSE_MC,

    /* Total number of efuses. */
    NUM_EFUSES
} efuse_t;

/* API */
uint16_t efuse_getRaw(efuse_t efuse);      // Returns the eFuse's raw ADC reading.
float efuse_getVoltage(efuse_t efuse);     // Returns the eFuse's voltage reading.
float efuse_getCurrent(efuse_t efuse);     // Returns the eFuse's current reading.
bool efuse_getFaultStatus(efuse_t efuse);  // Returns the eFuse's fault status (true = faulted, false = not faulted).
void efuse_enable(efuse_t efuse);          // Enables an eFuse.
void efuse_disable(efuse_t efuse);         // Disables an eFuse.
bool efuse_getEnableStatus(efuse_t efuse); // Returns whether or not the eFuse is enabled (true = eFuse is enabled, false = eFuse is disabled).

#endif /* u_efuses.h */