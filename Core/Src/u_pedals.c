#include "main.h"
#include "timer.h"
#include "debounce.h"
#include "u_can.h"
#include "tx_api.h"
#include "u_pedals.h"
#include "u_queues.h"
#include "u_mutexes.h"
#include "u_faults.h"
#include "u_general.h"
#include "u_efuses.h"
#include "u_dti.h"
#include "u_statemachine.h"

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

/**
 * @brief Determine if power to the motor controller should be disabled based on brake and accelerator pedal travel.
 * @param percentage_accel Percent travel of the accelerator pedal from 0-1
 * @param percentage_brake Brake pressure sensor reading, 0-1
 * @return bool True for prefault conditions met, false for no prefault.
 */
static bool _calc_bspd_prefault(float percentage_accel, float percentage_brake, float dc_current)
{
	static bool motor_disabled = false;

	/* EV.4.7: If brakes are engaged and APPS signals more than 25% pedal travel, disable power
	to the motor(s). Re-enable when accelerator has less than 5% pedal travel. */

	if (percentage_brake > PEDAL_HARD_BRAKE_THRESH && percentage_accel > 0.25) {
		motor_disabled = true;
		queue_send(&faults, &(fault_t){BSPD_PREFAULT});
	}

	/* Prevent a fault. */
	if (percentage_brake > PEDAL_HARD_BRAKE_THRESH && dc_current > 10) {
		motor_disabled = true;
		queue_send(&faults, &(fault_t){BSPD_PREFAULT});
	}

	if (motor_disabled) {
		if (percentage_accel < 0.05) {
			motor_disabled = false;
		}
	}

	return motor_disabled;
}

#ifndef POWER_REGRESSION_PEDAL_TORQUE_TRANSFER
/* Linearlly translates the "amount pressed" percentage of the acceleration pedal to torque. */
/* (i.e. This function creates a constant rate of torque increase relative to pedal travel). */
static void _linear_accel_to_torque(float percentage_accel)
{
	/* Sometimes, the pedal travel jumps to 3% even if it is not pressed. */
	if (percentage_accel < 0.03) {
		percentage_accel = 0.0;
	}
	if (percentage_accel > 1) {
		percentage_accel = 1.0;
	}

	/* Linearly map acceleration to torque */
	int16_t torque = (int16_t)(percentage_accel * MAX_TORQUE);

	dti_set_torque(torque);
}

#else
/* Non-linearlly translates the "amount pressed" percentage of the acceleration pedal to torque. */
/* (i.e. This function makes the pedal less sensitive at lower positions, and more agressive at higher positions). */
static void _power_regression_accel_to_torque(float percentage_accel)
{
	/* Sometimes, the pedal travel jumps to 1% even if it is not pressed. */
	if (fabs(percentage_accel - 0.01) < 0.001) {
		percentage_accel = 0;
	}
	/*  map acceleration to torque */
	int16_t torque =
		(int16_t)(0.137609 * powf(percentage_accel, 1.43068) * MAX_TORQUE);
	/* These values came from creating a power regression function intersecting three points: (0,0) (20,10) & (100,100)*/

	dti_set_torque(torque);
}
#endif

/**
 * @brief Derate torque target to keep car below the maximum pit/reverse mode speed.
 * 
 * @param mph Speed of the car
 * @param percentage_accel Percent travel of the acceleration pedal from 0-1
 * @return int16_t Derated torque
 */
static int16_t _derate_torque(float mph, float percentage_accel)
{
	static int16_t torque_accumulator[TORQUE_ACCUMULATOR_SIZE];
	/* index in moving average */
	static uint8_t index = 0;

	int16_t torque;

	/* If we are going too fast, we don't want to apply any torque to the moving average */
	if (mph > PIT_MAX_SPEED) {
		torque = 0;
	} else {
		/* Highest torque % in pit mode */
		static const float max_torque_percent = 0.3;
		/* Linearly derate torque from 30% to 0% as speed increases */
		float torque_derating_factor =
			max_torque_percent -
			(max_torque_percent / PIT_MAX_SPEED);
		percentage_accel *= torque_derating_factor;
		torque = MAX_TORQUE * percentage_accel;
	}

	/* Add value to moving average */
	torque_accumulator[index] = torque;
	index = (index + 1) % TORQUE_ACCUMULATOR_SIZE;

	/* Get moving average then send torque command to dti motor controller */
	int16_t sum = 0;
	for (uint8_t i = 0; i < TORQUE_ACCUMULATOR_SIZE; i++) {
		sum += torque_accumulator[i];
	}
	return sum / TORQUE_ACCUMULATOR_SIZE;
}

/**
 * @brief Calculate and send torque command to motor controller.
 * 
 * @param percentage_accel Accelerator pedal percent travel from 0-1
 */
