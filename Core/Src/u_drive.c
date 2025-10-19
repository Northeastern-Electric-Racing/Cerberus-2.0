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
static const int16_t _lookup_EfficiencyCurve[] = {-20,-19,-18,-18,-17,-16,-15,-14,-14,-13,-12,-12,-11,-10,-10,-9,-8,-8,-7,-7,-6,-6,-5,-5,-4,-4,-3,-3,-3,-2,-2,-2,-2,-1,-1,-1,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,3,3,4,4,5,5,6,6,7,8,8,9,10,10,11,12,13,14,14,15,16,17,18,19,20,21,22,23,24,25,26,27,29,30,31,32,33,34,36,37,38,40,41,42,44,45,46,48,49,50,52,53,55,56,58,59,60,62,63,65,66,68,70,71,73,74,76,77,79,81,82,84,85,87,89,90,92,93,95,97,98,100,102,103,105,106,108,110,111,113,115,116,118,120,121,123,124,126,128,129,131,133,134,136,137,139,141,142,144,145,147,148,150,151,153,154,156,157,159,160,162,163,165,166,168,169,170,172,173,175,176,177,179,180,181,182,184,185,186,187,189,190,191,192,193,194,195,196,197,198,200,200,201,202,203,204,205,206,207,208,208,209,210,211,211,212,213,213,214,214,215,215,216,216,217,217,218,218,218,219,219,219,220,220};
static const int16_t _lookup_PerformanceCurve[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,11,11,12,12,13,14,14,15,16,16,17,18,19,20,21,21,22,23,24,25,26,27,28,29,30,31,32,33,35,36,37,38,39,40,42,43,44,45,47,48,49,50,52,53,54,56,57,58,60,61,63,64,65,67,68,70,71,72,74,75,77,78,80,81,83,84,86,87,89,90,92,93,95,96,98,99,101,102,104,105,106,108,109,111,112,114,115,117,118,120,121,123,124,126,127,128,130,131,133,134,135,137,138,140,141,142,144,145,146,148,149,150,151,153,154,155,156,158,159,160,161,162,163,164,166,167,168,169,170,171,172,173,174,175,176,176,177,178,179,180,181,181,182,183,184,184,185,186,187,188,188,189,190,190,191,192,193,193,194,195,195,196,197,197,198,199,199,200,201,201,202,203,203,204,205,205,206,206,207,208,208,209,209,210,210,211,212,212,213,213,214,214,215,215,216,216,217,217,218,218,219,219,220,220};
static const int16_t _lookup_PitCurve[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,5,5,5,6,6,7,7,7,8,8,9,9,9,10,10,11,11,12,12,13,13,14,14,15,15,16,16,17,18,18,19,19,20,20,21,22,22,23,23,24,25,25,26,26,27,28,28,29,30,30,31,32,32,33,33,34,35,35,36,37,37,38,39,39,40,41,41,42,43,43,44,44,45,46,46,47,48,48,49,50,50,51,51,52,53,53,54,54,55,56,56,57,57,58,58,59,59,60,61,61,62,62,63,63,64,64,65,65,66,66,66,67,67,68,68,69,69,69,70,70,70,71,71,71,72,72,72,73,73,73,73,74,74,74,74,74,75,75,75,75,75,75,75,75,76,76,76,76,76,76,76,76,77,77,77,77,77,77,77,77,77,77,78,78,78,78,78,78,78,78,78,78,79,79,79,79,79,79,79,79,79,79,79,79,79,79,80,80,80,80,80,80,80,80,80,80,80,80,80,80,80,80,80,80};
static const int16_t _lookup_ReverseCurve[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,2,2,2,2,3,3,3,4,4,4,5,5,6,6,7,7,7,8,8,9,9,10,10,11,12,12,13,13,14,15,15,16,16,17,18,18,19,20,20,21,22,23,23,24,25,25,26,27,28,29,29,30,31,32,32,33,34,35,36,36,37,38,39,40,40,41,42,43,44,45,45,46,47,48,49,50,50,51,52,53,54,55,55,56,57,58,59,59,60,61,62,63,63,64,65,66,67,67,68,69,70,70,71,72,73,73,74,75,75,76,77,77,78,79,79,80,81,81,82,82,83,84,84,85,85,86,86,87,87,88,88,89,89,90,90,90,91,91,92,92,92,93,93,93,93,94,94,94,94,95,95,95,95,95,95,95,95,96,96,96,96,96,96,96,96,97,97,97,97,97,97,97,97,97,98,98,98,98,98,98,98,98,98,98,98,99,99,99,99,99,99,99,99,99,99,99,99,99,99,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100};

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
		dti_set_regen((uint16_t)((-torque / EMRAX_KT) * 10));		// u_TODO - check if EMRAX_KT is the correct torque->current constant for regen/brake current.
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

	/* Look up torque value and set torque. */
	float percentage = fmaxf(0.0f, fminf(1.0f, pedal_data.acceleration_percentage));
	int index = percentage * (_LOOKUP_TABLE_SIZE - 1) + 0.5f;
	int16_t torque = _lookup_PerformanceCurve[index];
	_set_torque(torque);
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
	int16_t torque = _lookup_EfficiencyCurve[index];
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

	/* Look up torque value and set torque. */
	float percentage = fmaxf(0.0f, fminf(1.0f, pedal_data.acceleration_percentage));
	int index = percentage * (_LOOKUP_TABLE_SIZE - 1) + 0.5f;
	int16_t torque = _lookup_PitCurve[index];
	// u_TODO - probably add a torque derating function based on mph: _derate_torque(&torque, MAX_MPH); or something. Look at the one currently in Cerberus
	_set_torque(torque);
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

	/* Look up torque value and set torque. */
	float percentage = fmaxf(0.0f, fminf(1.0f, pedal_data.acceleration_percentage));
	int index = percentage * (_LOOKUP_TABLE_SIZE - 1) + 0.5f;
	int16_t torque = _lookup_ReverseCurve[index];
	// u_TODO - probably add a torque derating function based on mph: _derate_torque(&torque, MAX_MPH); or something. Look at the one currently in Cerberus
	_set_reverse_torque(torque);
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
