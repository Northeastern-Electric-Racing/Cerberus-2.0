#include "u_queues.h"
#include "u_config.h"
#include <stdio.h>

/* Incoming Ethernet Queue */
TX_QUEUE eth_incoming;
static const queue_t _eth_incoming_config = {
    .queue         = &eth_incoming,              /* Pointer to the queue. */
    .name          = "Incoming Ethernet Queue",  /* Name of the queue. */
    .message_size  = sizeof(ethernet_message_t), /* Size of each queue message, in bytes. */
    .capacity      = 10,                         /* Number of messages the queue can hold. */
};

/* Outgoing Ethernet Queue */
TX_QUEUE eth_outgoing;
static const queue_t _eth_outgoing_config = {
    .queue         = &eth_outgoing,              /* Pointer to the queue. */
    .name          = "Outgoing Ethernet Queue",  /* Name of the queue. */
    .message_size  = sizeof(ethernet_message_t), /* Size of each queue message, in bytes. */
    .capacity      = 10,                         /* Number of messages the queue can hold. */
};

/* Helper function. Creates a ThreadX queue. */
static uint8_t _create_queue(TX_BYTE_POOL *byte_pool, const queue_t *queue) {
    UINT status;
    VOID *pointer;

    /* Calculate message size in 32-bit words (round up), and then validate it. */
    /* According to the Azure RTOS ThreadX Docs, "message sizes range from 1 32-bit word to 16 32-bit words". */
    /* Basically, queue messages have to be a multiple of 4 bytes? Kinda weird but this should handle it. */
    UINT message_size_words = (queue->message_size + 3) / 4;
    if (message_size_words < 1 || message_size_words > 16) {
        DEBUG_PRINT("ERROR: Invalid message size %d bytes (must be 1-64 bytes). Queue: %s", 
                    queue->message_size, queue->name);
        return U_ERROR;
    }

    /* Calculate total queue size in bytes (ThreadX needs the full size, even though individual messages are in 32-bit words) */
    ULONG queue_size_bytes = message_size_words * 4 * queue->capacity;

    /* Allocate the stack for the queue. */
    status = tx_byte_allocate(byte_pool, (VOID**) &pointer, queue_size_bytes, TX_NO_WAIT);
    if(status != TX_SUCCESS) {
        DEBUG_PRINT("ERROR: Failed to allocate stack before creating queue (Status: %d, Queue: %s).", status, queue->name);
        return U_ERROR;
    }

    /* Create the queue */
    status = tx_queue_create(queue->queue, queue->name, message_size_words, pointer, queue_size_bytes);
    if (status != TX_SUCCESS) {
        DEBUG_PRINT("ERROR: Failed to create queue (Status: %d, Queue: %s).", status, queue->name);
        tx_byte_release(pointer); // Free allocated memory if queue creation fails
        return U_ERROR;
    }

    return U_SUCCESS;

}

/* Initializes all ThreadX queues. 
*  Calls to _create_queue() should go in here
*/
uint8_t queues_init(TX_BYTE_POOL *byte_pool) {

    /* Create Queues */
    CATCH_ERROR(_create_queue(byte_pool, &_eth_incoming_config), U_SUCCESS); // Create Incoming Ethernet Queue.
    CATCH_ERROR(_create_queue(byte_pool, &_eth_outgoing_config), U_SUCCESS); // Create Outgoing Ethernet Queue.
    // add more queues here if need eventually

    DEBUG_PRINT("Ran queues_init().");
    return U_SUCCESS;
}

uint8_t queue_send(TX_QUEUE *queue, void *message) {
    UINT status;

    /* Send message to the queue. */
    status = tx_queue_send(queue, message, QUEUE_WAIT_TIME);
    if(status != TX_SUCCESS) {
        DEBUG_PRINT("ERROR: Failed to send message to queue (Status: %d, Queue: %s).", status, queue->tx_queue_name);
        return U_ERROR;
    }

    return U_SUCCESS;
}

uint8_t  queue_receive(TX_QUEUE *queue, void *message) {
    UINT status;

    /* Receive message from the queue. */
    status = tx_queue_receive(queue, message, QUEUE_WAIT_TIME);
    if(status != TX_SUCCESS) {
        DEBUG_PRINT("ERROR: Failed to receive message from queue (Status: %d, Queue: %s).", status, queue->tx_queue_name);
        return U_ERROR;
    }

    return U_SUCCESS;
}