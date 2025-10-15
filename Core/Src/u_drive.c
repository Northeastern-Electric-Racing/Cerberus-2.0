#include <stdint.h>
#include <stdbool.h>
#include "u_drive.h"
#include "u_faults.h"
#include "u_queues.h"

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

/* Globals. */
static float torque_limit_percentage = 1.0;
static uint16_t regen_limits[2] = { 0, 50 }; // [PERFORMANCE, ENDURANCE]
static bool launch_control_enabled = false;
static const float MPH_TO_KMH = 1.609;       // Factor for converting MPH to KMH

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

