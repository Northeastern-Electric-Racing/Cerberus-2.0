#include "u_lightning.h"
#include "can_messages_tx.h"

void send_lightning_board_status(Lightning_Board_Light_Status status) {
    send_lightning_board_light_status(status);
}
