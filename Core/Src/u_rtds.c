#include "tx_api.h"
#include "main.h"
#include "u_tx_timers.h"
#include "u_rtds.h"
#include "u_tx_debug.h"

/* Timer for RTDS. */
static void _timer_callback(ULONG args); // Forward declaration for callback function.
static timer_t rtds_timer = {
    .name = "RTDS Timer",
    .callback = _timer_callback,
    .callback_input = 0,
    .duration = RTDS_DURATION,
    .type = ONESHOT,
    .auto_activate = false
};

/* Timer for Reverse Sound. */
static void _reverse_sound_callback(ULONG args); // Forward declaratoin for callback function.
static timer_t reverse_sound_timer = {
    .name = "Reverse Sound Timer",
    .callback = _reverse_sound_callback,
    .callback_input = 0,
    .duration = 50,
    .type = PERIODIC,
    .auto_activate = false
};

/* Sets (i.e. turns on) the RTDS pin. */
static void _set_rtds_pin(void) {
    HAL_GPIO_WritePin(RTDS_GPIO_GPIO_Port, RTDS_GPIO_Pin, GPIO_PIN_SET); // Turn on RTDS pin.
    PRINTLN_INFO("Turned on RTDS pin.");
}

/* Clears (i.e. turns off) the RTDS pin. */
static void _clear_rtds_pin(void) {
    HAL_GPIO_WritePin(RTDS_GPIO_GPIO_Port, RTDS_GPIO_Pin, GPIO_PIN_RESET); // Turn off RTDS pin.
    PRINTLN_INFO("Turned off RTDS pin.");
}

/* Callback for RTDS. Turns off the RTDS after the timer expires. */
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
        int status = timer_init(&rtds_timer);
        if(status != U_SUCCESS) {
            PRINTLN_ERROR("Failed to create RTDS timer (Status: %d).", status);
            return U_ERROR;
        }

        /* Create reverse sound timer */
        status = timer_init(&reverse_sound_timer);
        if(status != U_SUCCESS) {
            PRINTLN_ERROR("Failed to create reverse sound timer (Status: %d).", status);
            return U_ERROR;
        }

        PRINTLN_INFO("Ran rtds_init().");
        return U_SUCCESS;
}

/* Sounds the RTDS (Ready-to-drive sound). */
int rtds_soundRTDS(void) {

    /* Trigger the RTDS sound. */
    _set_rtds_pin();

    /* Restart RTDS timer. */
    int status = timer_restart(&rtds_timer);
    if(status != U_SUCCESS) {
        PRINTLN_ERROR("Failed to restart RTDS timer (Status: %d).", status);
        return U_ERROR;
    }

    return U_SUCCESS;
}

/* Starts the reverse sound (periodic beeping). */
int rtds_startReverseSound(void) {
    /* Stop the Reverse Sound if it's already active. */
    rtds_stopReverseSound();
    
    /* Activate the reverse sound timer to start periodic beeping */
    int status = timer_start(&reverse_sound_timer);
    if(status != U_SUCCESS) {
        PRINTLN_ERROR("Failed to activate reverse sound timer (Status: %d).", status);
        return U_ERROR;
    }
    
    PRINTLN_INFO("Started reverse sound.");
    return U_SUCCESS;
}

/* Stops the reverse sound. */
int rtds_stopReverseSound(void) {
    /* Turn off the RTDS sound */
    _clear_rtds_pin();
    
    /* Deactivate the reverse sound timer */
    int status = timer_stop(&reverse_sound_timer);
    if(status != U_SUCCESS) {
        PRINTLN_ERROR("Failed to deactivate reverse sound timer (Status: %d).", status);
        return U_ERROR;
    }
    
    PRINTLN_INFO("Stopped reverse sound.");
    return U_SUCCESS;
}

/* Reads the status of the RTDS pin (true = RTDS pin is active, false = RTDS pin is not active). */
/* Useful for debugging. */
bool rtds_readRTDS(void) {
    return (bool)(HAL_GPIO_ReadPin(RTDS_GPIO_GPIO_Port, RTDS_GPIO_Pin) == GPIO_PIN_SET);
}