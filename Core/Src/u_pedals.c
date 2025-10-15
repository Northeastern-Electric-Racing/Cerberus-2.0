#include "main.h"
#include "timer.h"
#include "debounce.h"
#include "c_utils.h"
#include "u_can.h"
#include "tx_api.h"
#include "u_pedals.h"
#include "u_queues.h"
#include "u_mutexes.h"
#include "u_faults.h"
#include "u_tx_debug.h"
#include "u_efuses.h"
#include "u_dti.h"
#include "u_statemachine.h"
#include "u_adc.h"

/* Pedal sensors. This enum is ordered based on the order of the sensors' ADC indexes, as set up in u_adc.c  */
typedef enum {
    ACCEL_PEDAL_1, /* Sensor 1 for the Acceleration Pedal. */
    ACCEL_PEDAL_2, /* Sensor 2 for the Acceleration Pedal. */
    BRAKE_PEDAL_1, /* Sensor 1 for the Brake Pedal. */
    BRAKE_PEDAL_2, /* Sensor 2 for the Brake Pedal. */
    
    /* Total number of pedal sensors. */
    NUM_SENSORS
} pedal_sensor_t;

/* Globals. */
static float torque_limit_percentage = 1.0;
static uint16_t regen_limits[2] = { 0, 50 }; // [PERFORMANCE, ENDURANCE]
static bool launch_control_enabled = false;
static const float MPH_TO_KMH = 1.609;       // Factor for converting MPH to KMH
static bool brake_pressed = false;
static TX_TIMER pedal_data_timer;            // Timer for sending pedal data message.

/* Pedal Data. */
typedef struct {
	float voltage_accel1;
	float voltage_accel2;
	float voltage_brake1;
	float voltage_brake2;
	float percentage_accel;
	float percentage_brake;
} pedal_data_t;
static pedal_data_t pedal_data = { 0 };

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
#define MAX_APPS1_VOLTS		    3.03 // (Volts). Upper bound on APPS1 voltage range.
#define MIN_APPS1_VOLTS		    1.81 // (Volts). Lower bound on APPS1 voltage range.
#define MAX_APPS2_VOLTS		    2.28 // (Volts). Upper bound on APPS2 voltage range.
#define MIN_APPS2_VOLTS		    1.07 // (Volts). Lower bound on APPS2 voltage range.
#define PEDAL_BRAKE_THRESH	    0.20 // (Percantage). Pedal position above which the system registers the brake pedal as "pressed".
#define PEDAL_HARD_BRAKE_THRESH 0.50 // (Percentage). Pedal position above which a "hard brake" is detected.

/* Performance Limits */
#define PIT_MAX_SPEED           5.0 // (mph). Speed limit in pit mode.
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

/* Send Pedal Data Callback */
static void _send_pedal_data(ULONG args) {
    (void)args; // The args parameter is unused for this callback.

    /* Get the Pedal Data Mutex */
    mutex_get(&pedal_data_mutex);

    /* Pedal Volts Message. */
    can_msg_t pedals_volts_msg = { .id = CANID_PEDALS_VOLTS_MSG, .len = 8, .data = { 0 } };
    struct __attribute__((__packed__)) {
		uint16_t accel_1;
		uint16_t accel_2;
		uint16_t brake_1;
		uint16_t brake_2;
	} pedal_volts_data;

    /* Normalized Pedals Message. */
    can_msg_t pedals_norm_msg = { .id = CAN_ID_PEDALS_NORM_MSG, .len = 4, .data = { 0 } };
	struct __attribute((__packed__)) {
		uint16_t accel_norm;
		uint16_t brake_norm;
	} pedal_norm_data;

    /* Fill the Pedal Volts Message with the data. */
	pedal_volts_data.accel_1 = (uint16_t)(pedal_data.voltage_accel1 * 100);
	pedal_volts_data.accel_2 = (uint16_t)(pedal_data.voltage_accel2 * 100);
	pedal_volts_data.brake_1 = (uint16_t)(pedal_data.voltage_brake1 * 100);
	pedal_volts_data.brake_2 = (uint16_t)(pedal_data.voltage_brake2 * 100);
	endian_swap(&pedal_volts_data.accel_1, sizeof(pedal_volts_data.accel_1));
	endian_swap(&pedal_volts_data.accel_2, sizeof(pedal_volts_data.accel_2));
	endian_swap(&pedal_volts_data.brake_1, sizeof(pedal_volts_data.brake_1));
	endian_swap(&pedal_volts_data.brake_2, sizeof(pedal_volts_data.brake_2));
    memcpy(pedals_volts_msg.data, &pedal_volts_data, pedals_volts_msg.len);

    /* Fill the Normalized Pedals Message with the data. */
	pedal_norm_data.accel_norm = (uint16_t)(pedal_data.percentage_accel * 100);
	pedal_norm_data.brake_norm = (uint16_t)(pedal_data.percentage_brake * 100);
	endian_swap(&pedal_norm_data.accel_norm, sizeof(pedal_norm_data.accel_norm));
	endian_swap(&pedal_norm_data.brake_norm, sizeof(pedal_norm_data.brake_norm));
    memcpy(pedals_norm_msg.data, &pedal_norm_data, pedals_norm_msg.len);

    /* Queue the Messages. */
    queue_send(&can_outgoing, &pedals_volts_msg);
    queue_send(&can_outgoing, &pedals_norm_msg);

    /* Put the Pedal Data Mutex. */
    mutex_put(&pedal_data_mutex);
}

