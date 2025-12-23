#include "tx_api.h"
#include "u_tx_timers.h"
#include "u_bms.h"
#include "u_mutexes.h"
#include "u_faults.h"
#include "u_queues.h"

/* Config */
#define BMS_CAN_MONITOR_DELAY 4000

/* Globals. */
static uint16_t battbox_temp;

/* Fault callback(s). */
static void _bms_fault_callback(ULONG args) {queue_send(&faults, &(fault_t){BMS_CAN_MONITOR_FAULT}, TX_NO_WAIT);}; // Queues the BMS CAN Monitor Fault.
static timer_t bms_fault_timer = {
    .name = "BMS Fault Timer",
    .callback = _bms_fault_callback,
    .callback_input = 0,
    .duration = BMS_CAN_MONITOR_DELAY,
    .type = ONESHOT,
    .auto_activate = true
};

/* Initializes the BMS fault timer. */
int bms_init(void) {

    /* Create BMS Timer. */
    int status = timer_init(&bms_fault_timer);
    if(status != U_SUCCESS) {
        PRINTLN_ERROR("Failed to create BMS Fault Timer (Status: %d).", status);
        return U_ERROR;
    }

    PRINTLN_INFO("Ran bms_init().");

    return U_SUCCESS;
}

/* Restarts the BMS Fault Timer. */
int bms_handleDclMessage(void)
{
    int status = timer_restart(&bms_fault_timer);
    if(status != U_SUCCESS) {
        PRINTLN_ERROR("Failed to restart BMS Fault timer (Status: %d).", status);
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