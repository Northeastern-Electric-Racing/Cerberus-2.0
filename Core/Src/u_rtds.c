#include "tx_api.h"
#include "main.h"
#include "u_rtds.h"
#include "u_general.h"

/* Sounds the RTDS (Ready-to-drive sound). */
void rtds_soundRTDS(void) {
    HAL_GPIO_WritePin(RTDS_GPIO_GPIO_Port, RTDS_GPIO_Pin, GPIO_PIN_SET);
    tx_thread_sleep(RTDS_DURATION);
    HAL_GPIO_WritePin(RTDS_GPIO_GPIO_Port, RTDS_GPIO_Pin, GPIO_PIN_RESET);
    DEBUG_PRINTLN("sound_rtds() called.");
}

/* Reads the status of the RTDS pin (true = RTDS pin is active, false = RTDS pin is not active). */
/* Useful for debugging. */
bool rtds_readRTDS(void) {
    return (bool)(HAL_GPIO_ReadPin(RTDS_GPIO_GPIO_Port, RTDS_GPIO_Pin) == GPIO_PIN_SET);
}