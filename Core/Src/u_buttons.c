#include "u_buttons.h"
#include "u_tx_debug.h"

/* Processes incoming button presses. */
void buttons_process(button_t button) {
    switch(button) {
        case BUTTON_0:
            PRINTLN_INFO("Button 0 pressed.");
            break;
        case BUTTON_1:
            PRINTLN_INFO("Button 1 pressed.");
            break;
        case BUTTON_2:
            PRINTLN_INFO("Button 2 pressed.");
            break;
        case BUTTON_3:
            PRINTLN_INFO("Button 3 pressed.");
            break;
        case BUTTON_4:
            PRINTLN_INFO("Button 4 pressed.");
            break;
        case BUTTON_5:
            PRINTLN_INFO("Button 5 pressed.");
            break;
        case BUTTON_6:
            PRINTLN_INFO("Button 6 pressed.");
            break;
        case BUTTON_7:
            PRINTLN_INFO("Button 7 pressed.");
            break;
        case BUTTON_8:
            PRINTLN_INFO("Button 8 pressed.");
            break;
        case BUTTON_9:
            PRINTLN_INFO("Button 9 pressed.");
            break;
        default:
            break;
    }
}