#include "u_lightning.h"
#include "u_queues.h"
#include "u_can.h"

void send_lightning_board_light_status(Lightning_Board_Light_Status status) {
    can_msg_t can_msg = { .id = LIGHT_BOARD_CAN_MSG_ID, .len = 2, .data = { 0 } };
    can_msg.data[0] = status;
    queue_send(&can_outgoing, &can_msg);
}