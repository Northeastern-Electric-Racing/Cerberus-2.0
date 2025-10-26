#ifndef __U_PEDALS_H
#define __U_PEDALS_H

/* API */
void pedals_process(void);                              // Pedal Processing Function. Meant to be called by the pedals thread.

/* Pedal Data. */
typedef struct {
    float acceleration_percentage;
    float brake_percentage;
    bool brake_pressed;
} pedal_data_t;
pedal_data_t pedals_getData(void); // Returns the current pedals data.

#endif /* u_pedals.h */