#include "u_pedals.h"
#include "u_buttons.h"
#include <stdatomic.h>
#include "main.h"
#include "timer.h"
#include "debounce.h"
#include "can_messages_tx.h"
#include "c_utils.h"

/* Globals. */
static uint16_t regen_limits[2] = { 0, 50 }; // [PERFORMANCE, ENDURANCE]
static const float MPH_TO_KMH = 1.609;       // Factor for converting MPH to KMH

typedef enum {
    BRAKE_OC,
    BRAKE_SC,
    ACCEL_OC,
    ACCEL_SC,
    ACCEL_DIFF,
    BSPD_PREF,
	BMS_NOT_PRECHARGED_YET,
    NUM_LOCKS,
} drive_lock_t; // Add to this enum anything that can lock the drive
static uint8_t drive_lock_map = 0;

static _Atomic bool brake_pressed = false;
static _Atomic bool accel_pressed = false;
static _Atomic bool launch_control_enabled = false;
static float torque_limit_percentage = 1.0f;

/* Pedal Data. */
typedef struct {
	float voltage_accel1;
	float voltage_accel2;
	float voltage_brake1;
	float voltage_brake2;
	float percentage_accel;
	float percentage_brake;
	float psi_brake1;
	float psi_brake2;
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
#define MAX_APPS1_VOLTS		    3.4 // (Volts). Upper bound on APPS1 voltage range.
#define MIN_APPS1_VOLTS		    2.1 // (Volts). Lower bound on APPS1 voltage range.
#define MAX_APPS2_VOLTS		    2.2 // (Volts). Upper bound on APPS2 voltage range.
#define MIN_APPS2_VOLTS		    1.1 // (Volts). Lower bound on APPS2 voltage range.
#define PEDAL_BRAKE_THRESH	    0.15 // (Percantage). Pedal position above which the system registers the brake pedal as "pressed".
#define PEDAL_HARD_BRAKE_THRESH 0.20 // (Percentage). Pedal position above which a "hard brake" is detected.

/* Performance Limits */
#define PIT_MAX_SPEED           5.0 // (mph). Speed limit in pit mode.
#define MAX_TORQUE              160 // (Nm). Maximum torque output
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
#define APPS_THRESHOLD_TOLERANCE    0.20 // (Volts). Tolerance margin around the accelerator pedal.
#define BRAKE_THRESHOLD_TOLERANCE   0.25 // (Volts). Tolerance margin around the brake pedal.



struct drive_mode{
    void (*handle)(float, float);
    void (*button_functions[20])();
};





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


/* =================================== */
/*            DRIVE HANDLES            */
/* =================================== */

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

/* Converts the ADC to the voltage out of 5V (for rules). */
static float _adc_to_voltage(uint16_t raw_adc) {
    float v3_volts = raw_adc * MAX_VOLTS / MAX_ADC_VAL_12b;
	// undo 2k + 3k voltage divider on APPS lines
	return ((2000.0 + 3000) / 3000) * v3_volts;
}


/* */

struct drive_mode performance = {
    .handle = &_handle_performance,
    .button_functions = {&_handle_performance}
};


struct drive_mode endurance = {
    .handle = &_handle_endurance,
    .button_functions = {&_handle_performance}
};

struct drive_mode pit = {
    .handle = &_handle_pit,
    .button_functions = {&_handle_performance}
};

struct drive_mode reverse = {
    .handle = &_handle_reverse,
    .button_functions = {&_handle_performance}
};


