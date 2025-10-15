#include <stdint.h>
#include <stdbool.h>
#include "u_drive.h"
#include "u_faults.h"
#include "u_pedals.h"
#include "u_queues.h"
#include "u_statemachine.h"

/* =================================== */
/*         DRIVE CONFIG MACROS         */
/* =================================== */

/* Performance Limits */
#define PIT_MAX_SPEED           5.0 // (mph). Speed limit in pit mode.
#define TORQUE_ACCUMULATOR_SIZE 10  // (Number). Size of the moving average filter for torque stuff.
#define MAX_REGEN_CURRENT       250 // (AC Amps). Maximum regenerative braking current.
#define REGEN_INCREMENT_STEP    10  // (AC Amps). Step size for increasing/decreasing regenerative braking current.

/* Drive Mode Thresholds */
#define ACCELERATION_THRESHOLD 0.25 // (Percentage). Pedal position above which acceleration begins.
#define REGEN_THRESHOLD 0.10        // (Percentage). Pedal position below which regenerative braking activates.

/* BSPD (Brake System Plausibility Device) */
#define PEDAL_HARD_BRAKE_THRESH 0.50 // (Percentage). Pedal position above which a "hard brake" is detected.

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

/* Handle torque when the car is in PERFORMANCE. */
#define _PERFORMANCE(x) 1*x /* Relationship between acceleration pedal percentage and torque percentage. */
void drive_handlePerformance(void) {
	/* CONFIG. */
	const uint16_t MAX_TORQUE = 220;
	
	/* Get target torque. */
	pedal_data_t pedal_data = pedals_getData();
	uint16_t torque = _PERFORMANCE(pedal_data.acceleration_percentage) * MAX_TORQUE;

	dti_set_torque(torque);
}

/* Handle torque when the car is in PIT. */
#define _PIT(x) 1*x /* Relationship between acceleration pedal percentage and torque percentage. */
void drive_handlePit(void) {
	/* CONFIG. */
	const uint16_t MAX_TORQUE = 10;
	const uint16_t MAX_MPH = 5;

	/* Get target torque. */
	pedal_data_t pedal_data = pedals_getData();
	uint16_t torque = _PIT(pedal_data.acceleration_percentage) * MAX_TORQUE;

	if(dti_get_mph() > MAX_MPH) {
		torque = 0;
	}

	dti_set_torque(torque);
}

/* Handle torque when the car is in REVERSE. */
#define _REVERSE(x) 1*x /* Relationship between acceleration pedal percentage and torque percentage. */
void drive_handleReverse(void) {
	/* CONFIG. */
	const uint16_t MAX_TORQUE = 10;
	const uint16_t MAX_MPH = 5;

	/* Get target torque. */
	pedal_data_t pedal_data = pedals_getData();
	uint16_t torque = _REVERSE(pedal_data.acceleration_percentage) * MAX_TORQUE;

	if(dti_get_mph() > MAX_MPH) {
		torque = 0;
	}

	dti_set_torque(-1*torque);
	return;
}

/* Handle torque when the car is in FAULTED. */
void drive_handleFaulted(void) {
	dti_set_torque(0); // No torque should be applied in FAULTED.
	return;
}

/* Handle torque when the car is in READY. */
void drive_handleReady(void) {
	dti_set_torque(0); // No torque should be applied in READY.
	return;
}