static void _accel_pedal_regen_torque(float percentage_accel)
{
	/* Coefficient to map accel pedal travel % to the max torque */
	float coeff = (MAX_TORQUE * torque_limit_percentage);

	/* Makes acceleration pedal more sensitive since domain is compressed but range is the same */
	uint16_t torque = coeff * (percentage_accel - ACCELERATION_THRESHOLD);

	/* Limit torque percentage wise in endurance mode */
	if (torque > MAX_TORQUE * torque_limit_percentage) {
		torque = MAX_TORQUE * torque_limit_percentage;
	}

	dti_set_torque(torque);
}

/**
 * @brief Calculate regen braking AC current target based on accelerator pedal percent travel.
 * 
 * @param percentage_accel Accelerator pedal percent travel from 0-1
 */
static void _accel_pedal_regen_braking(float percentage_accel)
{
	uint16_t regen_limit = pedals_getRegenLimit();

	/* Calculate AC current target for regenerative braking */
	float regen_current =
		(regen_limit / REGEN_THRESHOLD) * (REGEN_THRESHOLD - percentage_accel);

	if (regen_current > regen_limit) {
		regen_current = regen_limit;
	}

	/* Send regen current to motor controller */
	dti_set_regen((uint16_t)(regen_current * 10));
}

/* Implements Launch Control. */
/* (i.e. Prevents the car from accelerating too aggressively from a standstill, helping to maintain traction). */
static void _launch_control(float mph, float percentage_accel)
{
	static float last_mph = 0.0f;
	static uint32_t prevTime = 0;
	static float prev_accel = 0;

    const float deltaMPHPS_max = 22.0f; // Miles per hour per second, based on matlab accel numbers
    const float max_limiting_mph = 30;

	if (prevTime == 0) { // Initialize time
		prevTime = HAL_GetTick();
		return;
	}

	uint32_t now = HAL_GetTick();
	uint32_t delta_ms = now - prevTime;

	float delta_mph = mph - last_mph;
	float max_delta_adjusted = deltaMPHPS_max * (delta_ms / 1000.0f);

	if (mph < max_limiting_mph && delta_mph > max_delta_adjusted) {
		_linear_accel_to_torque(prev_accel / 2);
	} else {
		_linear_accel_to_torque(percentage_accel);
	}

	// Update for next cycle
	prevTime = now;
	last_mph = mph;
	prev_accel = percentage_accel;
}

/* Manages torque control when the car is in Performance Mode. */
static void _handle_performance(float mph, float percentage_accel)
{
#ifndef POWER_REGRESSION_PEDAL_TORQUE_TRANSFER
	uint16_t regen_limit = pedals_getRegenLimit();
	if (regen_limit <= 0.01) {
		_linear_accel_to_torque(percentage_accel);
		return;
	}

	if (percentage_accel >= ACCELERATION_THRESHOLD) {
		if (launch_control_enabled) {
			_launch_control(mph, (percentage_accel - 0.25) / 0.75);
		} else {
			_accel_pedal_regen_torque(percentage_accel);
		}
	} else if (mph * MPH_TO_KMH > 5 && percentage_accel <= REGEN_THRESHOLD) {
		_accel_pedal_regen_braking(percentage_accel);
	} else {
		/* Pedal travel is between thresholds, so there should not be acceleration or braking */
		dti_set_torque(0);
	}
#else
	power_regression_accel_to_torque(percentage_accel);
#endif
}

/**
 * @brief Torque calculations for efficiency mode. If the driver is braking, do regenerative braking.
 * 
 * @param mph mph of the car
 * @param percentage_accel adjusted value of the acceleration pedal
 */
static void _handle_endurance(float mph, float percentage_accel)
{
	/* Pedal is in acceleration range. Set forward torque target. */
	if (percentage_accel >= ACCELERATION_THRESHOLD) {
		_accel_pedal_regen_torque(percentage_accel);
	} else if (mph * MPH_TO_KMH > 5 && percentage_accel <= REGEN_THRESHOLD) {
		_accel_pedal_regen_braking(percentage_accel);
	} else {
		/* Pedal travel is between thresholds, so there should not be acceleration or braking */
		dti_set_torque(0);
	}
}

/**
 * @brief Drive forward with a speed limit of 5 mph.
 * 
 * @param mph Current speed of the car.
 * @param percentage_accel % pedal travel of the accelerator pedal.
 */
static void _handle_pit(float mph, float percentage_accel)
{
	dti_set_torque(_derate_torque(mph, percentage_accel));
}

/**
 * @brief Drive in speed limited reverse mode.
 * 
 * @param mph Current speed of the car.
 * @param percentage_accel % pedal travel of the accelerator pedal.
 */
