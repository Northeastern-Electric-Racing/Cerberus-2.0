#include "tx_api.h"
#include "u_bms.h"
#include "u_mutexes.h"
#include "u_faults.h"
#include "u_queues.h"

/* Config */
#define BMS_CAN_MONITOR_DELAY 4000

/* Globals. */
static uint16_t battbox_temp;
static TX_TIMER bms_fault_timer;

/* Fault callback(s). */
static void _bms_fault_callback(ULONG args) {queue_send(&faults, &(fault_t){BMS_CAN_MONITOR_FAULT});}; // Queues the BMS CAN Monitor Fault.

/* Initializes the BMS fault timer. */
int bms_init(void) {

    /* Create BMS Timer. */
    int status = tx_timer_create(
        &bms_fault_timer,         /* Timer Instance */
        "BMS Fault Timer",        /* Timer Name */
        _bms_fault_callback,      /* Timer Expiration Callback */
        0,                        /* Callback Input */
        BMS_CAN_MONITOR_DELAY,    /* Ticks until timer expiration. */
        0,                        /* Number of ticks for all timer expirations after the first (0 makes this a one-shot timer). */
        TX_AUTO_ACTIVATE          /* Automatically start the timer. */
    );
    if(status != TX_SUCCESS) {
        DEBUG_PRINTLN("ERROR: Failed to create BMS Fault Timer (Status: %d/%s).", status, tx_status_toString(status));
        return U_ERROR;
    }

    DEBUG_PRINTLN("Ran bms_init().");

    return U_SUCCESS;
}

/* Restarts the BMS Fault Timer. */
int bms_handleDclMessage(void)
{
    /* Deactivate the BMS fault timer. */
    int status = tx_timer_deactivate(&bms_fault_timer);
    if(status != TX_SUCCESS) {
        DEBUG_PRINTLN("ERROR: Failed to deactivate BMS fault timer (Status: %d/%s).", status, tx_status_toString(status));
        return U_ERROR;
    }

    /* Change the BMS fault timer. */
    status = tx_timer_change(&bms_fault_timer, BMS_CAN_MONITOR_DELAY, 0);
    if(status != TX_SUCCESS) {
        DEBUG_PRINTLN("ERROR: Failed to change BMS fault timer (Status: %d/%s).", status, tx_status_toString(status));
        return U_ERROR;
    }

    /* Activate the BMS fault timer. */
    status = tx_timer_activate(&bms_fault_timer);
    if(status != TX_SUCCESS) {
        DEBUG_PRINTLN("ERROR: Failed to activate BMS fault timer (Status: %d/%s).", status, tx_status_toString(status));
        return U_ERROR;
    }

    return U_SUCCESS;
}

/* Returns the battbox temperature. */
uint16_t bms_getBattboxTemp(void) {
    mutex_get(&bms_mutex);
    uint16_t temp = battbox_temp;
    mutex_put(&bms_mutex);
    return temp;
}

/* Sets the battbox temperature. The "temp" parameter should be taken from the 'BMS/Cells/Temp_Avg_Value' CAN message. */
void bms_setBattboxTemp(uint16_t temp) {
    mutex_get(&bms_mutex);
    battbox_temp = temp;
    mutex_put(&bms_mutex);
}