#include "u_threads.h"

/* Helper function. Creates a ThreadX thread. */
static uint8_t _create_thread(TX_BYTE_POOL *byte_pool, const thread_t *thread) {
    CHAR *pointer;
    uint8_t status;

    /* Allocate the stack for the thread. */
    status = tx_byte_allocate(byte_pool, (VOID**) &pointer, thread->size, TX_NO_WAIT);
    if(status != TX_SUCCESS) {
        printf("[u_threads.c/_create_thread()] ERROR: Failed to allocate stack before creating thread (Status: %d, Thread: %s).\n", status, thread->name);
        return THREADS_STATUS_ERROR;
    }

    /* Create the thread. */
    status = tx_thread_create(thread->thread, thread->name, thread->function, thread->thread_input, pointer, thread->size, thread->priority, thread->threshold, thread->time_slice, thread->auto_start);
    if(status != TX_SUCCESS) {
        printf("[u_threads.c/_create_thread()] ERROR: Failed to create thread (Status: %d, Thread: %s).\n", status, thread->name);
        return THREADS_STATUS_ERROR;
    }
    
    return THREADS_STATUS_OK;
}

/* Template Thread */
static TX_THREAD template;
static const thread_t template_config = {
        .thread     = &template,         /* Thread */
        .name       = "Template Thread", /* Name */
        .function   = thread_template,   /* Thread Function */
        .size       = 512,               /* Stack Size (in bytes) */
        .priority   = 9,                 /* Priority */
        .threshold  = 9,                 /* Preemption Threshold */
        .time_slice = TX_NO_TIME_SLICE,  /* Time Slice */
        .auto_start = TX_AUTO_START,     /* Auto Start */
        .sleep      = 500                /* Sleep (in ticks) */
    };
VOID thread_template(ULONG thread_input) {
    
    while(1) {

        // TODO - make thread do stuff

        /* Sleep Thread for specified number of ticks. */
        tx_thread_sleep(template_config.sleep);
    }
}

/* Initializes all ThreadX threads. 
*  Calls to _threads_create() should go in here
*/
uint8_t threads_init(TX_BYTE_POOL *byte_pool) {

    /* Create Threads */
    _create_thread(byte_pool, &template_config); // Create Template Thread
    // add more threads here if need eventually

    printf("[u_threads.c/threads_init()] SUCCESS: Ran threads_init().\n");
    return THREADS_STATUS_OK;
}