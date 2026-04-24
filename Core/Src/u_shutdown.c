#include "u_shutdown.h"
#include <stdatomic.h>
#include "timer.h"
#include "u_tx_queues.h"
#include "u_queues.h"
#include "main.h"
#include "u_faults.h"
#include "u_lightning.h"
#include "debounce.h"
#include "can_messages_tx.h"

/* Bool to track the BMS shutdown state. */
static _Atomic bool bms_shutdown = true; // We should assume that we are shutdown is open (`false`) until BMS confirms that shutdown is open (`true`).
// BMS periodically sends out a CAN message reporting the shutdown state. That state is tracked here.
// When this bool is `false`, BMS is indicating that shutdown is open, which is bad.
// When this bool is `true`, BMS is indicating that shutdown is closed, meaning that we are in normal operation and everything is good

// callback for when either bms or imd indicates a fault - sets flag via arg
static void _lightning_board_status_callback(void *arg) {
    *(bool *)arg = true;
}

/* Updates the BMS shutdown state. Should only be called upon receiving the BMS shutdown state message. */
void update_bms_shutdown(bool new_state) {
    bms_shutdown = new_state;
}

/* Indicates if shutdown is closed or not.
 * When shutdown is closed, this function returns `true`. Otherwise, this function returns `false`.
 * Shutdown has to be closed for us to drive. When shutdown isn't closed, we are not allowed to drive.
*/
bool is_shutdown_closed(void) {
    return bms_shutdown;
}

/* Processes shutdown telemetry and lightning fault. Meant to be called by the shutdown thread. */
static nertimer_t lightning_status_timer;
static bool lightning_is_red = false;
#define LIGHTNING_BOARD_DEBOUNCE 500
void shutdown_process(void) {
    /* Read all of the shutdown pins. */
    bool bms_gpio = (HAL_GPIO_ReadPin(BMS_GPIO_GPIO_Port, BMS_GPIO_Pin) == GPIO_PIN_SET);
    bool bots_gpio = (HAL_GPIO_ReadPin(BOTS_GPIO_GPIO_Port, BOTS_GPIO_Pin) == GPIO_PIN_SET);
    bool spare_gpio = (HAL_GPIO_ReadPin(SPARE_GPIO_GPIO_Port, SPARE_GPIO_Pin) == GPIO_PIN_SET);
    bool bspd_gpio = (HAL_GPIO_ReadPin(BSPD_GPIO_GPIO_Port, BSPD_GPIO_Pin) == GPIO_PIN_SET);
    bool hv_c = (HAL_GPIO_ReadPin(HV_C_GPIO_GPIO_Port, HV_C_GPIO_Pin) == GPIO_PIN_SET);
    bool hvd_gpio = (HAL_GPIO_ReadPin(HVD_GPIO_GPIO_Port, HVD_GPIO_Pin) == GPIO_PIN_SET);
    bool imd_gpio = (HAL_GPIO_ReadPin(IMD_GPIO_GPIO_Port, IMD_GPIO_Pin) == GPIO_PIN_SET);
    bool ckpt_gpio = (HAL_GPIO_ReadPin(CKPT_GPIO_GPIO_Port, CKPT_GPIO_Pin) == GPIO_PIN_SET);
    bool inertia_sw_gpio = (HAL_GPIO_ReadPin(INERTIA_SW_GPIO_GPIO_Port, INERTIA_SW_GPIO_Pin) == GPIO_PIN_SET);
    bool tsms_gpio = (HAL_GPIO_ReadPin(TSMS_GPIO_GPIO_Port, TSMS_GPIO_Pin) == GPIO_PIN_SET);

    /* Send Shutdown Pins CAN message. */
    send_shutdown_pins(
        bms_gpio,
        bots_gpio,
        spare_gpio,
        bspd_gpio,
        hv_c,
        hvd_gpio,
        imd_gpio,
        ckpt_gpio,
        inertia_sw_gpio,
        tsms_gpio
    );

    /* Report BMS Shutdown as reported by VCU. */
    send_bms_shutdown_status_as_reported_by_vcu(bms_shutdown);
}