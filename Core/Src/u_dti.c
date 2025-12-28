/**
 * @file dti.c
 * @author Hamza Iqbal and Nick DePatie
 * @brief Source file for Motor Controller Driver
 * @version 0.1
 * @date 2023-08-22
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "u_dti.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "c_utils.h"
#include "u_bms.h"
#include "u_can.h"
#include "u_emrax.h"
#include "u_queues.h"
#include "u_mutexes.h"

#define CAN_QUEUE_SIZE 5 /* messages */
#define SAMPLES \
	3 /* determines number of torque request samples to average for dti*/

static dti_t mc;

void dti_init(void)
{
	mc.rpm = 0;
	mc.contr_temp = 0;
	mc.motor_temp = 0;
	mc.rpm = 0;

	PRINTLN_INFO("Ran dti_init().");
}

void dti_set_torque(int16_t torque)
{
	/* We can't change motor speed super fast else we blow diff, therefore low pass filter */
	// Static variables for the buffer and index
	static float buffer[SAMPLES] = { 0 };

	/* To disable current ramp up to dti uncomment sections
	with samples_seen and use samples_seen in the sum and average calcs*/
	//static float samples_seen = 0;

	static int index = 0;

	// Add the new value to the buffer
	buffer[index] = torque;

	// Increment the index, wrapping around if necessary
	index = (index + 1) % SAMPLES;
	/*
	samples_seen += 1;
	if (samples_seen > SAMPLES) {
		samples_seen = SAMPLES;
	}
	*/

	// Calculate the average of the buffer
	float sum = 0.0;
	for (int i = 0; i < SAMPLES; ++i) {
		sum += buffer[i];
	}
	float average = sum / SAMPLES;

	if (torque == 0) {
		average = 0;
	}

	/* Motor controller expects AC current target to be received as multiplied by 10 */
	int16_t ac_current = (((float)average / EMRAX_KT) * 10);

	dti_set_current(ac_current);
}

void dti_set_regen(uint16_t current_target)
{
	/* Simple moving average to smooth change in braking target */

	// Static variables for the buffer and index
	static uint16_t buffer[SAMPLES] = { 0 };
	static int index = 0;

	// Add the new value to the buffer
	buffer[index] = current_target;

	// Increment the index, wrapping around if necessary
	index = (index + 1) % SAMPLES;

	// Calculate the average of the buffer
	uint16_t sum = 0;
	for (int i = 0; i < SAMPLES; ++i) {
		sum += buffer[i];
	}
	uint16_t average = sum / SAMPLES;

	dti_send_brake_current(average);
}

void dti_set_current(int16_t current)
{
	can_msg_t msg = { .id = 0x036, .len = 2, .data = { 0 } };

#ifdef TSMS_OVERRIDE
	dti_set_drive_enable(false);
#else
	dti_set_drive_enable(true);
#endif

	/* Send CAN message in big endian format */

	// endian_swap(&current, sizeof(current));
	// memcpy(msg.data, &current, msg.len);
	int8_t msb = (int8_t)((current >> 8) & 0xFF);
	int8_t lsb = (uint8_t)(current & 0xFF);

	msg.data[0] = msb;
	msg.data[1] = lsb;

	queue_send(&can_outgoing, &msg, TX_NO_WAIT);
}

void dti_send_brake_current(uint16_t brake_current)
{
	can_msg_t msg = { .id = 0x056,
			  .len = 8,
			  .data = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 } };

	/* convert to big endian */
	endian_swap(&brake_current, sizeof(brake_current));

	/* Send CAN message */
	memcpy(&msg.data, &brake_current, 2);
	queue_send(&can_outgoing, &msg, TX_NO_WAIT);
}

void dti_set_speed(int32_t rpm)
{
	can_msg_t msg = { .id = 0x076, .len = 4, .data = { 0 } };

	rpm = rpm * EMRAX_NUM_POLE_PAIRS;

	/* convert to big endian */
	endian_swap(&rpm, sizeof(rpm));

	/* Send CAN message */
	memcpy(msg.data, &rpm, msg.len);
	queue_send(&can_outgoing, &msg, TX_NO_WAIT);
}

void dti_set_position(int16_t angle)
{
	can_msg_t msg = { .id = 0x096, .len = 2, .data = { 0 } };

	/* convert to big endian */
	endian_swap(&angle, sizeof(angle));

	/* Send CAN message */
	memcpy(msg.data, &angle, msg.len);
	queue_send(&can_outgoing, &msg, TX_NO_WAIT);
}

void dti_set_relative_current(int16_t relative_current)
{
	can_msg_t msg = { .id = 0x0B6, .len = 2, .data = { 0 } };

	/* convert to big endian */
	endian_swap(&relative_current, sizeof(relative_current));

	/* Send CAN message */
	memcpy(msg.data, &relative_current, msg.len);
	queue_send(&can_outgoing, &msg, TX_NO_WAIT);
}

void dti_set_relative_brake_current(int16_t relative_brake_current)
{
	can_msg_t msg = { .id = 0x0D6, .len = 2, .data = { 0 } };

	/* convert to big endian */
	endian_swap(&relative_brake_current, sizeof(relative_brake_current));

	/* Send CAN message */
	memcpy(msg.data, &relative_brake_current, msg.len);
	queue_send(&can_outgoing, &msg, TX_NO_WAIT);
}

void dti_set_digital_output(uint8_t output, bool value)
{
	can_msg_t msg = { .id = 0x0F6, .len = 1, .data = { 0 } };

	uint8_t ctrl = value >> output;

	/* Send CAN message */
	memcpy(msg.data, &ctrl, msg.len);
	queue_send(&can_outgoing, &msg, TX_NO_WAIT);
}

