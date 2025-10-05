#ifndef __STATEMACHINE_H
#define __STATEMACHINE_H

#include "u_tx_debug.h"
#include <stdint.h>
#include <stdio.h>

// NOTE: - This file is currenty unused. it was a draft for a statemachine redesign but im not doing that rn

/* Car states */
typedef enum {
    STATE_READY,
    STATE_PIT,
    STATE_REVERSE,
    STATE_PERFORMANCE,
    STATE_EFFICIENCY,
    STATE_FAULTED,
    MAX_STATES
} state_t;

/* Car events */
typedef enum {
    EVENT_BUTTON, // Event triggered by a button press.
    EVENT_DIAL,   // Event triggered by a dial switch.
    MAX_EVENTS
} event_t;

/**
 * @brief Processes an event based on the car's current state.
 * @param event The event that was triggered.
 * @param data Pointer to any data associated with the event. Can be NULL if not needed.
 */
uint8_t statemachine_process_event(event_t event, uint8_t *data);

#endif /* u_statemachine.h */

