#include "main.h"
#include "u_debug.h"
#include "u_tx_debug.h"
#include "serial.h"

/* This file includes stuff related to the VCU's debugging peripherals (user button, LEDs). */

/* Turns on the Red Debug LED. */
void debug_enableRedLED(void) {
    HAL_GPIO_WritePin(RED_LED_GPIO_Port, RED_LED_Pin, GPIO_PIN_SET);
    PRINTLN_INFO("Enabled Red Debug LED.");
}

/* Turns off the Red Debug LED. */
void debug_disableRedLED(void) {
    HAL_GPIO_WritePin(RED_LED_GPIO_Port, RED_LED_Pin, GPIO_PIN_RESET);
    PRINTLN_INFO("Disabled Red Debug LED.");
}

/* Turns on the Green Debug LED. */
void debug_enableGreenLED(void) {
    HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin, GPIO_PIN_SET);
    PRINTLN_INFO("Enabled Green Debug LED.");
}

/* Turns off the Green Debug LED. */
void debug_disableGreenLED(void) {
    HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin, GPIO_PIN_RESET);
    PRINTLN_INFO("Disabled Green Debug LED.");
}

/* Called when the user button is pressed. */
void debug_onUserButtonPressed(void) {
    static int times_pressed = 0;
    times_pressed++;
    PRINTLN_INFO("User button pressed.");
    serial_monitor("user_button", "times_pressed", "%d", times_pressed);
}