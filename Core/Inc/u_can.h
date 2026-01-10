#ifndef __U_CAN_H
#define __U_CAN_H

#include <stdint.h>
#include "fdcan.h"

uint8_t can1_init(FDCAN_HandleTypeDef *hcan);
void can_inbox(can_msg_t *message);

/* List of CAN interfaces */
extern can_t can1;
// add more as necessary

/* DTI CAN IDs */
#define DTI_CANID_ERPM	      0x416 /* ERPM, Duty, Input Voltage */
#define DTI_CANID_CURRENTS    0x436 /* AC Current, DC Current */
#define DTI_CANID_ERPM	      0x416 /* ERPM, Duty, Input Voltage */
#define DTI_CANID_CURRENTS    0x436 /* AC Current, DC Current */
#define DTI_CANID_TEMPS_FAULT 0x456 /* Controller Temp, Motor Temp, Faults */
#define DTI_CANID_ID_IQ	      0x476 /* Id, Iq values */
#define DTI_CANID_SIGNALS 	  0x496 /* Throttle signal, Brake signal, IO, Drive enable */

/* eFuse CAN IDs */
#define CANID_EFUSE_DASHBOARD 0xEF0
#define CANID_EFUSE_BRAKE     0xEF1
#define CANID_EFUSE_SHUTDOWN  0xEF2
#define CANID_EFUSE_LV        0xEF3
#define CANID_EFUSE_RADFAN    0xEF4
#define CANID_EFUSE_FANBATT   0xEF5
#define CANID_EFUSE_PUMP1     0xEF6
#define CANID_EFUSE_PUMP2     0xEF7
#define CANID_EFUSE_BATTBOX   0xEF8
#define CANID_EFUSE_MC        0xEF9

/* Misc CAN IDs */
#define CANID_FAULT_MSG	       0x502
#define CANID_SHUTDOWN_MSG	   0x123
#define CANID_PEDALS_VOLTS_MSG 0x504
#define CAN_ID_PEDALS_NORM_MSG 0x505
#define CANID_BMS_CELL_TEMPS   0x84
#define CANID_BMS_DCL_MSG      0x156
#define LIGHT_BOARD_CAN_MSG_ID 0xCA

/* Peripheral CAN IDs. */
#define CANID_IMU_ACCEL	       0x506
#define CANID_IMU_GYRO	       0x507
#define CANID_TEMP_SENSOR      0x508

#endif /* u_can.h */