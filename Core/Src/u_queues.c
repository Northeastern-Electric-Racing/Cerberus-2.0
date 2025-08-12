#include "u_queues.h"
#include "u_config.h"
#include <stdio.h>

/* 
*  NOTE: This file is kinda weird because of how ThreadX queues work. The size of each message in a ThreadX queue has to be a multiple of 4 bytes, since ThreadX
*  queues are implemented as arrays of 32-bit words. So, to create queues for structs that aren't a multiple of 4 bytes, you have to round up to the nearest multiple of 4 bytes.
*  This is handled in the _create_queue() function below.
*
*  Then, the queue_send() and queue_receive() functions use a buffer to send/receive messages to/from the queue.
*  This handles any conversions between the actual message size and the rounded-up size.
*
*  On the bright side (assuming my code works), this file should automatically handle all the 32-bit word stuff for you so you don't have to worry about it.
*
*/

/* Incoming Ethernet Queue */
queue_t eth_incoming;
static const QUEUE_CONFIG _eth_incoming_config = {
    .queue = &eth_incoming,                      /* Pointer to the queue. */
    .name = "Incoming Ethernet Queue",           /* Name of the queue. */
    .message_size = sizeof(ethernet_message_t),  /* Size of each queue message, in bytes. */
    .capacity = 10                               /* Number of messages the queue can hold. */
};

/* Outgoing Ethernet Queue */
queue_t eth_outgoing;
static const QUEUE_CONFIG _eth_outgoing_config = {
    .queue = &eth_outgoing,                      /* Pointer to the queue. */
    .name = "Outgoing Ethernet Queue",           /* Name of the queue. */
    .message_size = sizeof(ethernet_message_t),  /* Size of each queue message, in bytes. */
    .capacity = 10                               /* Number of messages the queue can hold. */
};

/* Helper function. Creates a ThreadX queue. */
static uint8_t _create_queue(TX_BYTE_POOL *byte_pool, const QUEUE_CONFIG *config) {
    uint8_t status;
    void *pointer;

    /* Calculate message size in 32-bit words (round up), and then validate it. */
    /* According to the Azure RTOS ThreadX Docs, "message sizes range from 1 32-bit word to 16 32-bit words". */
    /* Basically, queue messages have to be a multiple of 4 bytes? Kinda weird but this should handle it. */
    UINT message_size_words = (config->message_size + 3) / 4;
    if (message_size_words < 1 || message_size_words > 16) {
        DEBUG_PRINT("ERROR: Invalid message size %d bytes (must be 1-64 bytes). Queue: %s", 
                    config->message_size, config->name);
        return U_ERROR;
    }

    /* Store metadata */
    config->queue->bytes = config->message_size;
    config->queue->words = message_size_words;

    /* Calculate total queue size in bytes. */
    int queue_size_bytes = message_size_words * 4 * config->capacity;

    /* Allocate the stack for the queue. */
    status = tx_byte_allocate(byte_pool, (VOID**) &pointer, queue_size_bytes, TX_NO_WAIT);
    if(status != TX_SUCCESS) {
        DEBUG_PRINT("ERROR: Failed to allocate memory before creating queue (Status: %d, Queue: %s).", status, config->name);
        return U_ERROR;
    }

    /* Create the queue */
    status = tx_queue_create(&config->queue->_TX_QUEUE, config->name, message_size_words, pointer, queue_size_bytes);
    if (status != TX_SUCCESS) {
        DEBUG_PRINT("ERROR: Failed to create queue (Status: %d, Queue: %s).", status, config->name);
        tx_byte_release(pointer); // Free allocated memory if queue creation fails
        return U_ERROR;
    }

    return U_SUCCESS;

}

/* Initializes all ThreadX queues. 
*  Calls to _create_queue() should go in here
*/
uint8_t queues_init(TX_BYTE_POOL *byte_pool) {

    /* Create Incoming Ethernet Queue */
    uint8_t status = _create_queue(byte_pool, &_eth_incoming_config);
    if (status != U_SUCCESS) {
        DEBUG_PRINT("ERROR: Failed to create Incoming Ethernet Queue.");
        return status;
    }

    /* Create Outgoing Ethernet Queue */
    uint8_t status = _create_queue(byte_pool, &_eth_outgoing_config);
    if (status != U_SUCCESS) {
        DEBUG_PRINT("ERROR: Failed to create Outgoing Ethernet Queue.");
        return status;
    }

    DEBUG_PRINT("Ran queues_init().");
    return U_SUCCESS;
}

uint8_t queue_send(queue_t *queue, void *message) {
    UINT status;

    /* Create a buffer. */
    uint32_t buffer[queue->words];     // Max size is 16 words (64 bytes).
    memset(buffer, 0, sizeof(buffer)); // Initialize buffer to zero

    /* Copy message into the buffer. The buffer is what actually gets sent to the queue. */
    memcpy(buffer, message, queue->bytes);

    /* Send message (buffer) to the queue. */
    status = tx_queue_send(&queue->_TX_QUEUE, buffer, QUEUE_WAIT_TIME);
    if(status != TX_SUCCESS) {
        DEBUG_PRINT("ERROR: Failed to send message to queue (Status: %d, Queue: %s).", status, queue->_TX_QUEUE.tx_queue_name);
        return U_ERROR;
    }

    return U_SUCCESS;
}

uint8_t  queue_receive(queue_t *queue, void *message) {
    UINT status;

    /* Create a buffer */
    uint32_t buffer[queue->words];     // Max size is 16 words (64 bytes).
    memset(buffer, 0, sizeof(buffer)); // Initialize buffer to zero

    /* Receive message from the queue. */
    status = tx_queue_receive(&queue->_TX_QUEUE, buffer, QUEUE_WAIT_TIME);
    if(status != TX_SUCCESS) {
        DEBUG_PRINT("ERROR: Failed to receive message from queue (Status: %d, Queue: %s).", status, queue->_TX_QUEUE.tx_queue_name);
        return U_ERROR;
    }

    /* Copy the data from the buffer into the message pointer. Using memcpy() here prevents memory overflow. */
    memcpy(message, buffer, queue->bytes);

    return U_SUCCESS;
}