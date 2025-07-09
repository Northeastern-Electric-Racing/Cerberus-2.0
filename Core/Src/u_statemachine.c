#include "u_statemachine.h"

// u_TODO - this whole file is a todo. i dunno how state machines are supposed to work

static state_t current_state = STATE_READY; // Stores the car's current state. The car's initial state on startup can be set here.

/* Called when EVENT_BUTTON is triggered. */
static uint8_t _button(uint8_t *data) {
    /* Handle the event based on the current state. */
    switch (current_state) {
        case STATE_READY:
            // Do thing
            return U_SUCCESS;;
        case STATE_PIT:
            // Do thing
            return U_SUCCESS;

        // u_TODO - add more states here as needed.
        // u_TODO - if EVENT_BUTTON doesn't need to do anything in a state, prolly just don't add a case for it

        /* Do nothing by default. */
        default:
            printf("[u_statemachine.c/_button()] NOTE: EVENT_BUTTON was triggered, but no action was taken based on the car's state (current_state=%d).\n", current_state);
            return U_SUCCESS;
    }
}

/* Called when EVENT_DIAL is triggered. */
static uint8_t _dial(uint8_t *data) {
    /* Handle the event based on the current state. */
    switch (current_state) {
        case STATE_EFFICIENCY:
            // Do thing
            return U_SUCCESS;
        
        // u_TODO - add more states here as needed.
        // u_TODO - if EVENT_DIAL doesn't need to do anything in a state, prolly just don't add a case for it

        /* Do nothing by default. */
        default:
            printf("[u_statemachine.c/_dial()] NOTE: EVENT_DIAL was triggered, but no action was taken based on the car's state (current_state=%d).\n", current_state);
            return U_SUCCESS;
    }
}

uint8_t statemachine_process_event(event_t event, uint8_t *data) {
    switch (event) {
        /* Call the function corresponding to the event that occured. */
        case EVENT_BUTTON:       return _button(data);
        case EVENT_DIAL:         return _dial(data);
        // u_TODO - probably add more events as needed
        
        /* If an invalid event is passed in, return an error. */
        default: 
            printf("[u_statemachine.c/statemachine_process_event()] Invalid event passed into function: %d\n", current_state);
            return U_ERROR;
    }
}
