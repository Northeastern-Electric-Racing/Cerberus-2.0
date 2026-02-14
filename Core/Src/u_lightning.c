#include <stdatomic.h>
#include "u_lightning.h"
#include "can_messages_tx.h"
#include "u_faults.h"
#include "u_queues.h"
#include "u_tx_timers.h"
#include "tx_api.h"

/* Config */
#define LIGHTNING_CAN_MONITOR_DELAY 4000

/* Fault callback(s). */
static void _lightning_fault_callback(ULONG args) {
    queue_send(&faults, &(fault_t){LIGHTNING_CAN_MONITOR_FAULT}, TX_NO_WAIT);
}
// Queues the Lightning CAN Monitor Fault.
static timer_t lightning_fault_timer = {
    .name = "Lightning Fault Timer",
    .callback = _lightning_fault_callback,
    .callback_input = 0,
    .duration = LIGHTNING_CAN_MONITOR_DELAY,
    .type = ONESHOT,
    .auto_activate = true
};

/* Initializes the lightning fault timer. */
int lightning_init(void) {
    /* Create Lightning Timer. */
    int status = timer_init(&lightning_fault_timer);
    if (status != U_SUCCESS) {
        PRINTLN_ERROR("Failed to create lightning Fault Timer (Status: %d).", status);
        return U_ERROR;
    }

    PRINTLN_INFO("Ran lightning_init().");

    return U_SUCCESS;
}

/* Restarts the lightning fault timer. */
int lightning_handleIMUMessage(void) {
    int status = timer_restart(&lightning_fault_timer);
    if (status != U_SUCCESS) {
        PRINTLN_ERROR("Failed to restart lightning Fault timer (Status: %d).", status);
        return U_ERROR;
    }

    return U_SUCCESS;
}


void send_lightning_board_status(Lightning_Board_Light_Status status) {
    send_lightning_board_light_status(status);
}

void lightning_init(void) {
    send_lightning_board_light_status(LIGHT_OFF);
}
void update_lightning_board_status(bool bms_gpio, bool imd_gpio) {
    Lightning_Board_Light_Status status; // to create the status variable

    if (bms_gpio || imd_gpio) // if either of them are faulted
        {
            status = LIGHT_RED;
        }
        else
        {
            status = LIGHT_GREEN; 
        }
        send_lightning_board_status(status);
}

