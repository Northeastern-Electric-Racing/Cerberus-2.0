#include "main.h"
#include "timer.h"
#include "u_pedals.h"
#include "u_queues.h"
#include "u_faults.h"
#include "u_general.h"

/* Pedal sensors. This enum is ordered based on each sensor's ADC rank, which corresponds to the index of each sensor's data in the ADC buffer.  */
typedef enum {
    ACCEL_PEDAL_1, /* Sensor 1 for the Acceleration Pedal. */
    ACCEL_PEDAL_2, /* Sensor 2 for the Acceleration Pedal. */
    BRAKE_PEDAL_1, /* Sensor 1 for the Brake Pedal. */
    BRAKE_PEDAL_2, /* Sensor 2 for the Brake Pedal. */
    
    /* Total number of pedal sensors. */
    NUM_SENSORS
} pedal_sensor_t;
// u_TODO - once pedal ADC stuff is set up in CubeMX, make sure this order is accurate.

/* Globals. */
static float torque_limit_percentage = 1.0;
static uint16_t regen_limits[2] = { 0, 50 }; // [PERFORMANCE, ENDURANCE]
static bool launch_control_enabled = false;
static uint32_t _buffer[NUM_SENSORS];        // Buffer to hold Pedal ADC readings. Each index corresponds to a different eFuse.
static const float MPH_TO_KMH = 1.609;       // Factor for converting MPH to KMH
static bool brake_pressed = false;

/* Pedal Data. */
typedef struct {
	float accel1_volts;
	float accel2_volts;
	float brake1_volts;
	float brake2_volts;
	float accel_norm;
	float brake_norm;
} pedal_data_t;
static pedal_data_t pedal_data;

/* =================================== */
/*            CONFIG MACROS            */
/* =================================== */
/* ADC Stuff */
#define MAX_ADC_VAL_12b    4096       // Maximum value for a 12-bit ADC.

/* Motor Control Timing/Safety */
#define MIN_COMMAND_FREQ     60                      // (Hz). Minimum frequency for sending torque commands.
#define MAX_COMMAND_DELAY    1000 / MIN_COMMAND_FREQ // (ms). Maximum delay between torque commands.
#define REGEN_INCREMENT_STEP 10                      // (AC Amps). Steo size for increasing/decreasing regenerative braking current.

/* Voltage Stuff */
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

/* Fault Debounce Callbacks */
static void _open_circuit_fault_callback(void *arg) {queue_send(&faults, &(fault_t){ONBOARD_PEDAL_OPEN_CIRCUIT_FAULT});};   // Queues the Open Circuit Fault.
static void _short_circuit_fault_callback(void *arg) {queue_send(&faults, &(fault_t){ONBOARD_PEDAL_SHORT_CIRCUIT_FAULT});}; // Queues the Short Circuit Fault.
static void _pedal_difference_fault_callback(void *arg) {queue_send(&faults, &(fault_t){ONBOARD_PEDAL_DIFFERENCE_FAULT});}; // Queues the Pedal Difference Fault.

/* Calculates brake faults. */
static void _calculate_brake_faults(float voltage_brake1, float voltage_brake2) {
    /* Debounce Timers */
    static nertimer_t open_circuit_timer;  // Timer for the Open Circuit Fault
    static nertimer_t short_circuit_timer; // Timer for the Short Circuit Fault

    /* EV3.5.4: For analog acceleration control signals, this error checking must detect open circuit, short to ground and short to sensor power. */
    
    /* Open Circuit Fault */
    bool open_circuit_fault = (voltage_brake1 > BRAKE_SENSOR_IRREGULAR_HIGH + BRAKE_THRESHOLD_TOLERANCE) || (voltage_brake2 > BRAKE_SENSOR_IRREGULAR_HIGH + BRAKE_THRESHOLD_TOLERANCE);
    debounce(open_circuit_fault, &_open_circuit_fault_callback, BRAKE_FAULT_DEBOUNCE, NULL);

    /* Short Circuit Fault */
    bool short_circuit_fault = (voltage_brake1 < BRAKE_SENSOR_IRREGULAR_LOW - BRAKE_THRESHOLD_TOLERANCE) || (voltage_brake2 < BRAKE_SENSOR_IRREGULAR_LOW - BRAKE_THRESHOLD_TOLERANCE);
    debounce(short_circuit_fault, &_short_circuit_fault_callback, BRAKE_FAULT_DEBOUNCE, NULL);
}

