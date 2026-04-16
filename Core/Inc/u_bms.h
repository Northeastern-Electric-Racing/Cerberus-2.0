#ifndef __U_BMS_H
#define __U_BMS_H

#include <stdint.h>

/* API */
int bms_init(void);                     // Initializes the BMS fault timer.
int bms_handleDclMessage(void);         // Restarts the BMS Fault Timer.
float bms_getBattboxTemp(void);      // Returns the battbox temperature.
void bms_setBattboxTemp(float temp); // Sets the battbox temperature. The "temp" parameter should be taken from the 'BMS/Cells/Temp_Avg_Value' CAN message.

void bms_receivePrechargeState(uint8_t precharge); 
bool bms_getPrecharge(void); 
#endif /* u_bms.h */