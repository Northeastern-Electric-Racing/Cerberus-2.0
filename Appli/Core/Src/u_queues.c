#include "u_queues.h"

/* Helper function. Creates a ThreadX queue. */
static uint8_t _create_queue(const queue_t *queue) {
    UINT status;

    /* Create the queue. */
    status = tx_queue_create(queue->queue, queue->name, queue->message_size, queue->memory, queue->max_messages * queue->message_size);
    if(status != TX_SUCCESS) {
        printf("[u_queues.c/_create_queue()] ERROR: Failed to create queue (Status: %d, Queue: %s).\n", status, queue->name);
        return U_ERROR;
    }

    return U_SUCCESS;

}

/* Template Queue */
TX_QUEUE queue_template;
static UCHAR _template_memory[1 * 10];    /* (message_size * max_messages) */
static const queue_t _template_config = {
    .queue         = &queue_template,     /* Pointer to the queue. */
    .name          = "Template Queue",    /* Name of the queue. */
    .priority      = 0,                   /* Priority of the queue. */
    .message_size  = 1,                   /* Size of each queue message, in bytes. */
    .max_messages  = 10,                  /* Maximum number of messages in the queue. */
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