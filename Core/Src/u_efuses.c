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
} _metadata;

/* EFuse Table. */
/* This table should be kept in the same order as the efuse_t enum in the header file. */
static const _metadata efuses[] = {
    [EFUSE_DASHBOARD] = {.en_pin = EF_DASH_EN_Pin, .en_port = EF_DASH_EN_GPIO_Port, .er_pin = EF_DASH_ER_Pin, .er_port = EF_DASH_ER_GPIO_Port, .scale = SCALE(110)},
    [EFUSE_BRAKE] = {.en_pin = EF_BREAK_EN_Pin, .en_port = EF_BREAK_EN_GPIO_Port, .er_pin = EF_BREAK_ER_Pin, .er_port = EF_BREAK_ER_GPIO_Port, .scale = SCALE(200)},
    [EFUSE_SHUTDOWN] = {.en_pin = EF_SHUTDOWN_EN_Pin, .en_port = EF_SHUTDOWN_EN_GPIO_Port, .er_pin = EF_SHUTDOWN_ER_Pin, .er_port = EF_SHUTDOWN_ER_GPIO_Port, .scale = SCALE(110)},
    [EFUSE_LV] = {.en_pin = EF_LV_EN_Pin, .en_port = EF_LV_EN_GPIO_Port, .er_pin = EF_LV_ER_Pin, .er_port = EF_LV_ER_GPIO_Port, .scale = SCALE(75)},
    [EFUSE_RADFAN] = {.en_pin = EF_RADFAN_EN_Pin, .en_port = EF_RADFAN_EN_GPIO_Port, .er_pin = EF_RADFAN_ER_Pin, .er_port = EF_RADFAN_ER_GPIO_Port, .scale = SCALE(56)},
    [EFUSE_FANBATT] = {.en_pin = EF_FANBATT_EN_Pin, .en_port = EF_FANBATT_EN_GPIO_Port, .er_pin = EF_FANBATT_ER_Pin, .er_port = EF_FANBATT_ER_GPIO_Port, .scale = SCALE(39)},
    [EFUSE_PUMP1] = {.en_pin = EF_PUMP1_EN_Pin, .en_port = EF_PUMP1_EN_GPIO_Port, .er_pin = EF_PUMP1_ER_Pin, .er_port = EF_PUMP1_ER_GPIO_Port, .scale = SCALE(47)},
    [EFUSE_PUMP2] = {.en_pin = EF_PUMP2_EN_Pin, .en_port = EF_PUMP2_EN_GPIO_Port, .er_pin = EF_PUMP2_ER_Pin, .er_port = EF_PUMP2_ER_GPIO_Port, .scale = SCALE(47)},
    [EFUSE_BATTBOX] = {.en_pin = EF_BATTBOX_EN_Pin, .en_port = EF_BATTBOX_EN_GPIO_Port, .er_pin = EF_BATTBOX_ER_Pin, .er_port = EF_BATTBOX_ER_GPIO_Port, .scale = SCALE(39)},
    [EFUSE_MC] = {.en_pin = EF_MC_EN_Pin, .en_port = EF_MC_EN_GPIO_Port, .er_pin = EF_MC_ER_Pin, .er_port = EF_MC_ER_GPIO_Port, .scale = SCALE(56)}
};

/* Returns an instance of efuse_data_t with all current eFuse data. */
#define V_REF 3.3f // V_(REF) = 3V3
static efuse_data_t efuse_getData(void) {
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
    }

    return data;
}

void efuse_enable(efuse_t efuse) {
    HAL_GPIO_WritePin(efuses[efuse].en_port, efuses[efuse].en_pin, GPIO_PIN_SET);
}

void efuse_disable(efuse_t efuse) {
    HAL_GPIO_WritePin(efuses[efuse].en_port, efuses[efuse].en_pin, GPIO_PIN_RESET);
}

void efuse_init(void) {
    efuse_enable(EFUSE_DASHBOARD);
    efuse_enable(EFUSE_LV);
    efuse_enable(EFUSE_BATTBOX);
    efuse_enable(EFUSE_MC);
    efuse_enable(EFUSE_FANBATT);
}

