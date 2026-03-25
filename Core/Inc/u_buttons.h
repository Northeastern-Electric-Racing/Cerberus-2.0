#pragma once

/* Enum for each button. */
typedef enum {
    BUTTON_0,
    BUTTON_1,
    BUTTON_2,
    BUTTON_3,
    BUTTON_4,
    BUTTON_5,
    BUTTON_6,
    BUTTON_7,
    BUTTON_8,
    BUTTON_9
} button_t;

/* Processes incoming buttonp presses. */
void buttons_process(button_t button);