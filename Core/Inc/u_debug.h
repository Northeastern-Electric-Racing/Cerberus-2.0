#pragma once

#include <stdbool.h>

/* This file includes stuff related to the VCU's debugging peripherals (user button, LEDs). */

/* API */
void debug_enableRedLED(void);        // Turns on the Red Debug LED.
void debug_disableRedLED(void);       // Turns off the Red Debug LED.
void debug_enableGreenLED(void);      // Turns on the Green Debug LED.
void debug_disableGreenLED(void);     // Turns off the Green Debug LED.
void debug_onUserButtonPressed(void); // Called when the user button is pressed.