/* Calculates brake faults. */
static void _calculate_brake_faults(float voltage_brake1, float voltage_brake2) {
    /* Debounce Timers */
    static nertimer_t open_circuit_timer;  // Timer for the Open Circuit Fault
    static nertimer_t short_circuit_timer; // Timer for the Short Circuit Fault

    /* EV3.5.4: For analog acceleration control signals, this error checking must detect open circuit, short to ground and short to sensor power. */
    
    /* Open Circuit Fault */
    bool open_circuit_fault = (voltage_brake1 > BRAKE_SENSOR_IRREGULAR_HIGH + BRAKE_THRESHOLD_TOLERANCE) || (voltage_brake2 > BRAKE_SENSOR_IRREGULAR_HIGH + BRAKE_THRESHOLD_TOLERANCE);
    debounce(open_circuit_fault, &open_circuit_timer, BRAKE_FAULT_DEBOUNCE, &_open_circuit_fault_callback, NULL);

    /* Short Circuit Fault */
    bool short_circuit_fault = (voltage_brake1 < BRAKE_SENSOR_IRREGULAR_LOW - BRAKE_THRESHOLD_TOLERANCE) || (voltage_brake2 < BRAKE_SENSOR_IRREGULAR_LOW - BRAKE_THRESHOLD_TOLERANCE);
    debounce(short_circuit_fault, &short_circuit_timer, BRAKE_FAULT_DEBOUNCE, &_short_circuit_fault_callback, NULL);
}

/* Calculates Pedal Faults. */
static void _calculate_accel_faults(float voltage_accel1, float voltage_accel2, float percentage_accel1, float percentage_accel2) {
    /* Debounce Timers */
    static nertimer_t open_circuit_timer;     // Timer for the Open Circuit Fault
    static nertimer_t short_circuit_timer;    // Timer for the Short Circuit Fault
    static nertimer_t pedal_difference_timer; // Timer for the Pedal Difference Fault

    /* EV3.5.4: For analog acceleration control signals, this error checking must detect open circuit, short to ground and short to sensor power. */

    /* Open Circuit Fault */
    bool open_circuit_fault = (voltage_accel1 > MAX_VOLTS_UNSCALED - APPS_THRESHOLD_TOLERANCE) || (voltage_accel2 > MAX_VOLTS_UNSCALED - APPS_THRESHOLD_TOLERANCE);
    debounce(open_circuit_fault, &open_circuit_timer, PEDAL_FAULT_DEBOUNCE, &_open_circuit_fault_callback, NULL);

    /* Short Circuit Fault */
    bool short_circuit_fault = (voltage_accel1 < MIN_APPS1_VOLTS - APPS_THRESHOLD_TOLERANCE) || (voltage_accel2 < MIN_APPS2_VOLTS - APPS_THRESHOLD_TOLERANCE);
    debounce(short_circuit_fault, &short_circuit_timer, PEDAL_FAULT_DEBOUNCE, &_short_circuit_fault_callback, NULL);

    /* Pedal Difference Fault */
    /* Detects if the two accelerator pedal sensors give readings that differ by more than PEDAL_DIFF_THRESH. */
    bool pedal_difference_fault = fabs(percentage_accel1 - percentage_accel2) > PEDAL_DIFF_THRESH;
    debounce(pedal_difference_fault, &pedal_difference_timer, PEDAL_FAULT_DEBOUNCE, &_pedal_difference_fault_callback, NULL);
}

/* Returns the raw ADC readings for a pedal sensor. */
static uint16_t _get_raw_adc_reading(pedal_sensor_t pedal_sensor) {
	uint16_t reading;
	switch(pedal_sensor) {
		case ACCEL_PEDAL_1:
			reading = adc_getPedalData().accel_1;
			break;
		case ACCEL_PEDAL_2:
			reading = adc_getPedalData().accel_2;
			break;
		case BRAKE_PEDAL_1:
			reading = adc_getPedalData().brake_1;
			break;
		case BRAKE_PEDAL_2:
			reading = adc_getPedalData().brake_2;
			break;
		default:
			DEBUG_PRINTLN("ERROR: Unknown pedal sensor enum passed into function.");
			return U_ERROR;
	}
	return reading;
}

/* Converts the ADC to the voltage out of 5V (for rules). */
static float _get_sensor_voltage(pedal_sensor_t pedal_sensor) {
	uint16_t adc_data = _get_raw_adc_reading(pedal_sensor);
    float v3_volts = adc_data * MAX_VOLTS / MAX_ADC_VAL_12b;
	// undo 2k + 3k voltage divider on APPS lines
	return ((2000.0 + 3000) / 3000) * v3_volts;
}

