#ifndef __U_QUEUES_H
#define __U_QUEUES_H

#include "tx_api.h"
#include "u_config.h"
#include "u_ethernet.h"
#include <stdint.h>

/*
*   Basically just a wrapper for ThreadX stuff. Lets you create/configure queues.
*   
*   Author: Blake Jackson
*/

/* Queue Config Macros */
#define QUEUE_WAIT_TIME     MS_TO_TICKS(100) // Wait 100ms for queue stuff before timing out

typedef struct {
    TX_QUEUE *queue;           /* Pointer to the queue */
    CHAR     *name;            /* Name of the queue */
    UINT      message_size;    /* Size of each message in the queue, in bytes. */
    UINT      capacity;        /* Maximum number of messages in the queue */
} queue_t;

/* Queue List */
extern TX_QUEUE eth_incoming; // Incoming Ethernet Queue
extern TX_QUEUE eth_outgoing; // Outgoing Ethernet Queue
// add more as necessary

/* API */
uint8_t queues_init(); // Initializes all queues. Called from app_threadx.c
uint8_t queue_send(TX_QUEUE *queue, void *message); // Sends a message to the specified queue.
uint8_t queue_receive(TX_QUEUE *queue, void *message); // Receives a message from the specified queue.

#endif /* u_queues.h */