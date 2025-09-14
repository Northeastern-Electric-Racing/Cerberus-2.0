#include <stdio.h>
#include "u_mutexes.h"
#include "u_general.h"

/* Faults Mutex */
/* Used to protect multiple threads attempting to write to the fault flags variable at once. */
mutex_t faults_mutex = {
    .name = "Faults Mutex",        /* Name of the mutex. */
    .priority_inherit = TX_INHERIT /* Priority inheritance setting. */
};

/* Brake State Mutex */
mutex_t brake_state_mutex = {
    .name = "Brake State Mutex",   /* Name of the mutex. */
    .priority_inherit = TX_INHERIT /* Priority inheritance setting. */
};

/* Pedal Data Mutex */
mutex_t pedal_data_mutex = {
    .name = "Pedal Data Mutex",    /* Name of the mutex. */
    .priority_inherit = TX_INHERIT /* Priority inheritance setting. */
};

/* BMS Mutex */
mutex_t bms_mutex = {
    .name = "BMS Mutex",           /* Name of the mutex. */
    .priority_inherit = TX_INHERIT /* Priority inheritance setting. */
};

/* Torque Limit Mutex */
mutex_t torque_limit_mutex = {
    .name = "Torque Limit Mutex",  /* Name of the mutex. */
    .priority_inherit = TX_INHERIT /* Priority inheritance setting. */
};

/* DTI Mutex */
mutex_t dti_mutex = {
    .name = "DTI Mutex",           /* Name of the mutex. */
    .priority_inherit = TX_INHERIT /* Priority inheritance setting. */
};

/* Helper function. Creates a ThreadX mutex. */
static uint8_t _create_mutex(mutex_t *mutex) {
    uint8_t status = tx_mutex_create(&mutex->_TX_MUTEX, (CHAR*)mutex->name, mutex->priority_inherit);
    if(status != TX_SUCCESS) {
        DEBUG_PRINTLN("ERROR: Failed to create mutex (Status: %d/%s, Name: %s).", status, tx_status_toString(status), mutex->name);
        return status;
    }

    return U_SUCCESS;
}

/* Initializes all ThreadX mutexes. 
*  Calls to _create_mutex() should go in here
*/
uint8_t mutexes_init() {
    /* Create Mutexes. */
    CATCH_ERROR(_create_mutex(&faults_mutex), U_SUCCESS);       // Create Faults Mutex.
    CATCH_ERROR(_create_mutex(&brake_state_mutex), U_SUCCESS);  // Create Brake State Mutex.
    CATCH_ERROR(_create_mutex(&pedal_data_mutex), U_SUCCESS);   // Create Pedal Data Mutex.
    CATCH_ERROR(_create_mutex(&bms_mutex), U_SUCCESS);          // Create BMS Mutex.
    CATCH_ERROR(_create_mutex(&torque_limit_mutex), U_SUCCESS); // Create Torque Limit Mutex.
    CATCH_ERROR(_create_mutex(&dti_mutex), U_SUCCESS);          // Create DTI Mutex.
    // add more as necessary.

    DEBUG_PRINTLN("Ran mutexes_init().");
    return U_SUCCESS;
}

/* Get a mutex. */
uint8_t mutex_get(mutex_t *mutex) {
    uint8_t status = tx_mutex_get(&mutex->_TX_MUTEX, TX_WAIT_FOREVER);
    if(status != TX_SUCCESS) { // Note: As long as TX_WAIT_FOREVER is used, tx_mutex_get() should never be unsucessful here (since it waits indefinitely until the mutex is available).
        DEBUG_PRINTLN("ERROR: Failed to get mutex (Status: %d/%s, Mutex: %s).", status, tx_status_toString(status), mutex->name);
        return status;
    }

    return U_SUCCESS;
}

/* Put a mutex. */
uint8_t mutex_put(mutex_t *mutex) {
    uint8_t status = tx_mutex_put(&mutex->_TX_MUTEX);
    if(status != TX_SUCCESS) {
        DEBUG_PRINTLN("ERROR: Failed to put mutex (Status: %d/%s, Mutex: %s).", status, tx_status_toString(status), mutex->name);
        return status;
    }

    return U_SUCCESS;
}