#include <stdatomic.h>
#include "u_efuses.h"
#include "u_tx_debug.h"
#include "u_adc.h"

/* TPS1663x (eFuse) datasheet: https://www.ti.com/lit/ds/symlink/tps1663.pdf?ts=1756438634613 */

/* Config */
#define GAIN_IMON 27.9e-6f           /* GAIN_(IMON) = 27.9 uA/A. Taken from Altium schematic. */

/* Scale calculation macro. */
/* R_IMON is a value in kOhms. It depends on current (and differs for each eFuse). Check the Altium schematic. */
#define SCALE(R_IMON) (1.0f) / (GAIN_IMON * R_IMON * 1000.0f)

/* EFuse Metadata. */
typedef struct {
    int en_pin;             /* EN (Enable) pin for this eFuse. It enables/disables the eFuse. */
    GPIO_TypeDef* en_port;  /* GPIO port for the EN pin. */
    int er_pin;             /* ER (Error) pin for this eFuse. It indicates if the eFuse is experiencing an error. */
    GPIO_TypeDef* er_port;  /* GPIO port for the ER pin. */
    float scale;            /* scale = 1 / (GAIN_IMON * R_IMON). Used to calculate current. */
    efuse_control_state_t default_state; /* The default control state for the eFuse. This will be the initial state before any Calypso commands are received. */
} _metadata;

/* EFuse Table. */
/* This table should be kept in the same order as the efuse_t enum in the header file. */
static const _metadata efuses[] = {
    [EFUSE_DASHBOARD] = {.en_pin = EF_DASH_EN_Pin, .en_port = EF_DASH_EN_GPIO_Port, .er_pin = EF_DASH_ER_Pin, .er_port = EF_DASH_ER_GPIO_Port, .scale = SCALE(39), .default_state = EF_ON},
    [EFUSE_BRAKE] = {.en_pin = EF_BREAK_EN_Pin, .en_port = EF_BREAK_EN_GPIO_Port, .er_pin = EF_BREAK_ER_Pin, .er_port = EF_BREAK_ER_GPIO_Port, .scale = SCALE(200), .default_state = EF_ON},
    [EFUSE_SHUTDOWN] = {.en_pin = EF_SHUTDOWN_EN_Pin, .en_port = EF_SHUTDOWN_EN_GPIO_Port, .er_pin = EF_SHUTDOWN_ER_Pin, .er_port = EF_SHUTDOWN_ER_GPIO_Port, .scale = SCALE(110), .default_state = EF_ON},
    [EFUSE_LV] = {.en_pin = EF_LV_EN_Pin, .en_port = EF_LV_EN_GPIO_Port, .er_pin = EF_LV_ER_Pin, .er_port = EF_LV_ER_GPIO_Port, .scale = SCALE(39), .default_state = EF_ON},
    [EFUSE_RADFAN] = {.en_pin = EF_RADFAN_EN_Pin, .en_port = EF_RADFAN_EN_GPIO_Port, .er_pin = EF_RADFAN_ER_Pin, .er_port = EF_RADFAN_ER_GPIO_Port, .scale = SCALE(56), .default_state = EF_AUTO},
    [EFUSE_FANBATT] = {.en_pin = EF_FANBATT_EN_Pin, .en_port = EF_FANBATT_EN_GPIO_Port, .er_pin = EF_FANBATT_ER_Pin, .er_port = EF_FANBATT_ER_GPIO_Port, .scale = SCALE(27), .default_state = EF_AUTO},
    [EFUSE_PUMP1] = {.en_pin = EF_PUMP1_EN_Pin, .en_port = EF_PUMP1_EN_GPIO_Port, .er_pin = EF_PUMP1_ER_Pin, .er_port = EF_PUMP1_ER_GPIO_Port, .scale = SCALE(47), .default_state = EF_AUTO},
    [EFUSE_PUMP2] = {.en_pin = EF_PUMP2_EN_Pin, .en_port = EF_PUMP2_EN_GPIO_Port, .er_pin = EF_PUMP2_ER_Pin, .er_port = EF_PUMP2_ER_GPIO_Port, .scale = SCALE(47), .default_state = EF_AUTO},
    [EFUSE_BATTBOX] = {.en_pin = EF_BATTBOX_EN_Pin, .en_port = EF_BATTBOX_EN_GPIO_Port, .er_pin = EF_BATTBOX_ER_Pin, .er_port = EF_BATTBOX_ER_GPIO_Port, .scale = SCALE(56), .default_state = EF_ON},
    [EFUSE_MC] = {.en_pin = EF_MC_EN_Pin, .en_port = EF_MC_EN_GPIO_Port, .er_pin = EF_MC_ER_Pin, .er_port = EF_MC_ER_GPIO_Port, .scale = SCALE(56), .default_state = EF_ON}
};

