#ifndef __U_EFUSES_H
#define __U_EFUSES_H

#include <stdint.h>
#include <stdbool.h>
#include "main.h"

/* Override Calypso define. When uncommented, VCU eFuses will ignore all Calypso commands and fall back to its default state as specified in the eFuse config table. */
//#define EFUSES_OVERRIDE_CALYPSO

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
    EFUSE_SPARE,

    /* Total number of efuses. */
    NUM_EFUSES
} efuse_t;

/* Enum for the eFuse control states. */
typedef enum {
    EF_ON = 0,     // eFuse is enabled (on)
    EF_AUTO = 1,   // eFuse will turn on/off automatically based on sensor readings. For eFuses with no AUTO mode, AUTO will do the same thing as ON.
    EF_OFF = 2     // eFuse is disabled (off)
} efuse_control_state_t;

/* Struct for holding eFuse data. */
typedef struct {
    uint16_t raw[NUM_EFUSES];   // eFuse's raw ADC reading.
    float voltage[NUM_EFUSES];  // eFuse's voltage reading.
    float current[NUM_EFUSES];  // eFuse's current reading.
    bool faulted[NUM_EFUSES];   // eFuse's faulted state (true = faulted, false = not faulted).
    bool enabled[NUM_EFUSES];   // eFuse's enabled state (true = eFuse is enabled, false = eFuse is disabled).
    efuse_control_state_t control_state[NUM_EFUSES]; // The eFuse's control state as tracked by VCU. Calypso is the source of truth for these values; VCU is simply responding to Calypso's commanded state values.
} efuse_data_t;

/* API */
efuse_data_t efuse_getData(void);  // Returns an instance of efuse_data_t with all current eFuse data.
void efuse_enable(efuse_t efuse);  // Enables an eFuse.
void efuse_disable(efuse_t efuse); // Disables an eFuse.
void efuse_update_state(efuse_t efuse, efuse_control_state_t state); // Updates an eFuse's control state. Intended to be called when the relevant commands from Calypso are received.
int efuse_init(void); // Inititialize the _efuse_control_state array to the values configured in the efuses[] table.
efuse_control_state_t efuse_get_state(efuse_t efuse); // Gets the control state of the eFuse. 

#endif /* u_efuses.h */