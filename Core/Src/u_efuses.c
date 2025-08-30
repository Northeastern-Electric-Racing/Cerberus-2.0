#include "u_efuses.h"
#include "u_general.h"

static uint32_t buffer[10]; /* Buffer to hold eFuse ADC readings. Each index corresponds to a different eFuse. */

/* Dashboard eFuse. */
const efuse_t ef_dashboard = {
    .name = "Dashboard eFuse (EF_DASH)", /* eFuse Name. */
    .en_pin = EF_DASH_EN_Pin,            /* Enable pin. */
    .en_port = EF_DASH_EN_GPIO_Port,     /* GPIO port for Enable pin. */
    .er_pin = EF_DASH_ER_Pin,            /* Error pin. */
    .er_port = EF_DASH_ER_GPIO_Port,     /* GPIO port for Error pin. */
    .rank = 0                            /* eFuse ADC channel rank. */
};

/* Break eFuse. */
const efuse_t ef_break = {
    .name = "Break eFuse (EF_BREAK)", /* eFuse Name. */
    .en_pin = EF_BREAK_EN_Pin,        /* Enable pin. */
    .en_port = EF_BREAK_EN_GPIO_Port, /* GPIO port for Enable pin. */
    .er_pin = EF_BREAK_ER_Pin,        /* Error pin. */
    .er_port = EF_BREAK_ER_GPIO_Port, /* GPIO port for Error pin. */
    .rank = 1                         /* eFuse ADC channel rank. */
};

/* Shutdown eFuse. */
const efuse_t ef_shutdown = {
    .name = "Shutdown eFuse (EF_SHUTDOWN)", /* eFuse Name. */
    .en_pin = EF_SHUTDOWN_EN_Pin,           /* Enable pin. */
    .en_port = EF_SHUTDOWN_EN_GPIO_Port,    /* GPIO port for Enable pin. */
    .er_pin = EF_SHUTDOWN_ER_Pin,           /* Error pin. */
    .er_port = EF_SHUTDOWN_ER_GPIO_Port,    /* GPIO port for Error pin. */
    .rank = 2                               /* eFuse ADC channel rank. */
};

/* LV eFuse. */
const efuse_t ef_lv = {
    .name = "LV eFuse (EF_LV)",    /* eFuse Name. */
    .en_pin = EF_LV_EN_Pin,        /* Enable pin. */
    .en_port = EF_LV_EN_GPIO_Port, /* GPIO port for Enable pin. */
    .er_pin = EF_LV_ER_Pin,        /* Error pin. */
    .er_port = EF_LV_ER_GPIO_Port, /* GPIO port for Error pin. */
    .rank = 3                      /* eFuse ADC channel rank. */
};

/* Radfan eFuse. */
const efuse_t ef_radfan = {
    .name = "Radfan eFuse (EF_RADFAN)", /* eFuse Name. */
    .en_pin = EF_RADFAN_EN_Pin,         /* Enable pin. */
    .en_port = EF_RADFAN_EN_GPIO_Port,  /* GPIO port for Enable pin. */
    .er_pin = EF_RADFAN_ER_Pin,         /* Error pin. */
    .er_port = EF_RADFAN_ER_GPIO_Port,  /* GPIO port for Error pin. */
    .rank = 4                           /* eFuse ADC channel rank. */
};

/* Fatbatt eFuse. */
const efuse_t ef_fanbatt = {
    .name = "Fanbatt eFuse (EF_FANBATT)", /* eFuse Name. */
    .en_pin = EF_FANBATT_EN_Pin,          /* Enable pin. */
    .en_port = EF_FANBATT_EN_GPIO_Port,   /* GPIO port for Enable pin. */
    .er_pin = EF_FANBATT_ER_Pin,          /* Error pin. */
    .er_port = EF_FANBATT_ER_GPIO_Port,   /* GPIO port for Error pin. */
    .rank = 5                             /* eFuse ADC channel rank. */
};

/* Pump1 eFuse. */
const efuse_t ef_pump1 = {
    .name = "Pump1 eFuse (EF_PUMP1)", /* eFuse Name. */
    .en_pin = EF_PUMP1_EN_Pin,        /* Enable pin. */
    .en_port = EF_PUMP1_EN_GPIO_Port, /* GPIO port for Enable pin. */
    .er_pin = EF_PUMP1_ER_Pin,        /* Error pin. */
    .er_port = EF_PUMP1_ER_GPIO_Port, /* GPIO port for Error pin. */
    .rank = 6                         /* eFuse ADC channel rank. */
};

/* Pump2 eFuse. */
const efuse_t ef_pump2 = {
    .name = "Pump2 eFuse (EF_PUMP2)", /* eFuse Name. */
    .en_pin = EF_PUMP2_EN_Pin,        /* Enable pin. */
    .en_port = EF_PUMP2_EN_GPIO_Port, /* GPIO port for Enable pin. */
    .er_pin = EF_PUMP2_ER_Pin,        /* Error pin. */
    .er_port = EF_PUMP2_ER_GPIO_Port, /* GPIO port for Error pin. */
    .rank = 7                         /* eFuse ADC channel rank. */
};

/* Battbox eFuse. */
const efuse_t ef_battbox = {
    .name = "Battbox eFuse (EF_BATTBOX)", /* eFuse Name. */
    .en_pin = EF_BATTBOX_EN_Pin,          /* Enable pin. */
    .en_port = EF_BATTBOX_EN_GPIO_Port,   /* GPIO port for Enable pin. */
    .er_pin = EF_BATTBOX_ER_Pin,          /* Error pin. */
    .er_port = EF_BATTBOX_ER_GPIO_Port,   /* GPIO port for Error pin. */
    .rank = 8                             /* eFuse ADC channel rank. */
};

/* MC eFuse. */
const efuse_t ef_mc = {
    .name = "MC eFuse (EF_MC)",    /* eFuse Name. */
    .en_pin = EF_MC_EN_Pin,        /* Enable pin. */
    .en_port = EF_MC_EN_GPIO_Port, /* GPIO port for Enable pin. */
    .er_pin = EF_MC_ER_Pin,        /* Error pin. */
    .er_port = EF_MC_ER_GPIO_Port, /* GPIO port for Error pin. */
    .rank = 9                      /* eFuse ADC channel rank. */
};

uint8_t efuses_init(void) {

    /* Start ADC DMA */
    HAL_StatusTypeDef status = HAL_ADC_Start_DMA(&hadc1, buffer, 10);
    if(status != HAL_OK) {
        DEBUG_PRINTLN("ERROR: Failed to start ADC DMA for eFuses (Status: %d/%s).", status, hal_status_toString(status));
        return U_ERROR;
    }

    return U_SUCCESS;
}