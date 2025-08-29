#include <stdint.h>
#include "u_can.h"
#include "u_general.h"
#include "stm32h5xx_hal.h"


/* CAN interfaces */
can_t can1;
// add more as necessary...

uint8_t can1_init(FDCAN_HandleTypeDef *hcan) {
    
    /* Init CAN interface */
    HAL_StatusTypeDef status = can_init(&can1, hcan);
    if(status != HAL_OK) {
        DEBUG_PRINTLN("Failed to execute can_init() when initializing can1 (Status: %s).", hal_status_toString(status));
        return U_ERROR;
    }

    /* Add filters for standard IDs */
    uint16_t standard[] = {0x00, 0x00};
    status = can_add_filter_standard(&can1, standard);
    if(status != HAL_OK) {
        DEBUG_PRINTLN("Failed to add standard filter to can1 (Status: %s, ID1: %d, ID2: %d).", hal_status_toString(status), standard[0], standard[1]);
        return U_ERROR;
    }

    /* Add fitlers for extended IDs */
    uint32_t extended[] = {0x00, 0x00};
    status = can_add_filter_extended(&can1, extended);
    if (status != HAL_OK) {
        DEBUG_PRINTLN("Failed to add extended filter to can1 (Status: %s, ID1: %ld, ID2: %ld).", hal_status_toString(status), extended[0], extended[1]);
        return U_ERROR;
    }

    DEBUG_PRINTLN("Ran can1_init().");

    return U_SUCCESS;
}