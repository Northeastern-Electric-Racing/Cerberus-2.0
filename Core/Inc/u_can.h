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
#define CANID_EFUSE_LV	      0xEF3
#define CANID_EFUSE_RADFAN    0xEF4
#define CANID_EFUSE_FANBATT   0xEF5
#define CANID_EFUSE_PUMP1     0xEF6
#define CANID_EFUSE_PUMP2     0xEF7
#define CANID_EFUSE_BATTBOX   0xEF8
#define CANID_EFUSE_MC	      0xEF9

/* Calypso eFuse Control CAN IDs */
#define CANID_CALYPSO_EFCTRL_DASHBOARD 0xCAEF0
#define CANID_CALYPSO_EFCTRL_BRAKE     0xCAEF1
#define CANID_CALYPSO_EFCTRL_SHUTDOWN  0xCAEF2
#define CANID_CALYPSO_EFCTRL_LV        0xCAEF3
#define CANID_CALYPSO_EFCTRL_RADFAN    0xCAEF4
#define CANID_CALYPSO_EFCTRL_FANBATT   0xCAEF5
#define CANID_CALYPSO_EFCTRL_PUMPONE   0xCAEF6
#define CANID_CALYPSO_EFCTRL_PUMPTWO   0xCAEF7
#define CANID_CALYPSO_EFCTRL_BATTBOX   0xCAEF8
#define CANID_CALYPSO_EFCTRL_MC        0xCAEF9
#define CANID_CALYPSO_EFCTRL_SPARE     0xCAEFA

/* Calypso RTDS State Command CAN IDs */
#define CANID_CALYPSO_RTDS_STATE 0xDB1

/* Misc CAN IDs */
#define CANID_FAULT_MSG	       0x502
#define CANID_SHUTDOWN_MSG     0x123
#define CANID_PEDALS_VOLTS_MSG 0x504
#define CAN_ID_PEDALS_NORM_MSG 0x505
#define CANID_BMS_CELL_TEMPS   0x84
#define CANID_BMS_DCL_MSG      0x156
#define LIGHT_BOARD_CAN_MSG_ID 0xCA
#define IMU_CAN_MSG_ID	       0xAAB
#define CANID_VCU_TEST_MESSAGE 0xBAD
#define CANID_SHEPHERD_PRECHARGE 0x601
#define CANID_WHEEL_BUTTONS    0x680

/* Peripheral CAN IDs. */
#define CANID_IMU_ACCEL	  0x506
#define CANID_IMU_GYRO	  0x507
#define CANID_TEMP_SENSOR 0x508
#define CANID_F_RPM       0xDB0
#define CANID_R_RPM       0xDB1

#endif /* u_can.h */
