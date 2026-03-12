#include <stdint.h>
#include "u_can.h"
#include "u_tx_debug.h"
#include "u_nx_ethernet.h"
#include "u_bms.h"
#include "u_lightning.h"

/* CAN interfaces */
can_t can1;
// add more as necessary...

uint8_t can1_init(FDCAN_HandleTypeDef *hcan) {
    /* Init CAN interface */
    HAL_StatusTypeDef status = can_init(&can1, hcan);
    if (status != HAL_OK) {
        PRINTLN_ERROR("Failed to execute can_init() when initializing can1 (Status: %d/%s).", status, hal_status_toString(status));
        return U_ERROR;
    }

    /* Add filters for standard IDs */
    uint16_t standard[] = {CANID_BMS_DCL_MSG, CANID_BMS_CELL_TEMPS};
    status = can_add_filter_standard(&can1, standard);
    if (status != HAL_OK) {
        PRINTLN_ERROR("Failed to add standard filter to can1 (Status: %d/%s, ID1: 0x%X, ID2: 0x%X).", status, hal_status_toString(status), standard[0], standard[1]);
        return U_ERROR;
    }

    /* Add filters for standard IDs */
    standard[] = {IMU_CAN_MSG_ID, DTI_CANID_TEMPS_FAULT};
    status = can_add_filter_standard(&can1, standard);
    if (status != HAL_OK) {
        PRINTLN_ERROR("Failed to add standard filter to can1 (Status: %d/%s, ID1: 0x%X, ID2: 0x%X).", status, hal_status_toString(status), standard[0], standard[1]);
        return U_ERROR;
    }

    /* Add filters for standard IDs */
    standard[] = {DTI_CANID_ERPM, DTI_CANID_CURRENTS};
    status = can_add_filter_standard(&can1, standard);
    if (status != HAL_OK) {
        PRINTLN_ERROR("Failed to add standard filter to can1 (Status: %d/%s, ID1: 0x%X, ID2: 0x%X).", status, hal_status_toString(status), standard[0], standard[1]);
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
    switch (message->id) {
    case CANID_BMS_DCL_MSG:
        bms_handleDclMessage();
        break;
    case CANID_BMS_CELL_TEMPS:
        uint16_t battbox_temp = ((message->data[6] << 8) | message->data[7]) / 100; //  Get "BMS/Cells/Temp_Avg_Value"
        bms_setBattboxTemp(battbox_temp);
        break;
    case IMU_CAN_MSG_ID:
        lightning_handleIMUMessage();
        break;
    case DTI_CANID_TEMPS_FAULT:
        dti_record_temp(message);
        break;
    case DTI_CANID_ERPM:
        dti_record_rpm(message);
        break;
    case DTI_CANID_CURRENTS:
        dti_record_currents(message);
        break;
    default:
        PRINTLN_ERROR("Unknown CAN Message Recieved (Message ID: %ld).", message->id);
        break;
    }
}
