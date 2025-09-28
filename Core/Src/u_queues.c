#include "u_queues.h"
#include "u_can.h"
#include "u_tx_debug.h"
#include "u_faults.h"
#include "u_statemachine.h"
#include <stdio.h>

/* Incoming Ethernet Queue */
queue_t eth_incoming = {
    .name = "Incoming Ethernet Queue",          /* Name of the queue. */
    .message_size = sizeof(ethernet_message_t), /* Size of each queue message, in bytes. */
    .capacity = 10                              /* Number of messages the queue can hold. */
};

/* Outgoing Ethernet Queue */
queue_t eth_outgoing = {
    .name = "Outgoing Ethernet Queue",           /* Name of the queue. */
    .message_size = sizeof(ethernet_message_t),  /* Size of each queue message, in bytes. */
    .capacity = 10                               /* Number of messages the queue can hold. */
};

/* Incoming CAN Queue */
queue_t can_incoming = {
    .name = "Incoming CAN Queue",           /* Name of the queue. */
    .message_size = sizeof(can_msg_t),      /* Size of each queue message, in bytes. */
    .capacity = 10                          /* Number of messages the queue can hold. */
};

/* Outgoing CAN Queue */
queue_t can_outgoing = {
    .name = "Outgoing CAN Queue",          /* Name of the queue. */
    .message_size = sizeof(can_msg_t),     /* Size of each queue message, in bytes. */
    .capacity = 10                         /* Number of messages the queue can hold. */
};

/* Faults Queue */
queue_t faults = {
    .name = "Faults Queue",                /* Name of the queue. */
    .message_size = sizeof(fault_t),       /* Size of each queue message, in bytes. */
    .capacity = 10                         /* Number of messages the queue can hold. */
};

/* State Transition Queue */
queue_t state_transition_queue = {
    .name = "State Transition Queue",      /* Name of the queue. */
    .message_size = sizeof(state_req_t),   /* Size of each queue message, in bytes. */
    .capacity = 10                         /* Number of messages the queue can hold. */
};

/* Initializes all ThreadX queues. 
*  Calls to _create_queue() should go in here
*/
uint8_t queues_init(TX_BYTE_POOL *byte_pool) {

    /* Create Queues */
    CATCH_ERROR(create_queue(byte_pool, &eth_incoming), U_SUCCESS); // Create Incoming Ethernet Queue
    CATCH_ERROR(create_queue(byte_pool, &eth_outgoing), U_SUCCESS); // Create Outgoing Ethernet Queue
    CATCH_ERROR(create_queue(byte_pool, &can_incoming), U_SUCCESS); // Create Incoming CAN Queue
    CATCH_ERROR(create_queue(byte_pool, &can_outgoing), U_SUCCESS); // Create Outgoing CAN Queue
    CATCH_ERROR(create_queue(byte_pool, &faults), U_SUCCESS);       // Create Faults Queue
    CATCH_ERROR(create_queue(byte_pool, &state_transition_queue), U_SUCCESS); // Create state transition queue.

    DEBUG_PRINTLN("Ran queues_init().");
    return U_SUCCESS;
}