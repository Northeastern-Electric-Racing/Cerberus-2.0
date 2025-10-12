#ifndef __U_QUEUES_H
#define __U_QUEUES_H

#include "tx_api.h"
#include "u_tx_queues.h"
#include "u_nx_ethernet.h"
#include <stdint.h>

/*
*   Basically just a wrapper for ThreadX stuff. Lets you create/configure queues.
*   
*   Author: Blake Jackson
*/

/* Queue Config Macros */
#define QUEUE_WAIT_TIME TX_NO_WAIT // Wait time for queue stuff before timing out

/* Queue List */
extern queue_t eth_incoming; // Incoming Ethernet Queue
extern queue_t eth_outgoing; // Outgoing Ethernet Queue
extern queue_t can_incoming; // Incoming CAN Queue
extern queue_t can_outgoing; // Outgoing CAN Queue
extern queue_t faults;       // Faults Queue
extern queue_t state_transition_queue; // State Transition Queue
// add more as necessary

/* API */
uint8_t queues_init(TX_BYTE_POOL *byte_pool); // Initializes all queues. Called from app_threadx.c

#endif /* u_queues.h */