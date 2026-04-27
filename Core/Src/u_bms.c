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
static _Atomic bool precharge = false; // Default to false until BMS confirms precharge is complete

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

int bms_handleBmsFaultMessage(can_msg_t *message) {
    bms_critically_faulted_t data = { 0 };
    receive_bms_critically_faulted(message, &data); 
    PRINTLN_INFO("bms critically faulted=%d", data.critically_faulted);
    if (data.critically_faulted == true) {
        queue_send(&faults, &(fault_t){LATCHING_ACTIVE_FAULT}, TX_NO_WAIT);
    }
}

#define _GET_BIT(data, bit) (((data) & (1U << (bit))) != 0U)
int imd_handleImdFaultMessage(can_msg_t *message) {
    /* Extract the warnings and alarms field (bytes 4 and 5 of the message). */
    uint16_t warnings_and_alarms = 0;
    memcpy(&warnings_and_alarms, &message->data[4], 2); // Copy over two bytes of the message, starting at byte 4. This should result in byte 4 and byte 5 being copied over.
    
    /* Get all the bit states from the register. */
    bool device_error_active = _GET_BIT(warnings_and_alarms, 0); // true = device error active
    bool HV_pos_connection_failure = _GET_BIT(warnings_and_alarms, 1); // true = HV_pos connection failure
    bool HV_neg_connection_failure = _GET_BIT(warnings_and_alarms, 2); // true = HV_neg connection failure
    bool Earth_connection_failure = _GET_BIT(warnings_and_alarms, 3); // true = Earth connection failure
    bool Iso_alarm = _GET_BIT(warnings_and_alarms, 4); // true = Iso value below threshold error
    bool Iso_warning = _GET_BIT(warnings_and_alarms, 5); // true = Iso value below treshold warning
    bool Iso_outdated = _GET_BIT(warnings_and_alarms, 6); // true = Iso outdated
    bool Unbalance_alarm = _GET_BIT(warnings_and_alarms, 7); // true = unbalane value below threshold
    bool Undervoltage_alarm = _GET_BIT(warnings_and_alarms, 8); // true = undervoltage alarm
    bool Unsafe_to_start = _GET_BIT(warnings_and_alarms, 9); // true = Unsafe to start
    bool Earthlift_open = _GET_BIT(warnings_and_alarms, 10); // true = Earthlift open

    /* Do we have an error? */
    bool imd_error = 
    device_error_active ||
    HV_pos_connection_failure ||
    HV_neg_connection_failure ||
    Earth_connection_failure ||
    Iso_alarm ||
    Iso_warning ||
    Iso_outdated ||
    Unbalance_alarm ||
    Undervoltage_alarm ||
    Unsafe_to_start ||
    Earthlift_open;
    // Right now, if any of these are true, we are considering it an error.

    /* Update `has_imd_made_contact`, since we have made contact if this has been called. */
    if (imd_error) {
        queue_send(&faults, &(fault_t){LATCHING_ACTIVE_FAULT}, TX_NO_WAIT);
    }
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
