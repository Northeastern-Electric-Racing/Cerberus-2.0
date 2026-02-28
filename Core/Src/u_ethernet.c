#include "u_ethernet.h"

/* Callback for when a ethernet message is recieved. */
void _ethernet_recieve(ethernet_message_t message) {
  /* Send the message to the incoming ethernet queue. */
  int status = queue_send(&eth_incoming, &message, TX_NO_WAIT);
  if(status != U_SUCCESS) {
    PRINTLN_ERROR("Failed to send message to the incoming ethernet queue (Status: %d).", status);
    return;
  }
}

/* Initializes ethernet. */
int ethernet1_init(void) {
    /* PHY_RESET Pin has to be set HIGH for the PHY to function. */
    HAL_GPIO_WritePin(PHY_RESET_GPIO_Port, PHY_RESET_Pin, GPIO_PIN_SET);

    /* Init the ethernet. */
    return ethernet_init(VCU, nx_stm32_eth_driver, _ethernet_recieve);
}

/* Processes received ethernet messages. */
void ethernet_inbox(ethernet_message_t *message) {
    switch(message->message_id) {
        case 5:
            PRINTLN_INFO("Incoming Ethernet message: %s", message->data);
            break;
        case 0x02:
            // do thing
            break;
        case 0x03:
            // etc
            break;
        default:
            uint8_t buff = 0;
            buff = message->data[0];
            PRINTLN_ERROR("Unknown Ethernet Message Recieved (Message ID: %d).", message->message_id);
            
            PRINTLN_INFO("sender_id=%d, recipient_id=%d, message_id=%d, data_length=%d, data=%d", message->sender_id, message->recipient_id, message->message_id, message->data_length, message->data[0]);

            break;
    }
}
