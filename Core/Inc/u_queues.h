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
    TX_QUEUE _TX_QUEUE;      /* Queue instance. */
    size_t   bytes;           /* Size of each queue message, in bytes. */
    size_t   words;           /* Size of each queue message, in 32-bit words. */
} queue_t;

/* Queue List */
extern queue_t eth_incoming; // Incoming Ethernet Queue
extern queue_t eth_outgoing; // Outgoing Ethernet Queue
// add more as necessary

/* API */
uint8_t queues_init(); // Initializes all queues. Called from app_threadx.c
uint8_t queue_send(queue_t *queue, void *message); // Sends a message to the specified queue.
uint8_t queue_receive(queue_t *queue, void *message); // Receives a message from the specified queue.

/* Struct for configuring queues. */
typedef struct {
    queue_t  *queue;         /* Pointer to the queue instance */
    CHAR     *name;          /* Name of the queue */
    UINT      message_size;  /* Size of each message in the queue, in bytes. */
    UINT      capacity;      /* Maximum number of messages in the queue */
} QUEUE_CONFIG;

#endif /* u_queues.h */