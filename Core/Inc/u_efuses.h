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

/* Struct for holding eFuse data. */
typedef struct {
    uint16_t raw[NUM_EFUSES];   // eFuse's raw ADC reading.
    float voltage[NUM_EFUSES];  // eFuse's voltage reading.
    float current[NUM_EFUSES];  // eFuse's current reading.
    bool faulted[NUM_EFUSES];   // eFuse's faulted state (true = faulted, false = not faulted).
    bool enabled[NUM_EFUSES];   // eFuse's enabled state (true = eFuse is enabled, false = eFuse is disabled).
} efuse_data_t;

/* API */
efuse_data_t efuse_getData(void);  // Returns an instance of efuse_data_t with all current eFuse data.
void efuse_enable(efuse_t efuse);  // Enables an eFuse.
void efuse_disable(efuse_t efuse); // Disables an eFuse.

#endif /* u_efuses.h */