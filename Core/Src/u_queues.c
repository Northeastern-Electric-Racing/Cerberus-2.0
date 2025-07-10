#include "u_queues.h"
#include <stdio.h>

/* Helper function. Creates a ThreadX queue. */
static uint8_t _create_queue(const queue_t *queue) {
    UINT status;

    /* Create the queue. */
    status = tx_queue_create(queue->queue, queue->name, queue->message_size, queue->memory, queue->message_size * queue->capacity);
    if(status != TX_SUCCESS) {
        printf("[u_queues.c/_create_queue()] ERROR: Failed to create queue (Status: %d, Queue: %s).\n", status, queue->name);
        return U_ERROR;
    }

    return U_SUCCESS;

}

/* Incoming Ethernet Queue */
TX_QUEUE eth_incoming;
static UCHAR _eth_incoming_memory[sizeof(ethernet_message_t) * 10]; /* (message_size * capacity) */
static const queue_t _eth_incoming_config = {
    .queue         = &eth_incoming,              /* Pointer to the queue. */
    .name          = "Incoming Ethernet Queue",  /* Name of the queue. */
    .message_size  = sizeof(ethernet_message_t), /* Size of each queue message, in bytes. */
    .capacity      = 10,                         /* Number of messages the queue can hold. */
    .memory        = _eth_incoming_memory        /* Pointer to the queue's memory. */
};

/* Outgoing Ethernet Queue */
TX_QUEUE eth_outgoing;
static UCHAR _eth_outgoing_memory[sizeof(ethernet_message_t) * 10]; /* (message_size * capacity) */
static const queue_t _eth_outgoing_config = {
    .queue         = &eth_outgoing,              /* Pointer to the queue. */
    .name          = "Outgoing Ethernet Queue",  /* Name of the queue. */
    .message_size  = sizeof(ethernet_message_t), /* Size of each queue message, in bytes. */
    .capacity      = 10,                         /* Number of messages the queue can hold. */
    .memory        = _eth_outgoing_memory        /* Pointer to the queue's memory. */
};

/* Initializes all ThreadX queues. 
*  Calls to _create_queue() should go in here
*/
uint8_t queues_init() {

    /* Create Queues */
    _create_queue(&_eth_incoming_config); // Create Incoming Ethernet Queue.
    _create_queue(&_eth_outgoing_config); // Create Outgoing Ethernet Queue.
    // add more queues here if need eventually

    printf("[u_queues.c/queues_init()] Ran queues_init().\n");
    return U_SUCCESS;
}

uint8_t queue_send(TX_QUEUE *queue, void *message) {
    UINT status;

    /* Send message to the queue. */
    status = tx_queue_send(queue, message, TX_NO_WAIT);
    if(status != TX_SUCCESS) {
        printf("[u_queues.c/queues_send()] ERROR: Failed to send message to queue (Status: %d, Queue: %s).\n", status, queue->tx_queue_name);
        return U_ERROR;
    }

    return U_SUCCESS;
}

uint8_t  queue_receive(TX_QUEUE *queue, void *message) {
    UINT status;

    /* Receive message from the queue. */
    status = tx_queue_receive(queue, message, TX_NO_WAIT);
    if(status != TX_SUCCESS) {
        printf("[u_queues.c/queue_receive()] ERROR: Failed to receive message from queue (Status: %d, Queue: %s).\n", status, queue->tx_queue_name);
        return U_ERROR;
    }

    return U_SUCCESS;
}