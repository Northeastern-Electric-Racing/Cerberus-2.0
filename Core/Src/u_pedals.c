#include "u_pedals.h"

/* =================================== */
/*            CONFIG MACROS            */
/* =================================== */

/* Motor Control Timing/Safety */
#define MIN_COMMAND_FREQ     60                      // (Hz). Minimum frequency for sending torque commands.
#define MAX_COMMAND_DELAY    1000 / MIN_COMMAND_FREQ // (ms). Maximum delay between torque commands.
#define REGEN_INCREMENT_STEP 10                      // (AC Amps). Steo size for increasing/decreasing regenerative braking current.

/* Voltage Conversion/ADC Stuff */
#define MAX_ADC_VAL_12b    4096 // (Maximum value for a 12-bit ADC).
#define MAX_VOLTS          3.3  // (Volts). Maximum voltage for the ADC.
#define MAX_VOLTS_UNSCALED 5.0  // (Volts). Actual sensor voltage before voltage divider scaling.

/* Pedal Tuning */
#define MAX_APPS1_VOLTS		    3.03 // (Volts). Upper bound on APPS1 voltage range.
#define MIN_APPS1_VOLTS		    1.81 // (Volts). Lower bound on APPS1 voltage range.
#define MAX_APPS2_VOLTS		    2.28 // (Volts). Upper bound on APPS2 voltage range.
#define MIN_APPS2_VOLTS		    1.07 // (Volts). Lower bound on APPS2 voltage range.
#define PEDAL_BRAKE_THRESH	    0.20 // (Percantage). Pedal position above which the system registers the brake pedal as "pressed".
#define PEDAL_HARD_BRAKE_THRESH 0.50 // (Percentage). Pedal position above which a "hard brake" is detected.

/* Performance Limits */
#define PIT_MAX_SPEED           5.0 // (mph). Speed limit in pit mode.
#define MAX_TORQUE              220 // (Nm). Maximum torque output
#define TORQUE_ACCUMULATOR_SIZE 10  // (Number). Size of the moving average filter for torque stuff.
#define MAX_REGEN_CURRENT       250 // (AC Amps). Maximum regenerative braking current.

/* Endurance Mode */
#define ACCELERATION_THRESHOLD 0.25 // (Percentage). Pedal position above which acceleration begins.
#define REGEN_THRESHOLD 0.10        // (Percentage). Pedal position below which regenerative braking activates.

/* Fault Detection */
#define BRAKE_SENSOR_IRREGULAR_HIGH 4.5  // (Volts). The brake sensor voltage should not exceed this value.
#define BRAKE_SENSOR_IRREGULAR_LOW  0.5  // (Volts). The brake sensor voltage should not go below this value.
#define PEDAL_DIFF_THRESH           0.20 // (Percentage). Maximum allowed difference between the two accelerator sensors.
#define PEDAL_FAULT_DEBOUNCE        95   // (ms). Debounce time for pedal faults.
#define BRAKE_FAULT_DEBOUNCE        300  // (ms). Debounce time for brake faults.
#define APPS_THRESHOLD_TOLERANCE    0.45 // (Volts). Tolerance margin around the accelerator pedal.
#define BRAKE_THRESHOLD_TOLERANCE   0.25 // (Volts). Tolerance margin around the brake pedal.