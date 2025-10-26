#ifndef __U_LIGHTNING_H
#define __U_LIGHTNING_H

#define LIGHT_BOARD_CAN_MSG_ID 0xCA

typedef enum {
    LIGHT_OFF = 0,
    LIGHT_GREEN = 1,
    LIGHT_RED = 2
} Lightning_Board_Light_Status;

/**
 * @brief sends the given light status to the LightningBoard
 *
 * @param status the desired light status (ex: LIGHT_OFF, LIGHT_RED, LIGHT_GREEN)
 */
void send_lightning_board_light_status(Lightning_Board_Light_Status msg);

#endif /* u_lightning.h */