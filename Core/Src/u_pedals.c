#include <stdatomic.h>
#include "main.h"
#include "timer.h"
#include "debounce.h"
#include "can_messages_tx.h"
#include "c_utils.h"
#include "u_tx_timers.h"
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
    NUM_LOCKS,
} drive_lock_t; // Add to this enum anything that can lock the drive
static uint8_t drive_lock_map = 0;

static _Atomic bool brake_pressed = false;
static _Atomic bool launch_control_enabled = false;
static _Atomic float torque_limit_percentage = 1.0f;

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


/* Set a drive lock, remember to unset when the fault condition disappears*/
static void drive_lock_set(drive_lock_t lock) {
    PRINTLN_INFO("Drive Lock %d set", lock);
    NER_SET_BIT(drive_lock_map, lock);
}
/* Unset drive lock */
static void drive_lock_unset(drive_lock_t lock) {
    PRINTLN_INFO("Drive Lock %d unset", lock);
    NER_CLEAR_BIT(drive_lock_map, lock);
}

/* Unset drive lock */
static bool is_drive_locked(void) {
    return drive_lock_map > 0;
}


static void _onboard_brake_open_circuit_fault_callback(void *arg) {
    queue_send(&faults, &(fault_t){ONBOARD_BRAKE_OPEN_CIRCUIT_FAULT}, TX_NO_WAIT);
    drive_lock_set(BRAKE_OC);
} // Queues the Brake Open Circuit Fault.
static void _onboard_brake_short_circuit_fault_callback(void *arg) {
    queue_send(&faults, &(fault_t){ONBOARD_BRAKE_SHORT_CIRCUIT_FAULT}, TX_NO_WAIT);
    drive_lock_set(BRAKE_SC);
} // Queues the Brake Short Circuit Fault.
static void _onboard_accel_open_circuit_fault_callback(void *arg) {
    queue_send(&faults, &(fault_t){ONBOARD_ACCEL_OPEN_CIRCUIT_FAULT}, TX_NO_WAIT);
    drive_lock_set(ACCEL_OC);
} // Queues the Accel Open Circuit Fault.
static void _onboard_accel_short_circuit_fault_callback(void *arg) {
    queue_send(&faults, &(fault_t){ONBOARD_ACCEL_SHORT_CIRCUIT_FAULT}, TX_NO_WAIT);
    drive_lock_set(ACCEL_SC);
} // Queues the Accel Short Circuit Fault.
static void _pedal_difference_fault_callback(void *arg) {
    queue_send(&faults, &(fault_t){ONBOARD_PEDAL_DIFFERENCE_FAULT}, TX_NO_WAIT);
    drive_lock_set(ACCEL_DIFF);
} // Queues the Pedal Difference Fault.


/* Send Pedal Data Callback */
static void _send_pedal_data(ULONG args) {
    (void)args; // The args parameter is unused for this callback.

    /* Send Pedal Volts Message. */
	send_pedal_sensor_voltages(
		pedal_data.voltage_accel1,
		pedal_data.voltage_accel2,
		pedal_data.voltage_brake1,
		pedal_data.voltage_brake2
	);

	/* Send Pedals Percent Pressed Message. */
	send_pedal_percent_pressed_values(
		pedal_data.percentage_accel,
		pedal_data.percentage_brake
	);
}

/* Pedal Data Timer. */
static timer_t pedal_data_timer = {
	.name = "Pedal Data Timer",
	.callback = _send_pedal_data,
	.callback_input = 0,
	.duration = PEDAL_DATA_MSG_FREQUENCY,
	.type = PERIODIC,
	.auto_activate = true
};

