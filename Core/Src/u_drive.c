#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "u_emrax.h"
#include "u_drive.h"
#include "u_faults.h"
#include "u_pedals.h"
#include "u_queues.h"
#include "u_statemachine.h"

/* Torque Curve Lookup Tables (don't even try to read). */
#define _LOOKUP_TABLE_SIZE 256
static const int16_t _lut_TestCurve[_LOOKUP_TABLE_SIZE] = {-20,-19,-18,-18,-17,-16,-15,-15,-14,-13,-13,-12,-11,-11,-10,-9,-9,-8,-8,-7,-7,-6,-6,-5,-5,-5,-4,-4,-3,-3,-3,-2,-2,-2,-2,-2,-1,-1,-1,-1,-1,-1,-1,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,2,2,2,2,3,3,3,4,4,5,5,6,6,7,7,8,9,9,10,11,12,12,13,14,15,16,17,18,19,20,20,22,23,24,25,26,27,28,29,30,31,33,34,35,36,38,39,40,41,43,44,45,47,48,50,51,52,54,55,57,58,60,61,63,64,66,67,69,70,72,73,75,77,78,80,81,83,84,86,88,89,91,93,94,96,97,99,101,102,104,106,107,109,111,112,114,116,117,119,121,122,124,125,127,129,130,132,134,135,137,138,140,142,143,145,146,148,149,151,152,154,156,157,159,160,161,163,164,166,167,169,170,172,173,174,176,177,178,180,181,182,183,185,186,187,188,190,191,192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,208,209,210,211,211,212,213,213,214,214,215,215,216,216,217,217,218,218,218,219,219,219,220,220};

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

/* Wrapper for calling dti_set_torque() and dti_set_regen().
*  Positive torque values are passed directly into dti_set_torque(). Negative values are converted into a regen current, and passed into dti_set_regen().
*  This means that negative inputs will be converted to regen braking. So, do not use this function if you want REVERSE torque.
*/
static void _set_torque(int16_t torque) {
	if(torque >= 0) {
		/* Positive torque, so car should accelerate. */
		dti_set_torque(torque);
	}
	else {
		/* Negative torque, so car should engage regen braking. */
		// u_TODO - possibly scale down the regen torque based on current speed
		dti_set_regen((uint16_t)((torque / EMRAX_KT) * 10));		// u_TODO - check if EMRAX_KT is the correct torque->current constant for regen/brake current.
	}
}

/* Wrapper for calling dti_set_torque() when the car should be reversing. */
static void _set_reverse_torque(uint16_t torque) {
	dti_set_torque(-1*torque);
}

/* Handle torque when the car is in PERFORMANCE. */
void drive_handlePerformance(void) {
	/* Calculate BSPD Prefault. */
	pedal_data_t pedal_data = pedals_getData();
	if(_calc_bspd_prefault(pedal_data.acceleration_percentage, pedal_data.brake_percentage)) {
		/* Prefault triggered */
		// dti_set_torque(0);
		// osDelay(delay_time);
		// return;
	}
}

/* Handle torque/regen when the car is in EFFICIENCY. */
void drive_handleEfficiency(void) {
	/* Calculate BSPD Prefault. */
	pedal_data_t pedal_data = pedals_getData();
	if(_calc_bspd_prefault(pedal_data.acceleration_percentage, pedal_data.brake_percentage)) {
		/* Prefault triggered */
		// dti_set_torque(0);
		// osDelay(delay_time);
		// return;
	}

	/* Look up torque value and set torque. */
	float percentage = fmaxf(0.0f, fminf(1.0f, pedal_data.acceleration_percentage));
	int index = percentage * (_LOOKUP_TABLE_SIZE - 1) + 0.5f;
	int16_t torque = _lut_TestCurve[index];
	_set_torque(torque);
}

/* Handle torque/regen when the car is in PIT. */
void drive_handlePit(void) {
	/* Calculate BSPD Prefault. */
	pedal_data_t pedal_data = pedals_getData();
	if(_calc_bspd_prefault(pedal_data.acceleration_percentage, pedal_data.brake_percentage)) {
		/* Prefault triggered */
		// dti_set_torque(0);
		// osDelay(delay_time);
		// return;
	}
}

/* Handle torque when the car is in REVERSE. */
void drive_handleReverse(void) {
	/* Calculate BSPD Prefault. */
	pedal_data_t pedal_data = pedals_getData();
	if(_calc_bspd_prefault(pedal_data.acceleration_percentage, pedal_data.brake_percentage)) {
		/* Prefault triggered */
		// dti_set_torque(0);
		// osDelay(delay_time);
		// return;
	}
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
