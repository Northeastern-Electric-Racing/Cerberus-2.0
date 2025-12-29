#include <stdio.h>
#include "u_mutexes.h"
#include "u_tx_debug.h"

/* BMS Mutex */
mutex_t bms_mutex = {
    .name = "BMS Mutex",           /* Name of the mutex. */
    .priority_inherit = TX_INHERIT /* Priority inheritance setting. */
};

/* DTI Mutex */
mutex_t dti_mutex = {
    .name = "DTI Mutex",           /* Name of the mutex. */
    .priority_inherit = TX_INHERIT /* Priority inheritance setting. */
};

/* Peripherals Mutex */
mutex_t peripherals_mutex = {
    .name = "Peripherals Mutex",   /* Name of the mutex. */
    .priority_inherit = TX_INHERIT /* Priority inheritance setting. */
};

/* Initializes all ThreadX mutexes. 
*  Calls to _create_mutex() should go in here
*/
uint8_t mutexes_init() {
    /* Create Mutexes. */
    CATCH_ERROR(create_mutex(&bms_mutex), U_SUCCESS);          // Create BMS Mutex.
    CATCH_ERROR(create_mutex(&dti_mutex), U_SUCCESS);          // Create DTI Mutex.
    CATCH_ERROR(create_mutex(&peripherals_mutex), U_SUCCESS);  // Create Peripherals Mutex.

    // add more as necessary.

    PRINTLN_INFO("Ran mutexes_init().");
    return U_SUCCESS;
}