static void _handle_reverse(float mph, float percentage_accel)
{
	dti_set_torque(-1 * _derate_torque(fabs(mph), percentage_accel));
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

/* Initializes Pedals ADC and creates pedal data timer. */
int pedals_init(void) {
    /* Start ADC DMA */
    int status = HAL_ADC_Start_DMA(&hadc1, _buffer, NUM_SENSORS); // u_TODO - gotta correct this once pedals ADC stuff is set up in CubeMX. hadc1 is for the efuses not pedals
    if(status != HAL_OK) {
        DEBUG_PRINTLN("ERROR: Failed to start ADC DMA for pedals (Status: %d/%s).", status, hal_status_toString(status));
        return U_ERROR;
    }

    /* Create Pedal Data Timer. */
    status = tx_timer_create(
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
bool pedals_getBrakeState(void) {
    bool temp;
    mutex_get(&brake_state_mutex);
    temp = brake_pressed;
    mutex_put(&brake_state_mutex);
    return temp;
}

/* Returns the torque limit percentgae. */
float pedals_getTorqueLimitPercentage(void) {
	float temp;
	mutex_get(&torque_limit_mutex);
	temp = torque_limit_percentage;
	mutex_put(&torque_limit_mutex);
	return temp;
}

void pedals_setTorqueLimitPercentage(float percentage) {
	mutex_get(&torque_limit_mutex);
	torque_limit_percentage = percentage;
	mutex_put(&torque_limit_mutex);
}

void pedals_increaseTorqueLimit(void)
{
	mutex_get(&torque_limit_mutex);
	if (torque_limit_percentage + 0.1 > 1) {
		torque_limit_percentage = 1;
	} else {
		torque_limit_percentage += 0.1;
	}
	mutex_put(&torque_limit_mutex);
}

void pedals_decreaseTorqueLimit(void)
{
	mutex_get(&torque_limit_mutex);
	if (torque_limit_percentage - 0.1 < 0) {
		torque_limit_percentage = 0;
	} else {
		torque_limit_percentage -= 0.1;
	}
	mutex_put(&torque_limit_mutex);
}

void pedals_increaseRegenLimit(void)
{
	func_state_t func_state = get_func_state();
	if (func_state != F_PERFORMANCE && func_state != F_EFFICIENCY)
		return;
	uint16_t regen_limit = pedals_getRegenLimit();
	if (regen_limit + REGEN_INCREMENT_STEP > MAX_REGEN_CURRENT) {
		pedals_setRegenLimit(MAX_REGEN_CURRENT);
	} else {
		pedals_setRegenLimit(regen_limit += REGEN_INCREMENT_STEP);
	}
}

void pedals_decreaseRegenLimit(void)
{
	func_state_t func_state = get_func_state();
	if (func_state != F_PERFORMANCE && func_state != F_EFFICIENCY)
		return;
	uint16_t regen_limit = pedals_getRegenLimit();
	if (regen_limit - REGEN_INCREMENT_STEP < 0) {
		pedals_setRegenLimit(0);
	} else {
		pedals_setRegenLimit(regen_limit -= REGEN_INCREMENT_STEP);
	}
}

void pedals_setRegenLimit(uint16_t limit)
{
	func_state_t func_state = get_func_state();
	if (func_state != F_PERFORMANCE && func_state != F_EFFICIENCY)
		return;
	if (limit > MAX_REGEN_CURRENT) {
		regen_limits[func_state - F_PERFORMANCE] = MAX_REGEN_CURRENT;
	} else if (limit < 0.0) {
		regen_limits[func_state - F_PERFORMANCE] = 0.0;
	} else {
		regen_limits[func_state - F_PERFORMANCE] = limit;
	}
}

uint16_t pedals_getRegenLimit(void)
{
	func_state_t func_state = get_func_state();
	if (func_state != F_PERFORMANCE && func_state != F_EFFICIENCY) {
		return 0;
	}
	return regen_limits[get_func_state() - F_PERFORMANCE];
}

void pedals_toggleLaunchControl(void)
{
	launch_control_enabled = !launch_control_enabled;
}

bool pedals_getLaunchControl(void)
{
	return launch_control_enabled;
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
        efuse_enable(ef_break); // u_TODO - i'm pretty sure this is for the brakelight? but idk why its spelled like that in altium
    }
    else {
        brake_pressed = false;
        efuse_disable(ef_break);
    }
    mutex_put(&brake_state_mutex);

	uint16_t dc_current = dti_get_dc_current();
    float mph = dti_get_mph();

	if (_calc_bspd_prefault(pedal_data.percentage_accel, pedal_data.percentage_brake, dc_current)) {
		/* Prefault triggered */
		// dti_set_torque(0);
		// osDelay(delay_time);
    }

    switch(statemachine_getFunctionalState()) {
        case READY:
            dti_set_torque(0);
            break;
        case FAULTED:
            dti_set_torque(0);
            break;
        case F_PIT:
            _handle_pit(mph, pedal_data.percentage_accel);
            break;
        case F_REVERSE:
            _handle_reverse(mph, pedal_data.percentage_accel);
            break;
        case F_PERFORMANCE:
            _handle_performance(mph, pedal_data.percentage_accel);
            break;
        case F_EFFICIENCY:
            _handle_efficiency(mph, pedal_data.percentage_accel);
            break;
        default:
            DEBUG_PRINTLN("ERROR: Failed to process pedals due to unknown functional state.");
            break;
    }

    /* Return the pedal data mutex. */
    mutex_put(&pedal_data_mutex);

    return;
}