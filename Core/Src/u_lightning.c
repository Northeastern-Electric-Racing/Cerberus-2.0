#include "u_lightning.h"
#include "can_messages_tx.h"


void send_lightning_board_status(Lightning_Board_Light_Status status) {
    send_lightning_board_light_status(status);
}

void lightning_init(void) {
    send_lightning_board_light_status(LIGHT_OFF);
}
void update_lightning_board_status(bool bms_gpio, bool imd_gpio) {
    Lightning_Board_Light_Status status; // to create the status variable

    if (bms_gpio || imd_gpio) // if either of them are faulted
        {
            status = LIGHT_RED;
        }
        else
        {
            status = LIGHT_GREEN; 
        }
        send_lightning_board_status(status);
}