void efuse_update(void) {
    efuse_data_t eFuse_data = efuse_getData();

    uint16_t motor_temp = dti_get_motor_temp();
    uint16_t controller_temp = dti_get_controller_temp();

    if (motor_temp > PUMP1_UPPER_MOTOR_TEMP) {
        if (eFuse_data.enabled[EFUSE_PUMP1] == false) {
            efuse_enable(EFUSE_PUMP1);
        }
    }
    if (motor_temp < PUMP1_LOWER_MOTOR_TEMP) {
        if (eFuse_data.enabled[EFUSE_PUMP1] == true) {
            efuse_disable(EFUSE_PUMP1);
        }
    }

    if (controller_temp > PUMP2_UPPER_CONTROLLER_TEMP) {
        if (eFuse_data.enabled[EFUSE_PUMP2] == false) {
            efuse_enable(EFUSE_PUMP2);
        }
    }
    if (controller_temp < PUMP2_LOWER_CONTROLLER_TEMP) {
        if (eFuse_data.enabled[EFUSE_PUMP2] == true) {
            efuse_disable(EFUSE_PUMP2);
        }
    }

    if (motor_temp > RADFAN_UPPER_MOTOR_TEMP) {
        if (eFuse_data.enabled[EFUSE_RADFAN] == false) {
            efuse_enable(EFUSE_RADFAN);
        }
    }
    if (motor_temp < RADFAN_LOWER_MOTOR_TEMP) {
        if (eFuse_data.enabled[EFUSE_RADFAN] == true) {
            efuse_disable(EFUSE_RADFAN);
        }
    }
}

void efuse_send_to_dashboard(void) {
    efuse_data_t data = efuse_getData();

    send_dashboard_efuse(
        data.raw[EFUSE_DASHBOARD],
        data.voltage[EFUSE_DASHBOARD],
        data.current[EFUSE_DASHBOARD],
        data.faulted[EFUSE_DASHBOARD],
        data.enabled[EFUSE_DASHBOARD]
    );

    send_brake_efuse(
        data.raw[EFUSE_BRAKE],
        data.voltage[EFUSE_BRAKE],
        data.current[EFUSE_BRAKE],
        data.faulted[EFUSE_BRAKE],
        data.enabled[EFUSE_BRAKE]
    );

    send_shutdown_efuse(
        data.raw[EFUSE_SHUTDOWN],
        data.voltage[EFUSE_SHUTDOWN],
        data.current[EFUSE_SHUTDOWN],
        data.faulted[EFUSE_SHUTDOWN],
        data.enabled[EFUSE_SHUTDOWN]
    );

    send_lv_efuse(
        data.raw[EFUSE_LV],
        data.voltage[EFUSE_LV],
        data.current[EFUSE_LV],
        data.faulted[EFUSE_LV],
        data.enabled[EFUSE_LV]
    );

    send_radfan_efuse(
        data.raw[EFUSE_RADFAN],
        data.voltage[EFUSE_RADFAN],
        data.current[EFUSE_RADFAN],
        data.faulted[EFUSE_RADFAN],
        data.enabled[EFUSE_RADFAN]
    );

    send_fanbatt_efuse(
        data.raw[EFUSE_FANBATT],
        data.voltage[EFUSE_FANBATT],
        data.current[EFUSE_FANBATT],
        data.faulted[EFUSE_FANBATT],
        data.enabled[EFUSE_FANBATT]
    );

    send_pumpone_efuse(
        data.raw[EFUSE_PUMP1],
        data.voltage[EFUSE_PUMP1],
        data.current[EFUSE_PUMP1],
        data.faulted[EFUSE_PUMP1],
        data.enabled[EFUSE_PUMP1]
    );

    send_pumptwo_efuse(
        data.raw[EFUSE_PUMP2],
        data.voltage[EFUSE_PUMP2],
        data.current[EFUSE_PUMP2],
        data.faulted[EFUSE_PUMP2],
        data.enabled[EFUSE_PUMP2]
    );

    send_battbox_efuse(
        data.raw[EFUSE_BATTBOX],
        data.voltage[EFUSE_BATTBOX],
        data.current[EFUSE_BATTBOX],
        data.faulted[EFUSE_BATTBOX],
        data.enabled[EFUSE_BATTBOX]
    );

    send_mc_efuse(
        data.raw[EFUSE_MC],
        data.voltage[EFUSE_MC],
        data.current[EFUSE_MC],
        data.faulted[EFUSE_MC],
        data.enabled[EFUSE_MC]
    );
}

void efuse_send_to_serial(void) {
    efuse_data_t data = efuse_getData();

    serial_monitor("lv_efuse", "raw", "%d", data.raw[EFUSE_LV]);
    serial_monitor("lv_efuse", "voltage", "%f", data.voltage[EFUSE_LV]);
    serial_monitor("lv_efuse", "current", "%f", data.current[EFUSE_LV]);
    serial_monitor("lv_efuse", "faulted?", "%d", data.faulted[EFUSE_LV]);
    serial_monitor("lv_efuse", "enabled?", "%d", data.enabled[EFUSE_LV]);

    serial_monitor("radfan_efuse", "raw", "%d", data.raw[EFUSE_RADFAN]);
    serial_monitor("radfan_efuse", "voltage", "%f", data.voltage[EFUSE_RADFAN]);
    serial_monitor("radfan_efuse", "current", "%f", data.current[EFUSE_RADFAN]);
    serial_monitor("radfan_efuse", "faulted?", "%d", data.faulted[EFUSE_RADFAN]);
    serial_monitor("radfan_efuse", "enabled?", "%d", data.enabled[EFUSE_RADFAN]);
}
