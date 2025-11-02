#include "u_tsms.h"
#include "tx_api.h"
#include "u_tx_debug.h"
#include "main.h"
#include "u_mutexes.h"

#define _TSMS_DEBOUNCE_TIME 10 // Ticks for TSMS signal to debounce.

/* GLOBALS */
static bool tsms = false; // Stores TSMS state.
static TX_TIMER timer;    // TSMS Timer.

/* Timer Callback (called when the timer expires). */
static void _timer_callback(ULONG args) {
    /* If the TSMS pin is still HIGH, set 'tsms' to true. */
    if(HAL_GPIO_ReadPin(TSMS_GPIO_GPIO_Port, TSMS_GPIO_Pin) == GPIO_PIN_SET) {
        mutex_get(&tsms_mutex);
        tsms = true;
        mutex_put(&tsms_mutex);
    }
}

/* Triggers the TSMS debounce timer. */
static int _trigger_timer(void) {
    /* Deactivate the TSMS timer. */
    int status = tx_timer_deactivate(&timer);
    if(status != TX_SUCCESS) {
        PRINTLN_ERROR("Failed to deactivate TSMS timer (Status: %d/%s).", status, tx_status_toString(status));
        return U_ERROR;
    }

    /* Change the TSMS timer. */
    status = tx_timer_change(&timer, _TSMS_DEBOUNCE_TIME, 0);
    if(status != TX_SUCCESS) {
        PRINTLN_ERROR("Failed to change TSMS timer (Status: %d/%s).", status, tx_status_toString(status));
        return U_ERROR;
    }

    /* Activate the TSMS timer. */
    status = tx_timer_activate(&timer);
    if(status != TX_SUCCESS) {
        PRINTLN_ERROR("Failed to activate TSMS timer (Status: %d/%s).", status, tx_status_toString(status));
        return U_ERROR;
    }
    
    return U_SUCCESS;
}

/* Init TSMS Debounce Timer. */
int tsms_init(void) {
    int status = tx_timer_create(
        &timer,                   /* Timer Instance */
        "TSMS Timer",             /* Timer Name */
        _timer_callback,          /* Timer Expiration Callback */
        0,                        /* Callback Input */
        _TSMS_DEBOUNCE_TIME,      /* Ticks until timer expiration. */
        0,                        /* Number of ticks for all timer expirations after the first (0 makes this a one-shot timer). */
        TX_NO_ACTIVATE            /* Make the timer dormant until it is activated. */
    );
    if(status != TX_SUCCESS) {
        PRINTLN_ERROR("Failed to create TSMS debounce timer (Status: %d/%s).", status, tx_status_toString(status));
        return U_ERROR;
    }
    return U_SUCCESS;
}

/* Checks the state of the TSMS pin and updates the 'tsms' bool accordingly. Handles all the debounding stuff. Should only be called by the TSMS thread. */
void tsms_update(void) {
    if((HAL_GPIO_ReadPin(TSMS_GPIO_GPIO_Port, TSMS_GPIO_Pin) == GPIO_PIN_SET)) {
        /* If the TSMS pin is high, initiate a debounce routine. */
        /* But first, if a debounce routine is already active (i.e. the timer is actively running), just return and let the timer do its thing. */
        UINT active;
        int status = tx_timer_info_get(&timer, TX_NULL, &active, TX_NULL, TX_NULL, TX_NULL);
        if(status != TX_SUCCESS) {
            PRINTLN_ERROR("Failed to get active status of the TSMS timer (Status: %d/%s).", status, tx_status_toString(status));
            return;
        }

        /* If timer is active, return. */
        if(active) {return;}

        /* If timer isn't active, start a new debounce routine. */
        _trigger_timer();
        
    } else {
        /* If TSMS pin is not high, no debouncing is needed. Just set 'tsms' to false (after getting the mutex).*/
        mutex_get(&tsms_mutex);
        tsms = false;
        mutex_put(&tsms_mutex);
    }
}

/* Gets the Offical TSMS State (not the raw pin state, but the state stored in the debounced 'tsms' bool). */
bool tsms_get(void) {
    mutex_get(&tsms_mutex);
    bool state = tsms;
    mutex_put(&tsms_mutex);
    return state;
}

