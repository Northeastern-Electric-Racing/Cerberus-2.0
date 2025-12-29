/**
 * @file dti.h
 * @author Hamza Iqbal + Nick DePatie
 * @brief Driver to abstract sending and receiving CAN messages to control dti motor controller
 * @version 0.1
 * @date 2023-08-09
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef DTI_H
#define DTI_H

#include "fdcan.h"

/* Config. */
#define TIRE_DIAMETER 16 /* inches */
#define GEAR_RATIO    41 / 13.0 /* unitless */
#define POLE_PAIRS    10 /* unitless */

/**
 * @brief Initialize DTI interface.
 *
 * @return dti_t* Pointer to DTI struct
 */
void dti_init(void);

/**
 * @brief Get the RPM of the motor.
 *
 * @param dti Pointer to DTI struct
 * @return int32_t The RPM of the motor
 */
int32_t dti_get_rpm(void);

/**
 * @brief Process DTI ERPM CAN message.
 *
 * @param mc Pointer to struct representing motor controller
 * @param msg CAN message to process
 */
void dti_record_rpm(can_msg_t msg);

/**
 * @brief Get the MPH of the motor.
 *
 * @param dti Pointer to DTI struct
 * @return float
 */
float dti_get_mph(void);

/**
 * @brief Get the input voltage of the DTI.
 *
 * @param dti Pointer to DTI struct
 * @return uint16_t Input voltage of the DTI
 */
uint16_t dti_get_input_voltage(void);

/**
 * @brief Send CAN message to command torque from the motor controller. The torque to command is
 * smoothed with a moving average before being send to the motor controller.
 *
 * @param torque The torque target.
 */
void dti_set_torque(int16_t torque);

/**
 * @brief Set the brake AC current target for regenerative braking. Only positive values are
 * accepted by the DTI.
 *
 * @param current_target The desired AC current to do regenerative braking at. Must be positive.
 * This argument must be the actual value to set multiplied by 10.
 */
void dti_set_regen(uint16_t current_target);

/**
 * @brief Send a CAN message containing the AC current target for regenerative braking.
 *
 * @param brake_current AC current target for regenerative braking. The actual value sent to the
 * motor controller must be multiplied by 10.
 */
void dti_send_brake_current(uint16_t brake_current);

/**
 * @brief Send message for relative brake current target.
 *
 * @param relative_brake_current Percentage of brake current maximum multiplied by 10
 */
void dti_set_relative_brake_current(int16_t relative_brake_current);

/**
 * @brief Send AC current target command to DTI.
 *
 * @param current AC current target multiplied by 10
 */
void dti_set_current(int16_t current);

/**
 * @brief Send relative AC current target command to DTI.
 *
 * @param relative_current Percent of the maximum AC current multiplied by 10
 */
void dti_set_relative_current(int16_t relative_current);

/**
 * @brief Send drive enable command to DTI.
 *
 * @param drive_enable True to enable driving, false to disable
 */
void dti_set_drive_enable(bool drive_enable);

/**
 * @brief Record the controller and motor temperature in the DTI
 *
 * @param mc Pointer to DTI struct
 * @param msg CAN message containing temperature data
 */
void dti_record_temp(can_msg_t msg);

/**
 * @brief gets the current motor temperature from the DTI
 */
uint16_t dti_get_motor_temp(void);

/**
 * @brief gets the motor controller tempature
 */
uint16_t dti_get_controller_temp(void);

/**
 * @brief Record the currents from DTI
 *
 * @param mc Pointer to DTI struct
 * @param msg CAN message containing currents data
 */
void dti_record_currents(can_msg_t msg);

uint16_t dti_get_dc_current(void);

#endif
