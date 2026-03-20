#ifndef __U_RTDS_H
#define __U_RTDS_H

#include <stdbool.h>

/* Config: */
#define RTDS_DURATION 5000 /* RTDS duration (in ticks). */

/* API */
int rtds_init(void);                // Initializes the RTDS timer.
int rtds_soundRTDS(void);           // Sounds the RTDS (Ready-to-drive sound).
int rtds_startReverseSound(void);   // Starts the reverse sound (periodic beeping).
int rtds_stopReverseSound(void);    // Stops the reverse sound.
bool rtds_readRTDS(void);           // Reads the status of the RTDS pin (true = RTDS pin is active, false = RTDS pin is not active). Useful for debugging.
int rtds_isInReverse(bool* state);  // Checks if the RTDS is currently in reverse mode.
int rtds_isInSounding(bool* state); // Checks if the RTDS is currently in sounding mode.
int rtds_cancelRTDS(void);          // Cancels the RTDS sound. The RTDS sound already stops automatically once it's over, but this function lets you cancel it prematurely.

#endif /* u_rtds.h */