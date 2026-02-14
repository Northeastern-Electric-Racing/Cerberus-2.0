#ifndef __U_LIGHTNING_H
#define __U_LIGHTNING_H
#include <stdbool.h>

typedef enum {
    LIGHT_OFF = 0,
    LIGHT_GREEN = 1,
    LIGHT_RED = 2
} Lightning_Board_Light_Status;

/* Initializes the lightning fault timer. */
int lightning_init(void);

/* Restarts the lightning fault timer on status recv */
int lightning_handleIMUMessage(void);

/**
 * @brief sends the given light status to the LightningBoard
 *
 * @param status the desired light status (ex: LIGHT_OFF, LIGHT_RED, LIGHT_GREEN)
 */
void send_lightning_board_status(Lightning_Board_Light_Status msg);

/**
 * @brief initialises the lightning board (LIGHT_OFF at startup)

 */
void lightning_init(void);


#endif /* u_lightning.h */
