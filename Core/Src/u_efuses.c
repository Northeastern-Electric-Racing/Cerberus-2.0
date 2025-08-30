#include "u_efuses.h"
#include "u_general.h"

static uint32_t _buffer[NUM_EFUSES]; /* Buffer to hold eFuse ADC readings. Each index corresponds to a different eFuse. */

typedef struct {
    const int en_pin;             /* EN (Enable) pin for this eFuse. It enables/disables the eFuse. */
    const GPIO_TypeDef* en_port;  /* GPIO port for the EN pin. */
    const int er_pin;             /* ER (Error) pin for this eFuse. It indicates if the eFuse is experiencing an error. */
    const GPIO_TypeDef* er_port;  /* GPIO port for the ER pin. */
} _metadata;

/* eFuse Table */
/* This table should be kept in the same order as the efuse_t enum in the header file. */
static _metadata efuses[] = {
    [EFUSE_DASH] = {EF_DASH_EN_Pin, EF_DASH_EN_GPIO_Port, EF_DASH_ER_Pin, EF_DASH_ER_GPIO_Port},
    [EFUSE_BREAK] = {EF_BREAK_EN_Pin, EF_BREAK_EN_GPIO_Port, EF_BREAK_ER_Pin, EF_BREAK_ER_GPIO_Port},
    [EFUSE_SHUTDOWN] = {EF_SHUTDOWN_EN_Pin, EF_SHUTDOWN_EN_GPIO_Port, EF_SHUTDOWN_ER_Pin, EF_SHUTDOWN_ER_GPIO_Port},
    [EFUSE_LV] = {EF_LV_EN_Pin, EF_LV_EN_GPIO_Port, EF_LV_ER_Pin, EF_LV_ER_GPIO_Port},
    [EFUSE_RADFAN] = {EF_RADFAN_EN_Pin, EF_RADFAN_EN_GPIO_Port, EF_RADFAN_ER_Pin, EF_RADFAN_ER_GPIO_Port},
    [EFUSE_FANBATT] = {EF_FANBATT_EN_Pin, EF_FANBATT_EN_GPIO_Port, EF_FANBATT_ER_Pin, EF_FANBATT_ER_GPIO_Port},
    [EFUSE_PUMP1] = {EF_PUMP1_EN_Pin, EF_PUMP1_EN_GPIO_Port, EF_PUMP1_ER_Pin, EF_PUMP1_ER_GPIO_Port},
    [EFUSE_PUMP2] = {EF_PUMP2_EN_Pin, EF_PUMP2_EN_GPIO_Port, EF_PUMP2_ER_Pin, EF_PUMP2_ER_GPIO_Port},
    [EFUSE_BATTBOX] = {EF_BATTBOX_EN_Pin, EF_BATTBOX_EN_GPIO_Port, EF_BATTBOX_ER_Pin, EF_BATTBOX_ER_GPIO_Port},
    [EFUSE_MC] = {EF_MC_EN_Pin, EF_MC_EN_GPIO_Port, EF_MC_ER_Pin, EF_MC_ER_GPIO_Port}
};

uint8_t efuses_init(void) {

    /* Start ADC DMA */
    HAL_StatusTypeDef status = HAL_ADC_Start_DMA(&hadc1, _buffer, NUM_EFUSES);
    if(status != HAL_OK) {
        DEBUG_PRINTLN("ERROR: Failed to start ADC DMA for eFuses (Status: %d/%s).", status, hal_status_toString(status));
        return U_ERROR;
    }

    return U_SUCCESS;
}

/* Returns the eFuse's raw ADC reading. */
uint16_t efuse_getRaw(efuse_t efuse) {
    return _buffer[efuse];
}