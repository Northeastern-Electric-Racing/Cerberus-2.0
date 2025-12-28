#ifndef __U_PEDALS_H
#define __U_PEDALS_H

/* Pedal sensors. This enum is ordered based on the order of the sensors' ADC indexes, as set up in u_adc.c  */
typedef enum {
    PEDAL_ACCEL1, /* Sensor 1 for the Acceleration Pedal. */
    PEDAL_ACCEL2, /* Sensor 2 for the Acceleration Pedal. */
    PEDAL_BRAKE1, /* Sensor 1 for the Brake Pedal. */
    PEDAL_BRAKE2, /* Sensor 2 for the Brake Pedal. */
    
    /* Total number of pedal sensors. */
    NUM_PEDALS
} pedal_t;

/* LFIU Sensors. */
typedef enum {
    LFIU_1,     /* LFIU Current Sensor 1. */
    LFIU_2,     /* LFIU Current Sensor 2. */

    /* Total number of LFIU current sensors. */
    NUM_LFIU
} lfiu_t;

/* API */
int pedals_init(void);                                  // Initializes Pedals ADC and creates pedal data timer.
int pedals_process(void);                              // Pedal Processing Function. Meant to be called by the pedals thread.
bool pedals_getBrakeState(void);                        // Returns the brake state (true=brake pressed, false=brake not pressed).
float pedals_getTorqueLimitPercentage(void);            // Returns the torque limit percentage.
void pedals_setTorqueLimitPercentage(float percentage); // Sets the torque limit percentage.
void pedals_increaseTorqueLimit(void);                  // Increases torque limit.
void pedals_decreaseTorqueLimit(void);                  // Decreases torque limit.
void pedals_increaseRegenLimit(void);                   // Increase Regen Limit.
void pedals_decreaseRegenLimit(void);                   // Decrease Regen Limit.
void pedals_setRegenLimit(uint16_t limit);              // Sets regen limit.
uint16_t pedals_getRegenLimit(void);                    // Sets regen limit.
void pedals_toggleLaunchControl(void);                  // Toggles launch control.
bool pedals_getLaunchControl(void);                     // Gets launch control.

#endif /* u_pedals.h */