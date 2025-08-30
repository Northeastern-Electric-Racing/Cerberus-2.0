#ifndef __U_EFUSES_H
#define __U_EFUSES_H

#include <stdint.h>
#include "main.h"

typedef enum {

    /* eFuse List. */
    /* Note: this list should always be ordered by the eFuses' ADC ranks. These are defined in CubeMX. */
    EFUSE_DASH,
    EFUSE_BREAK,
    EFUSE_SHUTDOWN,
    EFUSE_LV,
    EFUSE_RADFAN,
    EFUSE_FANBATT,
    EFUSE_PUMP1,
    EFUSE_PUMP2,
    EFUSE_BATTBOX,
    EFUSE_MC,

    /* Total number of eFuses. */
    /* (always keep this at the end) */
    NUM_EFUSES

} efuse_t;

/* API */
uint8_t efuses_init(void); // Start eFuse-related ADC DMA.
uint16_t efuse_getRaw(efuse_t efuse); // Returns the eFuse's raw ADC reading.

#endif /* u_efuses.h */