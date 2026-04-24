#include <stdatomic.h>
#include "tx_api.h"
#include "u_tx_timers.h"
#include "u_bms.h"
#include "u_mutexes.h"
#include "u_faults.h"
#include "u_queues.h"

/* Config */
#define BMS_CAN_MONITOR_DELAY 4000

/* Globals. */
static _Atomic float battbox_temp;
static _Atomic bool precharge = true; // Default to false until BMS confirms precharge is complete

static void _bms_fault_callback(ULONG args); // Forward declaration

static timer_t bms_fault_timer = {
    .name = "BMS Fault Timer",
    .callback = _bms_fault_callback,
    .callback_input = 0,
    .duration = BMS_CAN_MONITOR_DELAY,
    .type = ONESHOT,
    .auto_activate = true
};

/* Fault callback(s). */
static void _bms_fault_callback(ULONG args) {
    queue_send(&faults, &(fault_t){BMS_CAN_MONITOR_FAULT}, TX_NO_WAIT);
    timer_restart(&bms_fault_timer);
} // Queues the BMS CAN Monitor Fault.

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
float bms_getBattboxTemp(void) {
    return battbox_temp;
}

/* Sets the battbox temperature. The "temp" parameter should be taken from the 'BMS/Cells/Temp_Avg_Value' CAN message. */
void bms_setBattboxTemp(float temp) {
    battbox_temp = temp;
}

void bms_receivePrechargeState(precharge_state_t state) {

    if (state == PRECHARGE_OPEN) {
        precharge = false;

    } else if (state == PRECHARGE_FLOATING) {
        precharge = false;
        // queue critical fault if precharge is floating
        queue_send(&faults, &(fault_t){PRECHARGE_FLOATING_FAULT}, TX_NO_WAIT);
    } else if (state == PRECHARGE_CLOSED) {
        precharge = true;
    } else {
        PRINTLN_WARNING("Received invalid precharge state from CAN message: %d", state);
    }
}

bool bms_getPrecharge(void) {
    return precharge;
}
