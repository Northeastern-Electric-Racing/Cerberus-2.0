#ifndef __U_QUEUES_H
#define __U_QUEUES_H

#include "tx_api.h"
#include "u_config.h"
#include <stdint.h>
#include <stdio.h>

typedef struct {
    TX_QUEUE *queue;           /* Pointer to the queue */
    CHAR     *name;            /* Name of the queue */
    UINT      priority;        /* Priority of the queue */
    UINT      message_size;    /* Size of each message in the queue, in bytes. */
    UINT      max_messages;    /* Maximum number of messages in the queue */
    UCHAR    *memory;          /* Pointer to the queue's memory */
} queue_t;

/* Queue List */
extern TX_QUEUE queue_template;
// add more as necessary

uint8_t queues_init();

#endif /* u_threads.h */