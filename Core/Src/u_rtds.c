#include "tx_api.h"
#include "main.h"
#include "u_rtds.h"
#include "u_general.h"

static TX_TIMER rtds_timer; /* Timer for the RTDS. */

/* Callback function. Turns off the RTDS after the timer expires. */
static void timer_callback(ULONG args) {
    HAL_GPIO_WritePin(RTDS_GPIO_GPIO_Port, RTDS_GPIO_Pin, GPIO_PIN_RESET); // Turn off RTDS pin.
    DEBUG_PRINTLN("Turned off RTDS pin.");
}

/* Initializes the RTDS timer. */
int rtds_init(void) {
        int status = tx_timer_create(
            &rtds_timer,        /* Timer Instance */
            "RTDS Timer",       /* Timer Name */
            timer_callback,     /* Timer Expiration Callback */
            0,                  /* Callback Input */
            RTDS_DURATION,      /* Ticks until timer expiration. */
            0,                  /* Number of ticks for all timer expirations after the first (0 makes this a one-shot timer). */
            TX_NO_ACTIVATE      /* Make the timer dormant until it is activated. */
        );
        if(status != TX_SUCCESS) {
            DEBUG_PRINTLN("ERROR: Failed to create RTDS timer (Status: %d/%s).", status, tx_status_toString(status));
            return U_ERROR;
        }

        DEBUG_PRINTLN("Ran rtds_init().");
        return U_SUCCESS;
}

/* Sounds the RTDS (Ready-to-drive sound). */
int rtds_soundRTDS(void) {

    /* Trigger the RTDS sound. */
    HAL_GPIO_WritePin(RTDS_GPIO_GPIO_Port, RTDS_GPIO_Pin, GPIO_PIN_SET);

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

/* Reads the status of the RTDS pin (true = RTDS pin is active, false = RTDS pin is not active). */
/* Useful for debugging. */
bool rtds_readRTDS(void) {
    return (bool)(HAL_GPIO_ReadPin(RTDS_GPIO_GPIO_Port, RTDS_GPIO_Pin) == GPIO_PIN_SET);
}