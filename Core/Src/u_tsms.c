#include <stdatomic.h>
#include "u_tsms.h"
#include "tx_api.h"
#include "u_tx_debug.h"
#include "u_tx_timers.h"
#include "main.h"
#include "u_mutexes.h"

#define _TSMS_DEBOUNCE_TIME 10 // Ticks for TSMS signal to debounce.

/* GLOBALS */
static _Atomic bool tsms = false; // Stores TSMS state.

/* TSMS Timer */
static void _timer_callback(ULONG args); // Forward declaration for the timer callback
static timer_t timer = {
    .name = "TSMS Timer",
    .callback = _timer_callback,
    .callback_input = 0,
    .duration = _TSMS_DEBOUNCE_TIME,
    .type = ONESHOT,
    .auto_activate = false
};

/* Timer Callback (called when the timer expires). */
static void _timer_callback(ULONG args) {
    /* If the TSMS pin is still HIGH, set 'tsms' to true. */
    if(HAL_GPIO_ReadPin(TSMS_GPIO_GPIO_Port, TSMS_GPIO_Pin) == GPIO_PIN_SET) {
        tsms = true;
    }
}

/* Init TSMS Debounce Timer. */
int tsms_init(void) {
    int status = timer_init(&timer);
    if(status != U_SUCCESS) {
        PRINTLN_ERROR("Failed to init TSMS debounce timer (Status: %d).", status);
        return U_ERROR;
    }
    return U_SUCCESS;
}

/* Checks the state of the TSMS pin and updates the 'tsms' bool accordingly. Handles all the debounding stuff. Should only be called by the TSMS thread. */
void tsms_update(void) {
    if((HAL_GPIO_ReadPin(TSMS_GPIO_GPIO_Port, TSMS_GPIO_Pin) == GPIO_PIN_SET)) {
        /* If the TSMS pin is high, initiate a debounce routine. */
        /* But first, if a debounce routine is already active (i.e. the timer is actively running), just return and let the timer do its thing. */
        bool active;
        int status = timer_isActive(&timer, &active);
        if(status != U_SUCCESS) {
            PRINTLN_ERROR("Failed to get active status of the TSMS timer (Status: %d).", status);
            return;
        }

        /* If timer is active, return. */
        if(active) {return;}

        /* If timer isn't active, start a new debounce routine. */
        status = timer_restart(&timer);
        if(status != U_SUCCESS) {
            PRINTLN_ERROR("Failed to restart TSMS debounce timer (Status: %d).", status);
        }
        
    } else {
        /* If TSMS pin is not high, no debouncing is needed. Just set 'tsms' to false (after getting the mutex).*/
        tsms = false;
    }
}

/* Gets the Offical TSMS State (not the raw pin state, but the state stored in the debounced 'tsms' bool). */
bool tsms_get(void) {
    return tsms;
}

