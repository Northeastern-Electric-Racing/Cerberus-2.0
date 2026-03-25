#ifndef __U_TC_H
#define __U_TC_H

#include "fdcan.h"

/**
 * @brief Initializes the TC module. Loads the tire curve from flash and checks
 * that it is valid.
 *
 * @return int Success or error code (U_SUCCESS or U_ERROR)
 */
int tc_init(void);

/**
 * @brief Enables traction control.
 * 
 * @return int 
 */
int enable_tc(void);

/**
 * @brief Disables traction control.
 * 
 * @return int 
 */
int disable_tc(void);

/**
 * @brief Records front wheel speeds from a CAN message.
 * Expected format: bytes 0-1 = int16 FL RPM, bytes 2-3 = int16 FR RPM.
 */
void tc_record_front_rpm(can_msg_t msg);

/**
 * @brief Runs one iteration of the traction control algorithm.
 * Updates the internal torque scale factor based on current slip.
 */
void tc_process(void);

/**
 * @brief Returns the current TC torque scale factor in [0.0, 1.0].
 * Multiply the requested torque by this value before sending to the DTI.
 */
float tc_get_torque_scale(void);

#endif /* u_tc.h */
