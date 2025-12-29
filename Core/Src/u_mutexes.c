#include <stdio.h>
#include "u_mutexes.h"
#include "u_tx_debug.h"

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
    CATCH_ERROR(create_mutex(&peripherals_mutex), U_SUCCESS);  // Create Peripherals Mutex.

    // add more as necessary.

    PRINTLN_INFO("Ran mutexes_init().");
    return U_SUCCESS;
}