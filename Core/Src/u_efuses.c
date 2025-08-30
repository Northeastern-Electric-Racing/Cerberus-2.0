#include "u_efuses.h"
#include "u_general.h"

/* TPS1663x (eFuse) datasheet: https://www.ti.com/lit/ds/symlink/tps1663.pdf?ts=1756438634613 */

/* Config */
#define GAIN_IMON 27.9e-6f           /* GAIN_(IMON) = 27.9 uA/A. Taken from Altium schematic. */
#define NUM_EFUSES 10                /* Number of eFuses. */
static uint32_t _buffer[NUM_EFUSES]; /* Buffer to hold eFuse ADC readings. Each index corresponds to a different eFuse. */

/* Scale calculation macro. */
/* R_IMON is a value in kOhms. It depends on current (and differs for each eFuse). Check the Altium schematic. */
#define SCALE(R_IMON) (1.0f) / (GAIN_IMON * R_IMON * 1000.0f)


/* Dashboard eFuse. */
const efuse_t ef_dashboard = {
    .en_pin = EF_DASH_EN_Pin,        /* Enable pin. */
    .en_port = EF_DASH_EN_GPIO_Port, /* GPIO port for Enable pin. */
    .er_pin = EF_DASH_ER_Pin,        /* Error pin. */
    .er_port = EF_DASH_ER_GPIO_Port, /* GPIO port for Error pin. */
    .scale = SCALE(110),             /* Used to calculate current. */
    .rank = 0                        /* eFuse ADC channel rank. */
};

/* Break eFuse. */
const efuse_t ef_break = {
    .en_pin = EF_BREAK_EN_Pin,         /* Enable pin. */
    .en_port = EF_BREAK_EN_GPIO_Port,  /* GPIO port for Enable pin. */
    .er_pin = EF_BREAK_ER_Pin,         /* Error pin. */
    .er_port = EF_BREAK_ER_GPIO_Port,  /* GPIO port for Error pin. */
    .scale = SCALE(200),               /* Used to calculate current. */
    .rank = 1                          /* eFuse ADC channel rank. */
};

/* Shutdown eFuse. */
const efuse_t ef_shutdown = {
    .en_pin = EF_SHUTDOWN_EN_Pin,        /* Enable pin. */
    .en_port = EF_SHUTDOWN_EN_GPIO_Port, /* GPIO port for Enable pin. */
    .er_pin = EF_SHUTDOWN_ER_Pin,        /* Error pin. */
    .er_port = EF_SHUTDOWN_ER_GPIO_Port, /* GPIO port for Error pin. */
    .scale = SCALE(110),                 /* Used to calculate current. */
    .rank = 2                            /* eFuse ADC channel rank. */
};

/* LV eFuse. */
const efuse_t ef_lv = {
    .en_pin = EF_LV_EN_Pin,        /* Enable pin. */
    .en_port = EF_LV_EN_GPIO_Port, /* GPIO port for Enable pin. */
    .er_pin = EF_LV_ER_Pin,        /* Error pin. */
    .er_port = EF_LV_ER_GPIO_Port, /* GPIO port for Error pin. */
    .scale = SCALE(75),            /* Used to calculate current. */
    .rank = 3                      /* eFuse ADC channel rank. */
};

/* Radfan eFuse. */
const efuse_t ef_radfan = {
    .en_pin = EF_RADFAN_EN_Pin,        /* Enable pin. */
    .en_port = EF_RADFAN_EN_GPIO_Port, /* GPIO port for Enable pin. */
    .er_pin = EF_RADFAN_ER_Pin,        /* Error pin. */
    .er_port = EF_RADFAN_ER_GPIO_Port, /* GPIO port for Error pin. */
    .scale = SCALE(56),                /* Used to calculate current. */
    .rank = 4                          /* eFuse ADC channel rank. */
};

/* Fatbatt eFuse. */
const efuse_t ef_fanbatt = {
    .en_pin = EF_FANBATT_EN_Pin,          /* Enable pin. */
    .en_port = EF_FANBATT_EN_GPIO_Port,   /* GPIO port for Enable pin. */
    .er_pin = EF_FANBATT_ER_Pin,          /* Error pin. */
    .er_port = EF_FANBATT_ER_GPIO_Port,   /* GPIO port for Error pin. */
    .scale = SCALE(39),                   /* Used to calculate current. */
    .rank = 5                             /* eFuse ADC channel rank. */
};

/* Pump1 eFuse. */
const efuse_t ef_pump1 = {
    .en_pin = EF_PUMP1_EN_Pin,            /* Enable pin. */
    .en_port = EF_PUMP1_EN_GPIO_Port,     /* GPIO port for Enable pin. */
    .er_pin = EF_PUMP1_ER_Pin,            /* Error pin. */
    .er_port = EF_PUMP1_ER_GPIO_Port,     /* GPIO port for Error pin. */
    .scale = SCALE(47),                   /* Used to calculate current. */
    .rank = 6                             /* eFuse ADC channel rank. */
};

/* Pump2 eFuse. */
const efuse_t ef_pump2 = {
    .en_pin = EF_PUMP2_EN_Pin,        /* Enable pin. */
    .en_port = EF_PUMP2_EN_GPIO_Port, /* GPIO port for Enable pin. */
    .er_pin = EF_PUMP2_ER_Pin,        /* Error pin. */
    .er_port = EF_PUMP2_ER_GPIO_Port, /* GPIO port for Error pin. */
    .scale = SCALE(47),               /* Used to calculate current. */
    .rank = 7                         /* eFuse ADC channel rank. */
};

/* Battbox eFuse. */
const efuse_t ef_battbox = {
    .en_pin = EF_BATTBOX_EN_Pin,        /* Enable pin. */
    .en_port = EF_BATTBOX_EN_GPIO_Port, /* GPIO port for Enable pin. */
    .er_pin = EF_BATTBOX_ER_Pin,        /* Error pin. */
    .er_port = EF_BATTBOX_ER_GPIO_Port, /* GPIO port for Error pin. */
    .scale = SCALE(39),                 /* Used to calculate current. */
    .rank = 8                           /* eFuse ADC channel rank. */
};

/* MC eFuse. */
const efuse_t ef_mc = {
    .en_pin = EF_MC_EN_Pin,        /* Enable pin. */
    .en_port = EF_MC_EN_GPIO_Port, /* GPIO port for Enable pin. */
    .er_pin = EF_MC_ER_Pin,        /* Error pin. */
    .er_port = EF_MC_ER_GPIO_Port, /* GPIO port for Error pin. */
    .scale = SCALE(56),            /* Used to calculate current. */
    .rank = 9                      /* eFuse ADC channel rank. */
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
    return _buffer[efuse.rank];
}

#define V_REF 3.3f // V_(REF) = 3V3
/* Returns the eFuse's V_(IMON) voltage reading. */
float efuse_getVoltage(efuse_t efuse) {
    // V_(IMON) = (ADC_READING / 4095) * V_(REF)
    const uint16_t ADC_READING = _buffer[efuse.rank];
    return ((float)ADC_READING / 4095) * V_REF;
}

/* Returns the eFuse's I_(OUT) current reading. */
float efuse_getCurrent(efuse_t efuse) {
    // I_(OUT) = V_(IMON) / (GAIN_(IMON) * R_(IMON))
    const float v_imon = efuse_getVoltage(efuse);
    return v_imon * efuse.scale;
}