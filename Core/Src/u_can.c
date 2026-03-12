#include <stdint.h>
#include "u_can.h"
#include "u_tx_debug.h"
#include "u_nx_ethernet.h"
#include "u_bms.h"
#include "u_lightning.h"
#include "u_dti.h"
#include "can_messages_tx.h"
#include "can_messages_rx.h"

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
    uint16_t standard1[] = {CANID_BMS_DCL_MSG, CANID_BMS_CELL_TEMPS};
    status = can_add_filter_standard(&can1, standard1);
    if (status != HAL_OK) {
        PRINTLN_ERROR("Failed to add standard filter to can1 (Status: %d/%s, ID1: 0x%X, ID2: 0x%X).", status, hal_status_toString(status), standard1[0], standard1[1]);
        return U_ERROR;
    }

    /* Add filters for standard IDs */
    uint16_t standard2[] = {IMU_CAN_MSG_ID, DTI_CANID_TEMPS_FAULT};
    status = can_add_filter_standard(&can1, standard2);
    if (status != HAL_OK) {
        PRINTLN_ERROR("Failed to add standard filter to can1 (Status: %d/%s, ID1: 0x%X, ID2: 0x%X).", status, hal_status_toString(status), standard2[0], standard2[1]);
        return U_ERROR;
    }

    /* Add filters for standard IDs */
    uint16_t standard3[] = {DTI_CANID_ERPM, DTI_CANID_CURRENTS};
    status = can_add_filter_standard(&can1, standard3);
    if (status != HAL_OK) {
        PRINTLN_ERROR("Failed to add standard filter to can1 (Status: %d/%s, ID1: 0x%X, ID2: 0x%X).", status, hal_status_toString(status), standard3[0], standard3[1]);
        return U_ERROR;
    }

    /* Add fitlers for extended IDs */
    uint32_t extended1[] = {CANID_CALYPSO_EFCTRL_DASHBOARD, CANID_CALYPSO_EFCTRL_BRAKE};
    status = can_add_filter_extended(&can1, extended1);
    if (status != HAL_OK) {
        PRINTLN_ERROR("Failed to add extended filter to can1 (Status: %d/%s, ID1: %ld, ID2: %ld).", status, hal_status_toString(status), extended1[0], extended1[1]);
        return U_ERROR;
    }

    /* Add fitlers for extended IDs */
    uint32_t extended2[] = {CANID_CALYPSO_EFCTRL_SHUTDOWN, CANID_CALYPSO_EFCTRL_LV};
    status = can_add_filter_extended(&can1, extended2);
    if (status != HAL_OK) {
        PRINTLN_ERROR("Failed to add extended filter to can1 (Status: %d/%s, ID1: %ld, ID2: %ld).", status, hal_status_toString(status), extended2[0], extended2[1]);
        return U_ERROR;
    }

    /* Add fitlers for extended IDs */
    uint32_t extended3[] = {CANID_CALYPSO_EFCTRL_RADFAN, CANID_CALYPSO_EFCTRL_FANBATT};
    status = can_add_filter_extended(&can1, extended3);
    if (status != HAL_OK) {
        PRINTLN_ERROR("Failed to add extended filter to can1 (Status: %d/%s, ID1: %ld, ID2: %ld).", status, hal_status_toString(status), extended3[0], extended3[1]);
        return U_ERROR;
    }

    /* Add fitlers for extended IDs */
    uint32_t extended4[] = {CANID_CALYPSO_EFCTRL_PUMPONE, CANID_CALYPSO_EFCTRL_PUMPTWO};
    status = can_add_filter_extended(&can1, extended4);
    if (status != HAL_OK) {
        PRINTLN_ERROR("Failed to add extended filter to can1 (Status: %d/%s, ID1: %ld, ID2: %ld).", status, hal_status_toString(status), extended4[0], extended4[1]);
        return U_ERROR;
    }

    /* Add fitlers for extended IDs */
    uint32_t extended5[] = {CANID_CALYPSO_EFCTRL_BATTBOX, CANID_CALYPSO_EFCTRL_MC};
    status = can_add_filter_extended(&can1, extended5);
    if (status != HAL_OK) {
        PRINTLN_ERROR("Failed to add extended filter to can1 (Status: %d/%s, ID1: %ld, ID2: %ld).", status, hal_status_toString(status), extended5[0], extended5[1]);
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
        cell_temperatures_t temps = { 0 };
        receive_cell_temperatures(message, &temps);
        bms_setBattboxTemp(temps.avg_val); // "BMS/Cells/Temp_Avg_Value"
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
    case CANID_CALYPSO_EFCTRL_DASHBOARD:
        
        break;
    default:
        PRINTLN_ERROR("Unknown CAN Message Recieved (Message ID: %ld).", message->id);
        break;
    }
}
