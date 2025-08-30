#ifndef __U_EFUSES_H
#define __U_EFUSES_H

#include <stdint.h>
#include <stdbool.h>
#include "main.h"

typedef struct {
    int en_pin;             /* EN (Enable) pin for this eFuse. It enables/disables the eFuse. */
    GPIO_TypeDef* en_port;  /* GPIO port for the EN pin. */
    int er_pin;             /* ER (Error) pin for this eFuse. It indicates if the eFuse is experiencing an error. */
    GPIO_TypeDef* er_port;  /* GPIO port for the ER pin. */
    float scale;            /* scale = 1 / (GAIN_IMON * R_IMON). Used to calculate current. */
    int rank;               /* Rank of the eFuse's ADC channel. Corresponds to the eFuse's index in the ADC buffer. */
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
uint8_t efuses_init(void);                 // Start eFuse-related ADC DMA.
uint16_t efuse_getRaw(efuse_t efuse);      // Returns the eFuse's raw ADC reading.
float efuse_getVoltage(efuse_t efuse);     // Returns the eFuse's voltage reading.
float efuse_getCurrent(efuse_t efuse);     // Returns the eFuse's current reading.
bool efuse_getFaultStatus(efuse_t efuse);  // Returns the eFuse's fault status (true = faulted, false = not faulted).
void efuse_enable(efuse_t efuse);          // Enables an eFuse.
void efuse_disable(efuse_t efuse);         // Disables an eFuse.
bool efuse_getEnableStatus(efuse_t efuse); // Returns whether or not the eFuse is enabled (true = eFuse is enabled, false = eFuse is disabled).

#endif /* u_efuses.h */