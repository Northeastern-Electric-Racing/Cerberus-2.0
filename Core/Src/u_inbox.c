#include <stdint.h>
#include "fdcan.h"
#include "u_tx_debug.h"
#include "u_ethernet.h"
#include "u_can.h"
#include "u_bms.h"

/* Processes received ethernet messages. */
void inbox_ethernet(ethernet_message_t *message) {
    switch(message->message_id) {
        case 0x01:
            // do thing
            break;
        case 0x02:
            // do thing
            break;
        case 0x03:
            // etc
            break;
        default:
            DEBUG_PRINTLN("ERROR: Unknown Ethernet Message Recieved (Message ID: %d).", message->message_id);
            break;
    }
}

/* Processes received CAN messages. */
void inbox_can(can_msg_t *message) {
    switch(message->id) {
        case CANID_BMS_DCL_MSG:
            bms_handleDclMessage();
            break;
        case CANID_BMS_CELL_TEMPS:
            uint16_t battbox_temp = ((message->data[6] << 8) | message->data[7]) / 100; //  Get "BMS/Cells/Temp_Avg_Value"
            bms_setBattboxTemp(battbox_temp);
            break;
        default:
            DEBUG_PRINTLN("ERROR: Unknown CAN Message Recieved (Message ID: %ld).", message->id);
            break;
    }
}