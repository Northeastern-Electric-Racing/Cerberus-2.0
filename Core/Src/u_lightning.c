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
static void _bms_fault_callback(ULONG args) {
    queue_send(&faults, &(fault_t){LIGHTNING_CAN_MONITOR_FAULT},
               TX_NO_WAIT);
}; // Queues the BMS CAN Monitor Fault.
static timer_t lightning_fault_timer = {.name = "Lightning Fault Timer",
                                        .callback = _bms_fault_callback,
                                        .callback_input = 0,
                                        .duration =
                                            LIGHTNING_CAN_MONITOR_DELAY,
                                        .type = ONESHOT,
                                        .auto_activate = true};

/* Initializes the lightning fault timer. */
int lightning_init(void) {
    /* Create BMS Timer. */
    int status = timer_init(&lightning_fault_timer);
    if (status != U_SUCCESS) {
        PRINTLN_ERROR("Failed to create BMS Fault Timer (Status: %d).",
                      status);
        return U_ERROR;
    }

    PRINTLN_INFO("Ran bms_init().");

    return U_SUCCESS;
}

/* Restarts the lightning fault timer. */
int lightning_handleIMUMessage(void) {
    int status = timer_restart(&lightning_fault_timer);
    if (status != U_SUCCESS) {
        PRINTLN_ERROR("Failed to restart BMS Fault timer (Status: %d).",
                      status);
        return U_ERROR;
    }

    return U_SUCCESS;
}

void send_lightning_board_status(Lightning_Board_Light_Status status) {
    send_lightning_board_light_status(status);
}
