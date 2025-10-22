#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "u_drive.h"
#include "u_faults.h"
#include "u_pedals.h"
#include "u_queues.h"
#include "u_statemachine.h"

/**
 * @brief Determine if power to the motor controller should be disabled based on brake and accelerator pedal travel.
 * @param percentage_accel Percent travel of the accelerator pedal from 0-1
 * @param percentage_brake Brake pressure sensor reading, 0-1
 * @return bool True for prefault conditions met, false for no prefault.
 */
static bool _calc_bspd_prefault(float percentage_accel, float percentage_brake)
{
	static bool motor_disabled = false;

	/* EV.4.7: If brakes are engaged and APPS signals more than 25% pedal travel, disable power
	to the motor(s). Re-enable when accelerator has less than 5% pedal travel. */

	const float PEDAL_HARD_BRAKE_THRESH = 0.50;

	if (percentage_brake > PEDAL_HARD_BRAKE_THRESH && percentage_accel > 0.25) {
		motor_disabled = true;
		queue_send(&faults, &(fault_t){BSPD_PREFAULT});
	}

	/* Prevent a fault. */
	float dc_current = dti_get_dc_current();
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

/* TORQUE RESPONSE CURVE TEMPLATES. */
#define _LINEAR(x) ( 1*x ) 						   			// f(x) = x
#define _PROGRESSIVE(x) ( powf(x,2) )				   		// f(x) = x^2
#define _S_CURVE(x) ( (3*powf((x), 2)) - (2*powf((x), 3)) ) // f(x) = 3x^2 - 2x^3

/* Handle torque when the car is in PERFORMANCE. */
#define _PERFORMANCE(x) _LINEAR(x) /* Relationship between acceleration pedal percentage and torque percentage. */
void drive_handlePerformance(void) {
	/* CONFIG. */
	const uint16_t MAX_TORQUE = 220;
	const float ACCELERATION_THRESHOLD = 0.25; /* Pedal position above which acceleration begins. */

	/* Calculate BSPD Prefault. */
	pedal_data_t pedal_data = pedals_getData();
	if(_calc_bspd_prefault(pedal_data.acceleration_percentage, pedal_data.brake_percentage)) {
		/* Prefault triggered */
		// dti_set_torque(0);
		// osDelay(delay_time);
		// return;
	}
	
	/* If acceleration pedal percentage isn't above the threshold, don't accelerate. */
	if (pedal_data.acceleration_percentage < ACCELERATION_THRESHOLD) {
        dti_set_torque(0);
        return;
    }

	/* Calculate a scaled acceleration percentage from the threshold. */
    float scaled = (pedal_data.acceleration_percentage - ACCELERATION_THRESHOLD) / (1.0f - ACCELERATION_THRESHOLD);
	uint16_t torque = _PERFORMANCE(scaled) * MAX_TORQUE;

	/* Apply filters. */
	// u_TODO

	dti_set_torque(torque);
}

/* Handle torque when the car is in EFFICIENCY. */
#define _EFFICIENCY(x) _LINEAR(x) /* Relationship between acceleration pedal percentage and torque percentage. */
void drive_handleEfficiency(void) {
	/* CONFIG. */
	const uint16_t MAX_TORQUE = 220;
	const float ACCELERATION_THRESHOLD = 0.25; /* Pedal position above which acceleration begins. */

	/* Calculate BSPD Prefault. */
	pedal_data_t pedal_data = pedals_getData();
	if(_calc_bspd_prefault(pedal_data.acceleration_percentage, pedal_data.brake_percentage)) {
		/* Prefault triggered */
		// dti_set_torque(0);
		// osDelay(delay_time);
		// return;
	}
	
	/* If acceleration pedal percentage isn't above the threshold, don't accelerate. */
	if (pedal_data.acceleration_percentage < ACCELERATION_THRESHOLD) {
        dti_set_torque(0);
        return;
    }

	/* Calculate a scaled acceleration percentage from the threshold. */
    float scaled = (pedal_data.acceleration_percentage - ACCELERATION_THRESHOLD) / (1.0f - ACCELERATION_THRESHOLD);
	uint16_t torque = _EFFICIENCY(scaled) * MAX_TORQUE;

	/* Apply filters. */
	// u_TODO

	dti_set_torque(torque);
}

/* Handle torque when the car is in PIT. */
#define _PIT(x) _LINEAR(x) /* Relationship between acceleration pedal percentage and torque percentage. */
void drive_handlePit(void) {
	/* CONFIG. */
	const uint16_t MAX_TORQUE = 10;
	const float ACCELERATION_THRESHOLD = 0.25; /* Pedal position above which acceleration begins. */

	/* Calculate BSPD Prefault. */
	pedal_data_t pedal_data = pedals_getData();
	if(_calc_bspd_prefault(pedal_data.acceleration_percentage, pedal_data.brake_percentage)) {
		/* Prefault triggered */
		// dti_set_torque(0);
		// osDelay(delay_time);
		// return;
	}

	/* If acceleration pedal percentage isn't above the threshold, don't accelerate. */
	if (pedal_data.acceleration_percentage < ACCELERATION_THRESHOLD) {
        dti_set_torque(0);
        return;
    }

	/* Calculate a scaled acceleration percentage from the threshold. */
    float scaled = (pedal_data.acceleration_percentage - ACCELERATION_THRESHOLD) / (1.0f - ACCELERATION_THRESHOLD);
	uint16_t torque = _PIT(scaled) * MAX_TORQUE;

	/* Apply filters. */
	// u_TODO

	dti_set_torque(torque);
}

/* Handle torque when the car is in REVERSE. */
#define _REVERSE(x) _LINEAR(x) /* Relationship between acceleration pedal percentage and torque percentage. */
void drive_handleReverse(void) {
	/* CONFIG. */
	const uint16_t MAX_TORQUE = 10;
	const float ACCELERATION_THRESHOLD = 0.25; /* Pedal position above which acceleration begins. */

	/* Calculate BSPD Prefault. */
	pedal_data_t pedal_data = pedals_getData();
	if(_calc_bspd_prefault(pedal_data.acceleration_percentage, pedal_data.brake_percentage)) {
		/* Prefault triggered */
		// dti_set_torque(0);
		// osDelay(delay_time);
		// return;
	}

	/* If acceleration pedal percentage isn't above the threshold, don't accelerate. */
	if (pedal_data.acceleration_percentage < ACCELERATION_THRESHOLD) {
        dti_set_torque(0);
        return;
    }

	/* Calculate a scaled acceleration percentage from the threshold. */
    float scaled = (pedal_data.acceleration_percentage - ACCELERATION_THRESHOLD) / (1.0f - ACCELERATION_THRESHOLD);
	uint16_t torque = _REVERSE(scaled) * MAX_TORQUE;

	/* Apply filters. */
	// u_TODO

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