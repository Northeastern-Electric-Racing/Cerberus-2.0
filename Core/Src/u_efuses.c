#include "u_efuses.h"
#include "u_general.h"
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

/* Returns the eFuse's raw ADC reading. */
uint16_t efuse_getRaw(efuse_t efuse) {
    uint16_t raw;
    switch(efuse) {
        case EFUSE_DASHBOARD:
            raw = adc_getEFuseData().dashboard;
            break;
        case EFUSE_BRAKE:
            raw = adc_getEFuseData().brake;
            break;
        case EFUSE_SHUTDOWN:
            raw = adc_getEFuseData().shutdown;
            break;
        case EFUSE_LV:
            raw = adc_getEFuseData().lv;
            break;
        case EFUSE_RADFAN:
            raw = adc_getEFuseData().radfan;
            break;
        case EFUSE_PUMP1:
            raw = adc_getEFuseData().pump1;
            break;
        case EFUSE_PUMP2:
            raw = adc_getEFuseData().pump2;
            break;
        case EFUSE_BATTBOX:
            raw = adc_getEFuseData().battbox;
            break;
        case EFUSE_MC:
            raw = adc_getEFuseData().mc;
            break;
        default:
            DEBUG_PRINTLN("ERROR: Unknown EFuse enum was passed into switch statement.");
            return U_ERROR;
    }
    return raw;
}

#define V_REF 3.3f // V_(REF) = 3V3
/* Returns the eFuse's V_(IMON) voltage reading. */
float efuse_getVoltage(efuse_t efuse) {
    // V_(IMON) = (ADC_READING / 4095) * V_(REF)
    const uint16_t ADC_READING = efuse_getRaw(efuse);
    return ((float)ADC_READING / 4095) * V_REF;
}

/* Returns the eFuse's I_(OUT) current reading. */
float efuse_getCurrent(efuse_t efuse) {
    // I_(OUT) = V_(IMON) / (GAIN_(IMON) * R_(IMON))
    const float v_imon = efuse_getVoltage(efuse);
    return v_imon * efuses[efuse].scale;
}

/* Returns the eFuse's fault status (true = faulted, false = not faulted). */
bool efuse_getFaultStatus(efuse_t efuse) {
    return (bool)(HAL_GPIO_ReadPin(efuses[efuse].er_port, efuses[efuse].er_pin) == GPIO_PIN_SET);
}

/* Enables an eFuse. */
void efuse_enable(efuse_t efuse) {
    HAL_GPIO_WritePin(efuses[efuse].en_port, efuses[efuse].en_pin, GPIO_PIN_SET);
}

/* Disables an eFuse. */
void efuse_disable(efuse_t efuse) {
    HAL_GPIO_WritePin(efuses[efuse].en_port, efuses[efuse].en_pin, GPIO_PIN_RESET);
}

/* Returns whether or not the eFuse is enabled (true = eFuse is enabled, false = eFuse is disabled). */
bool efuse_getEnableStatus(efuse_t efuse) {
    return (bool)(HAL_GPIO_ReadPin(efuses[efuse].en_port, efuses[efuse].en_pin) == GPIO_PIN_SET);
}