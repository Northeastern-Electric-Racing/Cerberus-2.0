#ifndef __U_BMS_H
#define __U_BMS_H

#include <stdint.h>

/* API */
int bms_init(void);                       // Initializes the BMS fault timer.
int bms_handleDclMessage(void);           // Restarts the BMS Fault Timer.
int bms_getBattboxTemp(uint16_t* buffer); // Returns the battbox temperature.
int bms_setBattboxTemp(uint16_t temp);    // Sets the battbox temperature. The "temp" parameter should be taken from the 'BMS/Cells/Temp_Avg_Value' CAN message.

#endif /* u_bms.h */