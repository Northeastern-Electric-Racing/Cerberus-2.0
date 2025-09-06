#ifndef __U_PEDALS_H
#define __U_PEDALS_H

/* API */
int pedals_init(void);           // Initializes Pedals ADC and creates pedal data timer.
void pedals_process(void);       // Pedal Processing Function. Meant to be called by the pedals thread.
bool pedals_getBrakeState(void); // Returns the brake state (true=brake pressed, false=brake not pressed).

#endif /* u_pedals.h */