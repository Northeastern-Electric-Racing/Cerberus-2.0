#include "tx_api.h"
#include "main.h"
#include "u_rtds.h"
#include "u_general.h"

static TX_TIMER rtds_timer; /* Timer for the RTDS. */
static TX_TIMER reverse_sound_timer; /* Timer for the reverse sound beeping. */

/* Sets (i.e. turns on) the RTDS pin. */
static void _set_rtds_pin(void) {
    HAL_GPIO_WritePin(RTDS_GPIO_GPIO_Port, RTDS_GPIO_Pin, GPIO_PIN_SET); // Turn on RTDS pin.
    DEBUG_PRINTLN("Turned on RTDS pin.");
}

/* Clears (i.e. turns off) the RTDS pin. */
static void _clear_rtds_pin(void) {
    HAL_GPIO_WritePin(RTDS_GPIO_GPIO_Port, RTDS_GPIO_Pin, GPIO_PIN_RESET); // Turn off RTDS pin.
    DEBUG_PRINTLN("Turned off RTDS pin.");;
}

/* Callback function. Turns off the RTDS after the timer expires. */
static void _timer_callback(ULONG args) {
    _clear_rtds_pin();
}

/* Callback function for reverse sound. Toggles the RTDS on/off to create beeping pattern. */
static void _reverse_sound_callback(ULONG args) {
    static bool sound_state = false;
    if(!sound_state) {
        _clear_rtds_pin();
    }
    else {
        _set_rtds_pin();
    }
    sound_state = !sound_state; /* Toggle for next callback */
}

/* Initializes the RTDS timer. */
int rtds_init(void) {
        int status = tx_timer_create(
            &rtds_timer,        /* Timer Instance */
            "RTDS Timer",       /* Timer Name */
            _timer_callback,    /* Timer Expiration Callback */
            0,                  /* Callback Input */
            RTDS_DURATION,      /* Ticks until timer expiration. */
            0,                  /* Number of ticks for all timer expirations after the first (0 makes this a one-shot timer). */
            TX_NO_ACTIVATE      /* Make the timer dormant until it is activated. */
        );
        if(status != TX_SUCCESS) {
            DEBUG_PRINTLN("ERROR: Failed to create RTDS timer (Status: %d/%s).", status, tx_status_toString(status));
            return U_ERROR;
        }

        /* Create reverse sound timer */
        status = tx_timer_create(
            &reverse_sound_timer,    /* Timer Instance */
            "Reverse Sound Timer",   /* Timer Name */
            _reverse_sound_callback, /* Timer Expiration Callback */
            0,                       /* Callback Input */
            50,                      /* Ticks until timer expiration. */
            50,                      /* Number of ticks for periodic timer. */
            TX_NO_ACTIVATE           /* Make the timer dormant until it is activated. */
        );
        if(status != TX_SUCCESS) {
            DEBUG_PRINTLN("ERROR: Failed to create reverse sound timer (Status: %d/%s).", status, tx_status_toString(status));
            return U_ERROR;
        }

        DEBUG_PRINTLN("Ran rtds_init().");
        return U_SUCCESS;
}

/* Sounds the RTDS (Ready-to-drive sound). */
int rtds_soundRTDS(void) {

    /* Trigger the RTDS sound. */
    _set_rtds_pin();

    /* Deactivate the RTDS timer. */
    int status = tx_timer_deactivate(&rtds_timer);
    if(status != TX_SUCCESS) {
        DEBUG_PRINTLN("ERROR: Failed to deactivate RTDS timer (Status: %d/%s).", status, tx_status_toString(status));
        return U_ERROR;
    }

    /* Change the RTDS timer. */
    status = tx_timer_change(&rtds_timer, RTDS_DURATION, 0);
    if(status != TX_SUCCESS) {
        DEBUG_PRINTLN("ERROR: Failed to change RTDS timer (Status: %d/%s).", status, tx_status_toString(status));
        return U_ERROR;
    }

    /* Activate the RTDS timer. */
    status = tx_timer_activate(&rtds_timer);
    if(status != TX_SUCCESS) {
        DEBUG_PRINTLN("ERROR: Failed to activate RTDS timer (Status: %d/%s).", status, tx_status_toString(status));
        return U_ERROR;
    }

    return U_SUCCESS;
}

/* Starts the reverse sound (periodic beeping). */
int rtds_startReverseSound(void) {
    /* Stop the Reverse Sound if it's already active. */
    rtds_stopReverseSound();
    
    /* Activate the reverse sound timer to start periodic beeping */
    int status = tx_timer_activate(&reverse_sound_timer);
    if(status != TX_SUCCESS) {
        DEBUG_PRINTLN("ERROR: Failed to activate reverse sound timer (Status: %d/%s).", status, tx_status_toString(status));
        return U_ERROR;
    }
    
    DEBUG_PRINTLN("Started reverse sound.");
    return U_SUCCESS;
}

/* Stops the reverse sound. */
int rtds_stopReverseSound(void) {
    /* Turn off the RTDS sound */
    _clear_rtds_pin();
    
    /* Deactivate the reverse sound timer */
    int status = tx_timer_deactivate(&reverse_sound_timer);
    if(status != TX_SUCCESS && status != TX_ACTIVATE_ERROR) { /* TX_ACTIVATE_ERROR means timer was already inactive */
        DEBUG_PRINTLN("ERROR: Failed to deactivate reverse sound timer (Status: %d/%s).", status, tx_status_toString(status));
        return U_ERROR;
    }
    
    DEBUG_PRINTLN("Stopped reverse sound.");
    return U_SUCCESS;
}

/* Reads the status of the RTDS pin (true = RTDS pin is active, false = RTDS pin is not active). */
/* Useful for debugging. */
bool rtds_readRTDS(void) {
    return (bool)(HAL_GPIO_ReadPin(RTDS_GPIO_GPIO_Port, RTDS_GPIO_Pin) == GPIO_PIN_SET);
}