#ifndef __U_RTDS_H
#define __U_RTDS_H

#include <stdbool.h>

/* Config: */
#define RTDS_DURATION 5000 /* RTDS duration (in ticks). */

/* API */
int rtds_init(void);               // Initializes the RTDS timer.
int rtds_soundRTDS(void);          // Sounds the RTDS (Ready-to-drive sound).
int rtds_startReverseSound(void);  // Starts the reverse sound (periodic beeping).
int rtds_stopReverseSound(void);   // Stops the reverse sound.
bool rtds_readRTDS(void);          // Reads the status of the RTDS pin (true = RTDS pin is active, false = RTDS pin is not active). Useful for debugging.

#endif /* u_rtds.h */