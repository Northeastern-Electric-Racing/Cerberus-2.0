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
// add more as necessary

#endif /* u_can.h */