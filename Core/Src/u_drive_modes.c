#include "u_drive_modes.h"

#include "u_pedals.h"
#include "u_buttons.h"
#include <stdatomic.h>
#include "main.h"
#include "timer.h"
#include "debounce.h"
#include "can_messages_tx.h"
#include "c_utils.h"
#include "u_statemachine.h"
#include "u_tc.h"
#include "u_tx_debug.h"

/* Globals. */
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

static float cruise_speed_setpoint = 20;

static test_mode_t nero_test_mode = CRUISE_CONTROL;
static test_mode_t test_mode = UNSELECTED;

#ifndef POWER_REGRESSION_PEDAL_TORQUE_TRANSFER
/* Linearlly translates the "amount pressed" percentage of the acceleration pedal to torque. */
/* (i.e. This function creates a constant rate of torque increase relative to pedal travel). */
static void _linear_accel_to_torque(float percentage_accel)
{
	/* Sometimes, the pedal travel jumps to 3% even if it is not pressed. */
	if (percentage_accel < 0.03) {
		percentage_accel = 0.0f;
	}
	if (percentage_accel > 1) {
		percentage_accel = 1.0f;
	}

	/* Linearly map acceleration to torque, scaled by TC */
	int16_t torque = (int16_t)(percentage_accel * MAX_TORQUE * tc_get_torque_scale());

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
		(int16_t)(0.137609 * powf(percentage_accel, 1.43068) * MAX_TORQUE * tc_get_torque_scale());
	/* These values came from creating a power regression function intersecting three points: (0,0) (20,10) & (100,100)*/

	dti_set_torque(torque);
}
#endif

/**
 * @brief Calculate and send torque command to motor controller.
 *
 * @param percentage_accel Accelerator pedal percent travel from 0-1
 * @param torque_limit_percentage Torque limit percent from 0-1
 */
static void _accel_pedal_regen_torque(float percentage_accel, float torque_limit_percentage)
{
	/* Coefficient to map accel pedal travel % to the % of max torqye we should command */
	float coeff = tc_get_torque_scale() * (percentage_accel - ACCELERATION_THRESHOLD) / (1.0 - ACCELERATION_THRESHOLD);

	/* Makes acceleration pedal more sensitive since domain is compressed but range is the same */
	uint16_t torque = coeff * torque_limit_percentage * MAX_TORQUE;

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
		((regen_limit - MIN_REGEN_CURRENT) / REGEN_THRESHOLD) * (REGEN_THRESHOLD - percentage_accel) + MIN_REGEN_CURRENT;

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
		if (pedals_getLaunchControl()) {
			_launch_control(mph, (percentage_accel - 0.25) / 0.75);
		} else {
			_accel_pedal_regen_torque(percentage_accel, pedals_getTorqueLimitPercentage());
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
		_accel_pedal_regen_torque(percentage_accel, pedals_getTorqueLimitPercentage());
	} else if (mph * MPH_TO_KMH > 5 && percentage_accel <= REGEN_THRESHOLD) {
		_accel_pedal_regen_braking(percentage_accel);
	} else {
		/* Pedal travel is between thresholds, so there should not be acceleration or braking */
		dti_set_torque(0);
	}
}

/**
 * @brief Drive at a targeted speed with at the torque given by the accelerator pedal.
 *
 * @param mph mpf of the car
 * @param percentage_accel adjusted value of the acceleration pedal
 */
static void _handle_cruise(float mph, float percentage_accel)
{
  // TODO: use dti set_speed api, mph param is from dti get_mph already
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

/* Handles the drive mode based responses to the requested pedal acceleration */
void drive_process(float mph, float percentage_accel)
{
	send_test_modes(0, cruise_speed_setpoint, get_test_modes_disabled());

	switch(get_func_state()) {
		case READY:
		case FAULTED:
			dti_set_torque(0);
			break;
		case F_PIT:
			_handle_pit(mph, percentage_accel);
			break;
		case F_REVERSE:
			_handle_reverse(mph, percentage_accel);
			break;
		case F_PERFORMANCE:
			_handle_performance(mph, percentage_accel);
			break;
		case F_EFFICIENCY:
			_handle_endurance(mph, percentage_accel);
			break;
		case F_TEST_MODES:
			switch (test_mode)
			{
				case UNSELECTED:
					return;
				case CRUISE_CONTROL:
					_handle_cruise(mph, percentage_accel);
					break;
				default:
					PRINTLN_ERROR("Failed to process drive mode due to unknown test drive mode state.");
					dti_set_torque(0);
					break;
			}
			break;
		default:
			PRINTLN_ERROR("Failed to process drive mode due to drive mode state.");
			dti_set_torque(0);
			break;
	}
}
