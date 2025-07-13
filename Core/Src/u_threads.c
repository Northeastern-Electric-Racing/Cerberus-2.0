#include "u_threads.h"
#include "u_queues.h"
#include "u_inbox.h"
#include "u_ethernet.h"

/* Default Thread */
static TX_THREAD _default;
static const thread_t _default_thread_config = {
        .thread     = &_default,         /* Thread */
        .name       = "Default Thread",  /* Name */
        .function   = default_thread,    /* Thread Function */
        .size       = 512,               /* Stack Size (in bytes) */
        .priority   = 9,                 /* Priority */
        .threshold  = 9,                 /* Preemption Threshold */
        .time_slice = TX_NO_TIME_SLICE,  /* Time Slice */
        .auto_start = TX_AUTO_START,     /* Auto Start */
        .sleep      = 500                /* Sleep (in ticks) */
    };
void default_thread(ULONG thread_input) {
    
    while(1) {

        // u_TODO - make thread do stuff

        /* Sleep Thread for specified number of ticks. */
        tx_thread_sleep(_default_thread_config.sleep);
    }
}

/* Ethernet Thread. Sends outgoing messages and processes incoming messages. */
static TX_THREAD _ethernet;
static const thread_t _ethernet_thread_config = {
        .thread     = &_ethernet,        /* Thread */
        .name       = "Ethernet Thread", /* Name */
        .function   = ethernet_thread,   /* Thread Function */
        .size       = 512,               /* Stack Size (in bytes) */
        .priority   = 9,                 /* Priority */
        .threshold  = 9,                 /* Preemption Threshold */
        .time_slice = TX_NO_TIME_SLICE,  /* Time Slice */
        .auto_start = TX_AUTO_START,     /* Auto Start */
        .sleep      = 500                /* Sleep (in ticks) */
    };
void ethernet_thread(ULONG thread_input) {
    
    while(1) {

        ethernet_message_t message;
        uint8_t status;

        /* Process outgoing messages */
        while(queue_receive(&eth_outgoing, &message) == U_SUCCESS) {
            status = ethernet_send_message(&message);
            if(status != U_SUCCESS) {
                DEBUG_PRINT("WARNING: Failed to send message after removing from outgoing queue (Message ID: %d).", message.message_id);
                // u_TODO - maybe add the message back into the queue if it fails to send? not sure if this is a good idea tho
                }
        }

        /* Process incoming messages */
        while(queue_receive(&eth_incoming, &message) == U_SUCCESS) {
            inbox_ethernet(&message);
        }

        /* Sleep Thread for specified number of ticks. */
        tx_thread_sleep(_ethernet_thread_config.sleep);
    }
}

/* Helper function. Creates a ThreadX thread. */
static uint8_t _create_thread(TX_BYTE_POOL *byte_pool, const thread_t *thread) {
    CHAR *pointer;
    uint8_t status;

    /* Allocate the stack for the thread. */
    status = tx_byte_allocate(byte_pool, (VOID**) &pointer, thread->size, TX_NO_WAIT);
    if(status != TX_SUCCESS) {
        DEBUG_PRINT("ERROR: Failed to allocate stack before creating thread (Status: %d, Thread: %s).", status, thread->name);
        return U_ERROR;
    }

    /* Create the thread. */
    status = tx_thread_create(thread->thread, thread->name, thread->function, thread->thread_input, pointer, thread->size, thread->priority, thread->threshold, thread->time_slice, thread->auto_start);
    if(status != TX_SUCCESS) {
        DEBUG_PRINT("ERROR: Failed to create thread (Status: %d, Thread: %s).", status, thread->name);
        return U_ERROR;
    }
    
    return U_SUCCESS;
}

/* Initializes all ThreadX threads. 
*  Calls to _create_thread() should go in here
*/
uint8_t threads_init(TX_BYTE_POOL *byte_pool) {

    /* Create Threads */
    CATCH_ERROR(_create_thread(byte_pool, &_default_thread_config), U_SUCCESS);  // Create Default thread.
    CATCH_ERROR(_create_thread(byte_pool, &_ethernet_thread_config), U_SUCCESS); // Create Ethernet thread.
    // add more threads here if need eventually

    DEBUG_PRINT("Ran threads_init().");
    return U_SUCCESS;
}