void dti_set_max_ac_current(int16_t current)
{
	can_msg_t msg = { .id = 0x116, .len = 2, .data = { 0 } };

	/* convert to big endian */
	endian_swap(&current, sizeof(current));

	/* Send CAN message */
	memcpy(msg.data, &current, msg.len);
	queue_send(&can_outgoing, &msg, TX_NO_WAIT);
}

void dti_set_max_ac_brake_current(int16_t current)
{
	can_msg_t msg = { .id = 0x136, .len = 2, .data = { 0 } };

	/* convert to big endian */
	endian_swap(&current, sizeof(current));

	/* Send CAN message */
	memcpy(msg.data, &current, msg.len);
	queue_send(&can_outgoing, &msg, TX_NO_WAIT);
}

void dti_set_max_dc_current(int16_t current)
{
	can_msg_t msg = { .id = 0x156, .len = 2, .data = { 0 } };

	/* convert to big endian */
	endian_swap(&current, sizeof(current));

	/* Send CAN message */
	memcpy(msg.data, &current, msg.len);
	queue_send(&can_outgoing, &msg, TX_NO_WAIT);
}

void dti_set_max_dc_brake_current(int16_t current)
{
	can_msg_t msg = { .id = 0x176, .len = 2, .data = { 0 } };

	/* convert to big endian */
	endian_swap(&current, sizeof(current));

	/* Send CAN message */
	memcpy(msg.data, &current, msg.len);
	queue_send(&can_outgoing, &msg, TX_NO_WAIT);
}

void dti_set_drive_enable(bool drive_enable)
{
	can_msg_t msg = { .id = 0x196, .len = 1, .data = { 0 } };

	/* Send CAN message */
	memcpy(msg.data, &drive_enable, msg.len);
	queue_send(&can_outgoing, &msg, TX_NO_WAIT);
}

int dti_get_rpm(int32_t* buffer)
{
	CATCH_ERROR(mutex_get(&dti_mutex), U_SUCCESS);
	*buffer = mc.rpm;
	CATCH_ERROR(mutex_put(&dti_mutex), U_SUCCESS);
	return U_SUCCESS;
}

int dti_get_mph(float* buffer)
{
	/* Get RPM. */
	int32_t rpm;
	CATCH_ERROR(dti_get_rpm(&rpm), U_SUCCESS);

	/* Convert RPM to MPH */
	// rpm * gear ratio = wheel rpm
	// tire diamter (in) to miles --> tire diamter miles
	// wheel rpm * 60 --> wheel rph
	// tire diamter miles * pi --> tire circumference
	// rph * wheel circumference miles --> mph
	*buffer = (rpm / (GEAR_RATIO)) * 60 *
	       (TIRE_DIAMETER / 63360.0) * M_PI;
	
	return U_SUCCESS;
}

int dti_record_rpm(can_msg_t msg)
{
	/* ERPM is first four bytes of can message in big endian format */
	int32_t erpm = (msg.data[0] << 24) + (msg.data[1] << 16) +
		       (msg.data[2] << 8) + (msg.data[3]);

	int32_t rpm = erpm / POLE_PAIRS;

	CATCH_ERROR(mutex_get(&dti_mutex), U_SUCCESS);
	mc.rpm = rpm;
	CATCH_ERROR(mutex_put(&dti_mutex), U_SUCCESS);

	return U_SUCCESS;
}

int dti_record_temp(can_msg_t msg)
{
	uint16_t controllerTemp = (msg.data[0] << 8) + (msg.data[1]);
	uint16_t motorTemp = (msg.data[2] << 8) + (msg.data[3]);

	controllerTemp /= 10;
	motorTemp /= 10;

	CATCH_ERROR(mutex_get(&dti_mutex), U_SUCCESS);
	mc.contr_temp = controllerTemp;
	mc.motor_temp = motorTemp;
	CATCH_ERROR(mutex_put(&dti_mutex), U_SUCCESS);
	return U_SUCCESS;
}

int dti_get_motor_temp(int16_t* buffer)
{
	CATCH_ERROR(mutex_get(&dti_mutex), U_SUCCESS);
	*buffer = mc.motor_temp;
	CATCH_ERROR(mutex_put(&dti_mutex), U_SUCCESS);
	return U_SUCCESS;
}

int dti_get_controller_temp(int16_t* buffer)
{
	CATCH_ERROR(mutex_get(&dti_mutex), U_SUCCESS);
	*buffer = mc.contr_temp;
	CATCH_ERROR(mutex_put(&dti_mutex), U_SUCCESS);
	return U_SUCCESS;
}

int dti_record_currents(can_msg_t msg)
{	
	CATCH_ERROR(mutex_get(&dti_mutex), U_SUCCESS);

	int16_t ac_current = (msg.data[0] << 8) + (msg.data[1]) / 10;
	int16_t dc_current = (msg.data[2] << 8) + (msg.data[3]) / 10;

	mc.ac_current = ac_current;
	mc.dc_current = dc_current;

	CATCH_ERROR(mutex_put(&dti_mutex), U_SUCCESS);
	return U_SUCCESS;
}

int dti_get_dc_current(uint16_t* buffer)
{
	CATCH_ERROR(mutex_get(&dti_mutex), U_SUCCESS);
	*buffer = mc.dc_current;
	CATCH_ERROR(mutex_put(&dti_mutex), U_SUCCESS);
	return U_SUCCESS;
}