/* Calculates Pedal Faults. */
static void _calculate_pedal_faults(float voltage_accel1, float voltage_accel2, float percentage_accel1, float percentage_accel2) {
    /* Debounce Timers */
    static nertimer_t open_circuit_timer;     // Timer for the Open Circuit Fault
    static nertimer_t short_circuit_timer;    // Timer for the Short Circuit Fault
    static nertimer_t pedal_difference_timer; // Timer for the Pedal Difference Fault

    /* EV3.5.4: For analog acceleration control signals, this error checking must detect open circuit, short to ground and short to sensor power. */

    /* Open Circuit Fault */
    bool open_circuit_fault = (voltage_accel1 > MAX_VOLTS_UNSCALED - APPS_THRESHOLD_TOLERANCE) || (voltage_accel2 > MAX_VOLTS_UNSCALED - APPS_THRESHOLD_TOLERANCE);
    debounce(open_circuit_fault, &_open_circuit_fault_callback, PEDAL_FAULT_DEBOUNCE, NULL);

    /* Short Circuit Fault */
    bool short_circuit_fault = (voltage_accel1 < MIN_APPS1_VOLTS - APPS_THRESHOLD_TOLERANCE) || (voltage_accel2 < MIN_APPS2_VOLTS - APPS_THRESHOLD_TOLERANCE);
    debounce(short_circuit_fault, &_short_circuit_fault_callback, PEDAL_FAULT_DEBOUNCE, NULL);

    /* Pedal Difference Fault */
    /* Detects if the two accelerator pedal sensors give readings that differ by more than PEDAL_DIFF_THRESH. */
    bool pedal_difference_fault = fabs(percentage_accel1 - percentage_accel2) > PEDAL_DIFF_THRESH;
    debounce(pedal_difference_fault, &_pedal_difference_fault_callback, PEDAL_FAULT_DEBOUNCE, NULL);
}

/* Returns the raw ADC readings for a pedal sensor. */
static uint16_t _get_raw_adc_reading(pedal_sensor_t pedal_sensor) {
    return _buffer[pedal_sensor];
}

/* Converts the ADC to the voltage out of 5V (for rules). */
static float _get_sensor_voltage(pedal_sensor_t pedal_sensor) {
    float v3_volts = _buffer[pedal_sensor] * MAX_VOLTS / MAX_ADC_VAL_12b;
	// undo 2k + 3k voltage divider on APPS lines
	return ((2000.0 + 3000) / 3000) * v3_volts;
}

/* Returns the percentage the pedal is pressed down. */
static float _get_pedal_percent_pressed(float voltage, float offset, float max)
{
	return voltage - offset < 0 ? 0 : (voltage - offset) / (max - offset);
}

/* Initializes Pedals ADC */
int pedals_init(void) {
    /* Start ADC DMA */
    HAL_StatusTypeDef status = HAL_ADC_Start_DMA(&hadc1, _buffer, NUM_SENSORS); // u_TODO - gotta correct this once pedals ADC stuff is set up in CubeMX. hadc1 is for the efuses not pedals
    if(status != HAL_OK) {
        DEBUG_PRINTLN("ERROR: Failed to start ADC DMA for pedals (Status: %d/%s).", status, hal_status_toString(status));
        return U_ERROR;
    }

    return U_SUCCESS;
}

/* Pedal Processing Function. Meant to be called by the pedals thread. */
void pedals_process(void) {
    return; // u_TODO - implement this. Maybe make stuff like calculate_brake_faults, calculate_pedal_faults, pedals_getRaw, pedals_getSensorVoltage, etc. static functions
}