/* Returns the percentage the pedal is pressed down. */
static float _get_pedal_percent_pressed(float voltage, float offset, float max)
{
	return voltage - offset < 0 ? 0 : (voltage - offset) / (max - offset);
}

/* Initializes Pedals ADC and creates pedal data timer. */
int pedals_init(void) {

    /* Create Pedal Data Timer. */
    int status = tx_timer_create(
        &pedal_data_timer,        /* Timer Instance */
        "Pedal Data Timer",       /* Timer Name */
        _send_pedal_data,         /* Timer Expiration Callback */
        0,                        /* Callback Input */
        PEDAL_DATA_MSG_FREQUENCY, /* Ticks until timer expiration. */
        PEDAL_DATA_MSG_FREQUENCY, /* Number of ticks for all timer expirations after the first (0 makes this a one-shot timer). */
        TX_AUTO_ACTIVATE          /* Automatically start the timer. */
    );
    if(status != TX_SUCCESS) {
        DEBUG_PRINTLN("ERROR: Failed to create Pedal Data Timer (Status: %d/%s).", status, tx_status_toString(status));
        return U_ERROR;
    }

	DEBUG_PRINTLN("Ran pedals_init().");

    return U_SUCCESS;
}

/* Returns the brake state (true=brake pressed, false=brake not pressed). */
bool pedals_isBrakePressed(void) {
    bool temp;
    mutex_get(&brake_state_mutex);
    temp = brake_pressed;
    mutex_put(&brake_state_mutex);
    return temp;
}

float pedals_getBrakePercentage(void) {
	float temp;
	mutex_get(&pedal_data_mutex);
	temp = pedal_data.percentage_brake;
	mutex_put(&pedal_data_mutex);
	return temp;
}

/* Gets the percentage pressed of the acceleration pedal. */
float pedals_getAccelerationPercentage(void) {
	float temp;
	mutex_get(&pedal_data_mutex);
	temp = pedal_data.percentage_accel;
	mutex_put(&pedal_data_mutex);
	return temp;
}

/* Pedal Processing Function. Meant to be called by the pedals thread. */
void pedals_process(void) {

    /* Get the pedal data mutex. */
    mutex_get(&pedal_data_mutex);

    /* Get pedal voltage data. */
    pedal_data.voltage_accel1 = _get_sensor_voltage(ACCEL_PEDAL_1);
	pedal_data.voltage_accel2 = _get_sensor_voltage(ACCEL_PEDAL_2);
	pedal_data.voltage_brake1 = _get_sensor_voltage(BRAKE_PEDAL_1);
	pedal_data.voltage_brake2 = _get_sensor_voltage(BRAKE_PEDAL_2);

    /* Calculate acceleration pedal percentage pressed. */
    float accel1_percentage = _get_pedal_percent_pressed(pedal_data.voltage_accel1, MIN_APPS1_VOLTS, MAX_APPS1_VOLTS); // For sensor 1...
    float accel2_percentage = _get_pedal_percent_pressed(pedal_data.voltage_accel2, MIN_APPS2_VOLTS, MAX_APPS2_VOLTS); // For sensor 2...
    pedal_data.percentage_accel = (accel1_percentage + accel2_percentage) / 2; /* Record the averaged percentage. */
    _calculate_accel_faults(pedal_data.voltage_accel1, pedal_data.voltage_accel2, accel1_percentage, accel2_percentage); // Check for faults.

    /* Calculate brake pedal percentage pressed. */
    // u_TODO - this is slightly different to how its done in Cerberus (1.0). I changed it to match how acceleration pedal percentages are calculated, but maybe brake percentage isn't supposed to be calculated this way?
    float brake1_percentage = _get_pedal_percent_pressed(pedal_data.voltage_brake1, 0, MAX_VOLTS_UNSCALED); // For sensor 1...
    float brake2_percentage = _get_pedal_percent_pressed(pedal_data.voltage_brake2, 0, MAX_VOLTS_UNSCALED); // For sensor 2...
    pedal_data.percentage_brake = (brake1_percentage + brake2_percentage) / 2; /* Record the averaged percentage. */
    _calculate_brake_faults(pedal_data.voltage_brake1, pedal_data.voltage_brake2); // Check for faults.

    /* Set brake state, and turn brakelight on/off. */
    mutex_get(&brake_state_mutex);
    if(pedal_data.percentage_brake > PEDAL_BRAKE_THRESH) {
        brake_pressed = true;
        efuse_enable(EFUSE_BRAKE);
    }
    else {
        brake_pressed = false;
        efuse_disable(EFUSE_BRAKE);
    }
    mutex_put(&brake_state_mutex);

    /* Return the pedal data mutex. */
    mutex_put(&pedal_data_mutex);

    return;
}