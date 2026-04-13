#include "u_shutdown.h"
#include <stdatomic.h>
#include "timer.h"
#include "u_tx_queues.h"
#include "u_queues.h"
#include "main.h"
#include "u_faults.h"
#include "u_lightning.h"
#include "debounce.h"

/* Bool to track the BMS shutdown state. */
static _Atomic bool bms_shutdown = false;
// BMS periodically sends out a CAN message reporting the shutdown state. That state is tracked here.
// When this bool is `false`, BMS is indicating that shutdown is NOT active, meaning that we are in normal operation and everything is good.
// When this bool is `true`, BMS is indicating that shutdown IS active, which is bad.get_shutdown

/* Bool the track the VCU shutdown state. */
static _Atomic bool vcu_shutdown = false;
// This is the shutdown state reported by VCU's own pins. It's updated periodically by the shutdown thread, in shutdown_process().shutdown.h
// Just like BMS, when this bool is `false`, VCU's pins are indicating that shutdown is NOT active (meaning normal operation), while `true` indicates that shutdown IS active (not good).
// It should usually by consistent with the BMS shutdown state, but there are certain shutdown pins that are only sent to BMS.
// So, this state should always be used IN COMBINATION with the BMS shutdown state when determining if shutdown is active.
// I.e., the true shutdown state is (bms_shutdown || vcu_shutdown).

// callback for when either bms or imd indicates a fault - sets flag via arg
static void _lightning_board_status_callback(void *arg) {
    *(bool *)arg = true;
}

/* Updates the BMS shutdown state. Should only be called upon receiving the BMS shutdown state message. */
void update_bms_shutdown(bool new_state) {
    bms_shutdown = new_state;
}

/* Indicates if shutdown is active. */
bool is_shutdown_active(void) {
    return bms_shutdown || vcu_shutdown;
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
        tsms_gpio,
        0
    );

    /* If any of the pins AREN'T high, that means shutdown is active. */
    vcu_shutdown = !bms_gpio || !bots_gpio || !spare_gpio || !bspd_gpio || !hv_c || !hvd_gpio || !imd_gpio || !ckpt_gpio || !inertia_sw_gpio || !tsms_gpio;

    /* Lightning status with debounce. */
    bool lightning_fault = !bms_gpio || !imd_gpio;

    // Always call debounce so the cancel path runs when the fault clears.
    // The callback sets lightning_is_red=true via arg; we track it separately
    // so RED is sent every loop iteration (not just once per debounce cycle).
    debounce(lightning_fault, &lightning_status_timer, LIGHTNING_BOARD_DEBOUNCE, &_lightning_board_status_callback, &lightning_is_red);
    if (!lightning_fault) {
        lightning_is_red = false;
        send_lightning_board_status(LIGHT_GREEN);
    } else if (lightning_is_red) {
        send_lightning_board_status(LIGHT_RED);
    }

    /* Check if shutdown is active. If it is, trigger the fault. */
    if (is_shutdown_active()) { // if tsms is still on when shutdown is active, trigger fault
        queue_send(&faults, &(fault_t){SHUTDOWN_FAULT}, TX_NO_WAIT);
    }
}