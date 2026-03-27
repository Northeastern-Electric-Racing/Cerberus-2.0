#pragma once

/* Enum for each button. */
typedef enum {
    BUTTON_ESC,
    BUTTON_LEFT,
    BUTTON_LAUNCH_CONTROL_OFF,
    BUTTON_UP_REGEN,
    BUTTON_DOWN_REGEN,
    BUTTON_ENTER,
    BUTTON_RIGHT,
    BUTTON_LAUNCH_CONTROL_ON,
    BUTTON_UP_TORQUE,
    BUTTON_DOWN_TORQUE
} button_t;

/* Processes incoming buttonp presses. */
void buttons_process(button_t button);