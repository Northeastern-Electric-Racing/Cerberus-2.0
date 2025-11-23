#include <stdint.h>
#include "u_can.h"
#include "u_tx_debug.h"
#include "u_nx_ethernet.h"
#include "u_bms.h"


/* CAN interfaces */
can_t can1;
// add more as necessary...

uint8_t can1_init(FDCAN_HandleTypeDef *hcan) {
    
    /* Init CAN interface */
    HAL_StatusTypeDef status = can_init(&can1, hcan);
    if(status != HAL_OK) {
        PRINTLN_ERROR("Failed to execute can_init() when initializing can1 (Status: %d/%s).", status, hal_status_toString(status));
        return U_ERROR;
    }

    /* Add filters for standard IDs */
    uint16_t standard[] = {0x00, 0x00};
    status = can_add_filter_standard(&can1, standard);
    if(status != HAL_OK) {
        PRINTLN_ERROR("Failed to add standard filter to can1 (Status: %d/%s, ID1: %d, ID2: %d).", status, hal_status_toString(status), standard[0], standard[1]);
        return U_ERROR;
    }

    /* Add fitlers for extended IDs */
    uint32_t extended[] = {0x00, 0x00};
    status = can_add_filter_extended(&can1, extended);
    if (status != HAL_OK) {
        PRINTLN_ERROR("Failed to add extended filter to can1 (Status: %d/%s, ID1: %ld, ID2: %ld).", status, hal_status_toString(status), extended[0], extended[1]);
        return U_ERROR;
    }

    PRINTLN_INFO("Ran can1_init().");

    return U_SUCCESS;
}

/* Processes received CAN messages. */
void can_inbox(can_msg_t *message) {
    switch(message->id) {
        case CANID_BMS_DCL_MSG:
            bms_handleDclMessage();
            break;
        case CANID_BMS_CELL_TEMPS:
            uint16_t battbox_temp = ((message->data[6] << 8) | message->data[7]) / 100; //  Get "BMS/Cells/Temp_Avg_Value"
            bms_setBattboxTemp(battbox_temp);
            break;
        default:
            PRINTLN_ERROR("Unknown CAN Message Recieved (Message ID: %ld).", message->id);
            break;
    }
}