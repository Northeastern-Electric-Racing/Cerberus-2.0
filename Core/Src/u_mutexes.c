#include <stdio.h>
#include "u_mutexes.h"
#include "u_tx_debug.h"

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

/* ADC1 Mutex */
mutex_t adc1_mutex = {
    .name = "ADC1 Mutex",           /* Name of the mutex. */
    .priority_inherit = TX_INHERIT /* Priority inheritance setting. */
};

/* ADC2 Mutex */
mutex_t adc2_mutex = {
    .name = "ADC2 Mutex",           /* Name of the mutex. */
    .priority_inherit = TX_INHERIT /* Priority inheritance setting. */
};

/* Initializes all ThreadX mutexes. 
*  Calls to _create_mutex() should go in here
*/
uint8_t mutexes_init() {
    /* Create Mutexes. */
    CATCH_ERROR(create_mutex(&faults_mutex), U_SUCCESS);       // Create Faults Mutex.
    CATCH_ERROR(create_mutex(&brake_state_mutex), U_SUCCESS);  // Create Brake State Mutex.
    CATCH_ERROR(create_mutex(&pedal_data_mutex), U_SUCCESS);   // Create Pedal Data Mutex.
    CATCH_ERROR(create_mutex(&bms_mutex), U_SUCCESS);          // Create BMS Mutex.
    CATCH_ERROR(create_mutex(&torque_limit_mutex), U_SUCCESS); // Create Torque Limit Mutex.
    CATCH_ERROR(create_mutex(&dti_mutex), U_SUCCESS);          // Create DTI Mutex.
    CATCH_ERROR(create_mutex(&adc1_mutex), U_SUCCESS);         // Create ADC1 Mutex.
    CATCH_ERROR(create_mutex(&adc2_mutex), U_SUCCESS);         // Create ADC2 Mutex.

    // add more as necessary.

    DEBUG_PRINTLN("Ran mutexes_init().");
    return U_SUCCESS;
}