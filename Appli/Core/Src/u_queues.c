#include "u_queues.h"

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

/* Template Queue */
TX_QUEUE queue_template;
static UCHAR _template_memory[1 * 10];    /* (message_size * capacity) */
static const queue_t _template_config = {
    .queue         = &queue_template,     /* Pointer to the queue. */
    .name          = "Template Queue",    /* Name of the queue. */
    .priority      = 0,                   /* Priority of the queue. */
    .message_size  = 1,                   /* Size of each queue message, in bytes. */
    .capacity      = 10,                  /* Number of messages the queue can hold. */
    .memory        = _template_memory     /* Pointer to the queue's memory. */
};

/* Initializes all ThreadX queues. 
*  Calls to _create_queue() should go in here
*/
uint8_t queues_init() {

    /* Create Queues */
    _create_queue(&_template_config); // Create Template queue.
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