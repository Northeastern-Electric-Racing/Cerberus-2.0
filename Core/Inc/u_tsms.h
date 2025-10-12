#pragma once

#include <stdbool.h>

/* API */
int tsms_init(void);    /* Init TSMS Debounce Timer. */
void tsms_update(void); /* Checks the state of the TSMS pin and updates the 'tsms' bool accordingly. Handles all the debounding stuff. Should only be called by the TSMS thread. */
bool tsms_get(void);    /* Gets the Offical TSMS State (not the raw pin state, but the state stored in the debounced 'tsms' bool). */