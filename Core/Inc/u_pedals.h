#include <stdbool.h>

#ifndef __U_PEDALS_H
#define __U_PEDALS_H

static uint16_t regen_limits[2] = { 0, 150 }; // [PERFORMANCE, ENDURANCE]
static const float MPH_TO_KMH = 1.609;       // Factor for converting MPH to KMH

/* =================================== */
/*            CONFIG MACROS            */
/* =================================== */
/* Misc */
#define MAX_ADC_VAL_12b    4096       // Maximum value for a 12-bit ADC.
#define PEDAL_DATA_MSG_FREQUENCY 100  // (Ticks). How often the pedal data message should get sent.

/* Motor Control Timing/Safety */
#define MIN_COMMAND_FREQ     60                      // (Hz). Minimum frequency for sending torque commands.
#define MAX_COMMAND_DELAY    1000 / MIN_COMMAND_FREQ // (ms). Maximum delay between torque commands.
#define REGEN_INCREMENT_STEP 10                      // (AC Amps). Steo size for increasing/decreasing regenerative braking current.

/* Voltage Stuff */
#define MAX_VOLTS          3.3  // (Volts). Maximum voltage for the ADC.
#define MAX_VOLTS_UNSCALED 5.0  // (Volts). Actual sensor voltage before voltage divider scaling.

/* Pedal Tuning */
#define MAX_APPS1_VOLTS		    3.4 // (Volts). Upper bound on APPS1 voltage range.
#define MIN_APPS1_VOLTS		    2.1 // (Volts). Lower bound on APPS1 voltage range.
#define MAX_APPS2_VOLTS		    2.2 // (Volts). Upper bound on APPS2 voltage range.
#define MIN_APPS2_VOLTS		    1.1 // (Volts). Lower bound on APPS2 voltage range.
#define PEDAL_BRAKE_THRESH	    0.15 // (Percantage). Pedal position above which the system registers the brake pedal as "pressed".
#define PEDAL_HARD_BRAKE_THRESH 0.20 // (Percentage). Pedal position above which a "hard brake" is detected.

/* Performance Limits */
#define PIT_MAX_SPEED           5.0 // (mph). Speed limit in pit mode.
#define MAX_TORQUE              214 // (Nm). Maximum torque output
#define TORQUE_ACCUMULATOR_SIZE 10  // (Number). Size of the moving average filter for torque stuff.
#define MAX_REGEN_CURRENT       400 // (AC Amps). Maximum regenerative braking current.

/* Endurance Mode */
#define ACCELERATION_THRESHOLD 0.30 // (Percentage). Pedal position above which acceleration begins.
#define REGEN_THRESHOLD 0.25        // (Percentage). Pedal position below which regenerative braking activates.

/* Fault Detection */
#define BRAKE_SENSOR_IRREGULAR_HIGH 4.5  // (Volts). The brake sensor voltage should not exceed this value.
#define BRAKE_SENSOR_IRREGULAR_LOW  0.5  // (Volts). The brake sensor voltage should not go below this value.
#define PEDAL_DIFF_THRESH           0.20 // (Percentage). Maximum allowed difference between the two accelerator sensors.
#define PEDAL_FAULT_DEBOUNCE        95   // (ms). Debounce time for pedal faults.
#define BRAKE_FAULT_DEBOUNCE        300  // (ms). Debounce time for brake faults.
#define APPS_THRESHOLD_TOLERANCE    0.20 // (Volts). Tolerance margin around the accelerator pedal.
#define BRAKE_THRESHOLD_TOLERANCE   0.25 // (Volts). Tolerance margin around the brake pedal.

/* Pedal sensors. This enum is ordered based on the order of the sensors' ADC
 * indexes, as set up in u_adc.c  */
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
  LFIU_1, /* LFIU Current Sensor 1. */
  LFIU_2, /* LFIU Current Sensor 2. */

  /* Total number of LFIU current sensors. */
  NUM_LFIU
} lfiu_t;

/* API */
int pedals_init(void); // Initializes Pedals ADC and creates pedal data timer.
void pedals_process(void); // Pedal Processing Function. Meant to be called by
                           // the pedals thread.
bool pedals_getBrakeState(void); // Returns the brake state (true=brake pressed,
                                 // false=brake not pressed).
bool pedals_getAccelState(void); // Returns the accel state (true=accel pressed,
                                 // false=accel not pressed).
float pedals_getTorqueLimitPercentage(
    void); // Returns the torque limit percentage.
void pedals_setTorqueLimitPercentage(
    float percentage);                     // Sets the torque limit percentage.
void pedals_increaseTorqueLimit(void);     // Increases torque limit.
void pedals_decreaseTorqueLimit(void);     // Decreases torque limit.
void pedals_increaseRegenLimit(void);      // Increase Regen Limit.
void pedals_decreaseRegenLimit(void);      // Decrease Regen Limit.
void pedals_setRegenLimit(uint16_t limit); // Sets regen limit.
uint16_t pedals_getRegenLimit(void);       // Sets regen limit.
void pedals_toggleLaunchControl(void);     // Toggles launch control.
void pedals_enableLaunchControl(void);     // Enables launch control.
void pedals_disableLaunchControl(void);    // Disables launch control.
bool pedals_getLaunchControl(void);        // Gets launch control.

void _handle_endurance(float mph, float percentage_accel);
#endif /* u_pedals.h */