/* Calculates brake faults. */
static void _calculate_brake_faults(float voltage_brake1, float voltage_brake2) {
    /* Debounce Timers */
    static nertimer_t open_circuit_timer;  // Timer for the Open Circuit Fault
    static nertimer_t short_circuit_timer; // Timer for the Short Circuit Fault

    /* EV3.5.4: For analog acceleration control signals, this error checking must detect open circuit, short to ground and short to sensor power. */

    /* Open Circuit Fault */
    bool open_circuit_fault = (voltage_brake1 > BRAKE_SENSOR_IRREGULAR_HIGH + BRAKE_THRESHOLD_TOLERANCE) || (voltage_brake2 > BRAKE_SENSOR_IRREGULAR_HIGH + BRAKE_THRESHOLD_TOLERANCE);
    debounce(open_circuit_fault, &open_circuit_timer, BRAKE_FAULT_DEBOUNCE, &_onboard_brake_open_circuit_fault_callback, NULL);
    if (!open_circuit_fault) {
        drive_lock_unset(BRAKE_OC);
    }

    /* Short Circuit Fault */
    bool short_circuit_fault = (voltage_brake1 < BRAKE_SENSOR_IRREGULAR_LOW - BRAKE_THRESHOLD_TOLERANCE) || (voltage_brake2 < BRAKE_SENSOR_IRREGULAR_LOW - BRAKE_THRESHOLD_TOLERANCE);
    debounce(short_circuit_fault, &short_circuit_timer, BRAKE_FAULT_DEBOUNCE, &_onboard_brake_short_circuit_fault_callback, NULL);
    if (!short_circuit_fault) {
        drive_lock_unset(BRAKE_SC);
    }
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
    debounce(open_circuit_fault, &open_circuit_timer, PEDAL_FAULT_DEBOUNCE, &_onboard_accel_open_circuit_fault_callback, NULL);
    if (!open_circuit_fault) {
        drive_lock_unset(ACCEL_OC);
    }

    /* Short Circuit Fault */
    bool short_circuit_fault = (voltage_accel1 < MIN_APPS1_VOLTS - APPS_THRESHOLD_TOLERANCE) || (voltage_accel2 < MIN_APPS2_VOLTS - APPS_THRESHOLD_TOLERANCE);
    debounce(short_circuit_fault, &short_circuit_timer, PEDAL_FAULT_DEBOUNCE, &_onboard_accel_short_circuit_fault_callback, NULL);
    if (!short_circuit_fault) {
        drive_lock_unset(ACCEL_SC);
    }

    /* Pedal Difference Fault */
    /* Detects if the two accelerator pedal sensors give readings that differ by more than PEDAL_DIFF_THRESH. */
    bool pedal_difference_fault = fabs(percentage_accel1 - percentage_accel2) > PEDAL_DIFF_THRESH;
    debounce(pedal_difference_fault, &pedal_difference_timer, PEDAL_FAULT_DEBOUNCE, &_pedal_difference_fault_callback, NULL);
    if (!pedal_difference_fault) {
        drive_lock_unset(ACCEL_DIFF);
    }
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
		queue_send(&faults, &(fault_t){BSPD_PREFAULT}, TX_NO_WAIT);
	}

	/* Prevent a fault. */
	if (percentage_brake > PEDAL_HARD_BRAKE_THRESH && dc_current > 10) {
		motor_disabled = true;
		queue_send(&faults, &(fault_t){BSPD_PREFAULT}, TX_NO_WAIT);
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

/* Converts the ADC to the voltage out of 5V (for rules). */
static float _adc_to_voltage(uint16_t raw_adc) {
    float v3_volts = raw_adc * MAX_VOLTS / MAX_ADC_VAL_12b;
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
    int status = timer_init(&pedal_data_timer);
    if(status != U_SUCCESS) {
        PRINTLN_ERROR("Failed to create Pedal Data Timer (Status: %d).", status);
        return U_ERROR;
    }

	PRINTLN_INFO("Ran pedals_init().");

    return U_SUCCESS;
}

/* Returns the brake state (true=brake pressed, false=brake not pressed). */
bool pedals_getBrakeState(void) {
    return brake_pressed;
}

/* Returns the torque limit percentgae. */
float pedals_getTorqueLimitPercentage(void) {
	return torque_limit_percentage;
}

void pedals_setTorqueLimitPercentage(float percentage) {
	torque_limit_percentage = percentage;
}

void pedals_increaseTorqueLimit(void)
{
	if (torque_limit_percentage + 0.1 > 1) {
		torque_limit_percentage = 1;
	} else {
		torque_limit_percentage += 0.1;
	}
}

void pedals_decreaseTorqueLimit(void)
{
	if (torque_limit_percentage - 0.1 < 0) {
		torque_limit_percentage = 0;
	} else {
		torque_limit_percentage -= 0.1;
	}
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

    /* Get pedal voltage data. */
	raw_pedal_adc_t raw = adc_getPedalData();
    pedal_data.voltage_accel1 = _adc_to_voltage(raw.data[PEDAL_ACCEL1]);
	pedal_data.voltage_accel2 = _adc_to_voltage(raw.data[PEDAL_ACCEL2]);
	pedal_data.voltage_brake1 = _adc_to_voltage(raw.data[PEDAL_BRAKE1]);
	pedal_data.voltage_brake2 = _adc_to_voltage(raw.data[PEDAL_BRAKE2]);

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
    if(pedal_data.percentage_brake > PEDAL_BRAKE_THRESH) {
        brake_pressed = true;
        efuse_enable(EFUSE_BRAKE);
    }
    else {
        brake_pressed = false;
        efuse_disable(EFUSE_BRAKE);
    }

	uint16_t dc_current = dti_get_dc_current();
    float mph = dti_get_mph();

	if (_calc_bspd_prefault(pedal_data.percentage_accel, pedal_data.percentage_brake, dc_current)) {
		/* Prefault triggered */
		drive_lock_set(BSPD_PREF);
    } else {
        drive_lock_unset(BSPD_PREF);
    }

	// if we have a drive lock condition, set torque to zero and bail
	if (is_drive_locked()) {
        dti_set_torque(0);
	    return;
	}

    switch(get_func_state()) {
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
            _handle_endurance(mph, pedal_data.percentage_accel);
            break;
        default:
            PRINTLN_ERROR("Failed to process pedals due to unknown functional state.");
            break;
    }

    return;
}

_Static_assert(NUM_LOCKS <= sizeof(drive_lock_map) * 8, "Increase the drive lock map to accomodate more drive locks."); // Ensures there aren't more locks than the bitmap can hold.
