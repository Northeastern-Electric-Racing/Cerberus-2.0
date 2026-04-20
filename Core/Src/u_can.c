#include <stdint.h>
#include "u_can.h"
#include "u_tx_debug.h"
#include "u_nx_ethernet.h"
#include "u_bms.h"
#include "u_lightning.h"
#include "u_tc.h"
#include "u_rtds.h"
#include "u_buttons.h"
#include "u_statemachine.h"
#include "u_dti.h"
#include "u_efuses.h"
#include "serial.h"
#include "u_shutdown.h"
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
    uint16_t standard2[] = {0x00, DTI_CANID_TEMPS_FAULT};
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

    /* Add filters for standard IDs */
    uint16_t standard4[] = {CANID_SHEPHERD_PRECHARGE, CANID_WHEEL_BUTTONS};
    status = can_add_filter_standard(&can1, standard4);
    if (status != HAL_OK) {
        PRINTLN_ERROR("Failed to add standard filter to can1 (Status: %d/%s, ID1: 0x%X, ID2: 0x%X).", status, hal_status_toString(status), standard4[0], standard4[1]);
        return U_ERROR;
    }

    /* Add filters for standard IDs */
    uint16_t standard5[] = {CANID_SHUTDOWN, 0x00};
    status = can_add_filter_standard(&can1, standard5);
    if (status != HAL_OK) {
        PRINTLN_ERROR("Failed to add standard filter to can1 (Status: %d/%s, ID1: 0x%X, ID2: 0x%X).", status, hal_status_toString(status), standard5[0], standard5[1]);
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

    /* Add fitlers for extended IDs */
    uint32_t extended6[] = {CANID_CALYPSO_EFCTRL_SPARE, CANID_CALYPSO_RTDS_STATE};
    status = can_add_filter_extended(&can1, extended6);
    if (status != HAL_OK) {
        PRINTLN_ERROR("Failed to add extended filter to can1 (Status: %d/%s, ID1: %ld, ID2: %ld).", status, hal_status_toString(status), extended6[0], extended6[1]);
        return U_ERROR;
    }

    /* Add fitlers for extended IDs */
    uint32_t extended7[] = {CANID_LIGHTNING_PULSE, 0x00};
    status = can_add_filter_extended(&can1, extended7);
    if (status != HAL_OK) {
        PRINTLN_ERROR("Failed to add extended filter to can1 (Status: %d/%s, ID1: %ld, ID2: %ld).", status, hal_status_toString(status), extended7[0], extended7[1]);
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
    case CANID_LIGHTNING_PULSE:
        PRINTLN_INFO("lightning - received the IMU message");
        lightning_handleIMUMessage();
        break;
    case CANID_F_RPM:
        tc_record_front_rpm(*message);
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
        dashboard_efuse_state_t dashboard = { 0 };
        receive_dashboard_efuse_state(message, &dashboard);
        efuse_update_state(EFUSE_DASHBOARD, (efuse_control_state_t)dashboard.state);
        break;
    case CANID_CALYPSO_EFCTRL_BRAKE:
        brake_efuse_state_t brake = { 0 };
        receive_brake_efuse_state(message, &brake);
        efuse_update_state(EFUSE_BRAKE, (efuse_control_state_t)brake.state);
        break;
    case CANID_CALYPSO_EFCTRL_SHUTDOWN:
        shutdown_efuse_state_t shutdown = { 0 };
        receive_shutdown_efuse_state(message, &shutdown);
        efuse_update_state(EFUSE_SHUTDOWN, (efuse_control_state_t)shutdown.state);
        break;
    case CANID_CALYPSO_EFCTRL_LV:
        lv_efuse_state_t lv = { 0 };
        receive_lv_efuse_state(message, &lv);
        efuse_update_state(EFUSE_LV, (efuse_control_state_t)lv.state);
        break;
    case CANID_CALYPSO_EFCTRL_RADFAN:
        radfan_efuse_state_t radfan = { 0 };
        receive_radfan_efuse_state(message, &radfan);
        efuse_update_state(EFUSE_RADFAN, (efuse_control_state_t)radfan.state);
        break;
    case CANID_CALYPSO_EFCTRL_FANBATT:
        fanbatt_efuse_state_t fanbatt = { 0 };
        receive_fanbatt_efuse_state(message, &fanbatt);
        efuse_update_state(EFUSE_FANBATT, (efuse_control_state_t)fanbatt.state);
        break;
    case CANID_CALYPSO_EFCTRL_PUMPONE:
        pumpone_efuse_state_t pumpone = { 0 };
        receive_pumpone_efuse_state(message, &pumpone);
        efuse_update_state(EFUSE_PUMP1, (efuse_control_state_t)pumpone.state);
        break;
    case CANID_CALYPSO_EFCTRL_PUMPTWO:
        pumptwo_efuse_state_t pumptwo = { 0 };
        receive_pumptwo_efuse_state(message, &pumptwo);
        efuse_update_state(EFUSE_PUMP2, (efuse_control_state_t)pumptwo.state);
        break;
    case CANID_CALYPSO_EFCTRL_BATTBOX:
        battbox_efuse_state_t battbox = { 0 };
        receive_battbox_efuse_state(message, &battbox);
        efuse_update_state(EFUSE_BATTBOX, (efuse_control_state_t)battbox.state);
        break;
    case CANID_CALYPSO_EFCTRL_MC:
        mc_efuse_state_t mc = { 0 };
        receive_mc_efuse_state(message, &mc);
        efuse_update_state(EFUSE_MC, (efuse_control_state_t)mc.state);
        break;
    case CANID_SHEPHERD_PRECHARGE: 
        bms_receivePrechargeState(message->data[0]); //first byte of the can mssg data
        break;
    case CANID_CALYPSO_EFCTRL_SPARE:
        spare_efuse_state_t spare = { 0 };
        receive_spare_efuse_state(message, &spare);
        efuse_update_state(EFUSE_SPARE, (efuse_control_state_t)spare.state);
        break;
    case CANID_CALYPSO_RTDS_STATE:
        /* 0 = Sound RTDS. 1 = Cancel RTDS. 2 = Start Reverse, 3 = Stop Reverse */
        enum {
            SOUND_RTDS = 0,
            CANCEL_RTDS = 1,
            START_REVERSE = 2,
            STOP_REVERSE = 3
        };

        rtds_command_message_t commands = { 0 };
        receive_rtds_command_message(message, &commands);
        switch(commands.command) {
            case SOUND_RTDS: rtds_soundRTDS(); break;
            case CANCEL_RTDS: rtds_cancelRTDS(); break;
            case START_REVERSE: rtds_startReverseSound(); break;
            case STOP_REVERSE: rtds_stopReverseSound(); break;
            default: break;
        }
        break;
    case CANID_WHEEL_BUTTONS:
        wheel_buttons_t wheel_buttons = { 0 };
        receive_wheel_buttons(message, &wheel_buttons);
        buttons_process((button_t)wheel_buttons.button_id);
        break;
    case CANID_SHUTDOWN:
        shutdown_as_read_by_bms_t bms = { 0 };
        receive_shutdown_as_read_by_bms(message, &bms);
        update_bms_shutdown(bms.shutdown);

        /* If shutdown is active, cancel the RTDS sound if it's active. */
        if(bms.shutdown == true) {
            rtds_cancelRTDS();
            rtds_stopReverseSound();
        }
        break;
    default:
        PRINTLN_WARNING("Unknown CAN Message Recieved (Message ID: 0x%X).", message->id);
        break;

    }
}
