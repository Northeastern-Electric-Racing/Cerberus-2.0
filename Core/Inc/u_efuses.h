#ifndef __U_EFUSES_H
#define __U_EFUSES_H

#include <stdint.h>
#include <stdbool.h>
#include "main.h"
#include "can_messages_tx.h"
#include "serial.h"
#include "u_dti.h"

/* Tempeature constants to control */

#define PUMP1_UPPER_MOTOR_TEMP	45
#define PUMP1_LOWER_MOTOR_TEMP	35
#define PUMP2_UPPER_CONTROLLER_TEMP	45
#define PUMP2_LOWER_CONTROLLER_TEMP	35
#define RADFAN_UPPER_MOTOR_TEMP 65
#define RADFAN_LOWER_MOTOR_TEMP 35

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
void efuse_enable(efuse_t efuse);  // Enables an eFuse.
void efuse_disable(efuse_t efuse); // Disables an eFuse.


/**
 * @brief Enables eFuses which should be on by default.
 * 
 * The following eFuses should be enabled by default:
 * - `EFUSE_DASHBOARD`
 * - `EFUSE_LV`
 * - `EFUSE_BATTBOX`
 * - `EFUSE_MC`
 * - `EFUSE_FANBATT`  ?? (We may need to do this conditionally based on pack temp)
 */
void efuse_init(void);

/**
 * @brief Updates the states of eFuses.
 * 
 * The following eFuses are controlled based on temperature:
 * - `EFUSE_PUMP1`
 *      - Turned on if the motor temperature exceeds `PUMP1_UPPER_MOTOR_TEMP`.
 *      - Turned off if the motor temperature falls below `PUMP1_LOWER_MOTOR_TEMP`.
 * - `EFUSE_PUMP2`
 *      - Turned on if the motor controller (MC) temperature exceeds `PUMP2_UPPER_CONTROLLER_TEMP`.
 *      - Turned off if the MC temperature falls below `PUMP2_LOWER_CONTROLLER_TEMP`.
 * - `EFUSE_RADFAN`
 *      - Turned on if the motor temperature exceeds `RADFAN_UPPER_MOTOR_TEMP`.
 *      - Turned off if the motor temperature falls below `RADFAN_LOWER_MOTOR_TEMP`.
 * 
 * Also, some eFuses should be updated according to the CAN message.
 */
void efuse_update(void);

void efuse_send_to_dashboard(void);
void efuse_send_to_serial(void);

#endif /* u_efuses.h */