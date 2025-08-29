#include <stdio.h>
#include "u_mutexes.h"
#include "u_general.h"

/* Template Mutex */
/* only exists to show how to create mutexes (since there aren't any as of writing this. can be removed once an actual real mutex is set up here. */
mutex_t template_mutex = {
    .name = "Template Mutex",      /* Name of the mutex. */
    .priority_inherit = TX_INHERIT /* Priority inheritance setting. */
};

/* Helper function. Creates a ThreadX mutex. */
static uint8_t _create_mutex(mutex_t *mutex) {
    uint8_t status = tx_mutex_create(&mutex->_TX_MUTEX, mutex->name, mutex->priority_inherit);
    if(status != TX_SUCCESS) {
        DEBUG_PRINTLN("ERROR: Failed to create mutex (Status: %s, Name: %s).", tx_status_toString(status), mutex->name);
        return status;
    }

    return U_SUCCESS;
}

/* Initializes all ThreadX mutexes. 
*  Calls to _create_mutex() should go in here
*/
uint8_t mutexes_init() {
    /* Create Mutexes. */
    CATCH_ERROR(_create_mutex(&template_mutex), U_SUCCESS);  // Create Template Mutex.
    // add more as necessary.

    DEBUG_PRINTLN("Ran mutexes_init().");
    return U_SUCCESS;
}

/* Get a mutex. */
uint8_t mutex_get(mutex_t *mutex, ULONG wait_option) {
    uint8_t status = tx_mutex_get(&mutex->_TX_MUTEX, wait_option);
    if(status != TX_SUCCESS) {
        DEBUG_PRINTLN("ERROR: Failed to get mutex (Status: %s, Mutex: %s).", tx_status_toString(status), mutex->name);
        return status;
    }

    return U_SUCCESS;
}

/* Put a mutex. */
uint8_t mutex_put(mutex_t *mutex) {
    uint8_t status = tx_mutex_put(&mutex->_TX_MUTEX);
    if(status != TX_SUCCESS) {
        DEBUG_PRINTLN("ERROR: Failed to put mutex (Status: %s, Mutex: %s).", tx_status_toString(status), mutex->name);
        return status;
    }

    return U_SUCCESS;
}