/* eFuse State Array (updated by Calypso command messages). */
static _Atomic efuse_control_state_t _efuse_control_state[NUM_EFUSES]; // The initial/default values for each index is set in efuse_init(), as configured in the efuses[] table.

/* Inititialize the _efuse_control_state array to the values configured in the efuses[] table. */
int efuse_init(void) {

    for(efuse_t efuse = 0; efuse < NUM_EFUSES; efuse++) {
        _efuse_control_state[efuse] = efuses[efuse].default_state;
    }

    return U_SUCCESS;
}

/* Returns an instance of efuse_data_t with all current eFuse data. */
#define V_REF 3.3f // V_(REF) = 3V3
efuse_data_t efuse_getData(void) {
    raw_efuse_adc_t adc = adc_getEFuseData();

    /* Loop through each eFuse and calculate the necessary values. */
    efuse_data_t data = { 0 };
    for(efuse_t efuse = 0; efuse < NUM_EFUSES; efuse++) {
        data.raw[efuse] = adc.data[efuse]; // Get Raw ADC readings.

        /* Calculate the eFuse's V_(IMON) voltage reading. */
        // V_(IMON) = (ADC_READING / 4095) * V_(REF)
        data.voltage[efuse] = ((float)(adc.data[efuse]) / 4095) * V_REF;

        /* Calculate the eFuse's I_(OUT) current reading. */
        data.current[efuse] = data.voltage[efuse] * efuses[efuse].scale;

        /* Get the eFuse's fault status (true = faulted, false = not faulted). */
        data.faulted[efuse] = (bool)(HAL_GPIO_ReadPin(efuses[efuse].er_port, efuses[efuse].er_pin) == GPIO_PIN_RESET); // GPIO_PIN_RESET means that a fault has been detected. GPIO_PIN_SET means that no fault has been detected.

        /* Get the eFuse's enable status (true = eFuse is enabled, false = eFuse is disabled). */
        data.enabled[efuse] = (bool)(HAL_GPIO_ReadPin(efuses[efuse].en_port, efuses[efuse].en_pin) == GPIO_PIN_SET);

        /* Get the eFuse's current control state (as understood by VCU) from the control state tracking array. */
        data.control_state[efuse] = _efuse_control_state[efuse];
    }

    return data;
}

/* Enables an eFuse. */
void efuse_enable(efuse_t efuse) {
    HAL_GPIO_WritePin(efuses[efuse].en_port, efuses[efuse].en_pin, GPIO_PIN_SET);
}

/* Disables an eFuse. */
void efuse_disable(efuse_t efuse) {
    HAL_GPIO_WritePin(efuses[efuse].en_port, efuses[efuse].en_pin, GPIO_PIN_RESET);
}

/* Updates an eFuse's control state. Intended to be called when the relevant commands from Calypso are received. */
void efuse_update_state(efuse_t efuse, efuse_control_state_t state) {
    _efuse_control_state[efuse] = state;
}