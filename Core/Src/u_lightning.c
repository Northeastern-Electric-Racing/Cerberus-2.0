#include "u_lightning.h"
#include "u_queues.h"
#include "u_can.h"

/**
 * @brief sends the given light status to the LightningBoard
 *
 * @param status the desired light status (ex: LIGHT_OFF, LIGHT_RED, LIGHT_GREEN)
 */
void send_lightning_board_light_status(Lightning_Board_Light_Status status) {
    can_msg_t can_msg = { .id = 0x036, .len = 2, .data = { 0 } };
    can_msg.data[0] = status;
    queue_send(&can_outgoing, &can_msg);
}