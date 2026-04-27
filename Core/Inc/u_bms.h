#ifndef __U_BMS_H
#define __U_BMS_H

#include <stdint.h>
#include "can_messages_rx.h"    

/* API */

/* Precharge States from BMS */
typedef enum {
    PRECHARGE_OPEN = 0, // Precharge is open and inactive
    PRECHARGE_FLOATING = 1, // Precharge is floating below activation threshold
    PRECHARGE_CLOSED = 2, // Precharge is closed and active
} precharge_state_t;

int bms_init(void);                     // Initializes the BMS fault timer.
int bms_handleDclMessage(void);         // Restarts the BMS Fault Timer.
float bms_getBattboxTemp(void);      // Returns the battbox temperature.
void bms_setBattboxTemp(float temp); // Sets the battbox temperature. The "temp" parameter should be taken from the 'BMS/Cells/Temp_Avg_Value' CAN message.

void bms_receivePrechargeState(precharge_state_t precharge); 
bool bms_getPrecharge(void); 

int bms_handleBmsFaultMessage(can_msg_t *message); // CAN faults upon receiving a latching fault message from BMS
int imd_handleImdFaultMessage(can_msg_t *message); // CAN faults upon receiving a latctching fault message from IMD


#endif /* u_bms.h */