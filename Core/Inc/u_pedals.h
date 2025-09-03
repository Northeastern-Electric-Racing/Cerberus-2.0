#ifndef __U_PEDALS_H
#define __U_PEDALS_H

/* API */
int pedals_init(void);     // Initializes Pedals ADC
void pedals_process(void); // Pedal Processing Function. Meant to be called by the pedals thread.

#endif /* u_pedals.h */