#ifndef __U_CAN_H
#define __U_CAN_H

#include <stdint.h>
#include "fdcan.h"

uint8_t can1_init(FDCAN_HandleTypeDef *hcan);

/* List of CAN interfaces */
extern can_t can1;
// add more as necessary

/* List of CAN IDs */
#define CANID_FAULT_MSG	       0x502
#define CANID_SHUTDOWN_MSG	   0x123
#define CANID_PEDALS_VOLTS_MSG 0x504
#define CAN_ID_PEDALS_NORM_MSG 0x505
// add more as necessary

#endif /* u_can.h */