#ifndef __U_EFUSES_H
#define __U_EFUSES_H

#include <stdint.h>
#include "main.h"

typedef struct {

    /* General Config */
    const char* name;             /* Name of the eFuse. Used for debugging. */

    /* Enable Pin & Port */
    const int en_pin;             /* EN (Enable) pin for this eFuse. It enables/disables the eFuse. */
    const GPIO_TypeDef* en_port;  /* GPIO port for the EN pin. */

    /* Error Pin & Port */
    const int er_pin;             /* ER (Error) pin for this eFuse. It indicates if the eFuse is experiencing an error. */
    const GPIO_TypeDef* er_port;  /* GPIO port for the ER pin. */

    /* ADC Stuff */
    const int rank; /* Rank of the eFuse's ADC channel. Corresponds to the eFuse's index in the ADC buffer. */

} efuse_t;

/* eFuse List: */
extern const efuse_t ef_dashboard; // Dashboard eFuse
extern const efuse_t ef_break;     // Break eFuse
extern const efuse_t ef_shutdown;  // Shutdown eFuse
extern const efuse_t ef_lv;        // LV eFuse
extern const efuse_t ef_radfan;    // Radfan eFuse
extern const efuse_t ef_fanbatt;   // Fanbatt eFuse
extern const efuse_t ef_pump1;     // Pump1 eFuse
extern const efuse_t ef_pump2;     // Pump2 eFuse
extern const efuse_t ef_battbox;   // Battbox eFuse
extern const efuse_t ef_mc;        // MC eFuse

/* API */
uint8_t efuses_init(void);

#endif /* u_efuses.h */