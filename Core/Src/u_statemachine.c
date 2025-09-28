#include "u_statemachine.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tx_api.h"
#include "bitstream.h"
#include "u_dti.h"
#include "u_pedals.h"
#include "u_can.h"
#include "u_rtds.h"
#include "u_tx_debug.h"
#include "u_queues.h"
#include "u_faults.h"
#include "u_pedals.h"

#define STATE_TRANS_QUEUE_SIZE 4

#define SEND_NERO_TIMEOUT	200 /*in millis*/
#define TS_RISING_BLOCK_TIMEOUT 3000 /*in millis*/

// #define DISABLE_REVERSE

/* Globals. */
static state_t cerberus_state;
static TX_TIMER ts_rising_timer;
static bool is_ts_rising = false;
static bool enter_drive_enabled = false;

static void _rising_ts_cb(ULONG input)
{
	enter_drive_enabled = true;
}

/* Gets TSMS status. */
static bool _get_tsms(void) {
	// U_TODO - This needs to be implemented.
	DEBUG_PRINTLN("ERROR: _get_tsms() has not been implemented yet.");
	return false;
}

static void _send_nero_msg(void)
{
	bitstream_t nero_msg;
	uint8_t bitstream_data[6];
	bitstream_init(&nero_msg, bitstream_data, 6); // Create 5-byte bitstream

	bitstream_add(&nero_msg, get_nero_state().home_mode, 4);
	bitstream_add(&nero_msg, get_nero_state().nero_index, 4);
	bitstream_add_signed(&nero_msg, dti_get_mph() * 10, 16);
	bitstream_add(&nero_msg, _get_tsms(), 1);
	bitstream_add(&nero_msg, pedals_getTorqueLimitPercentage() * 100, 7);
	bitstream_add(&nero_msg, cerberus_state.functional != F_REVERSE, 1);
	bitstream_add(&nero_msg, pedals_getRegenLimit(), 10);
	bitstream_add(&nero_msg, pedals_getLaunchControl(), 1);

	can_msg_t msg = { .id = 0x501, .len = sizeof(bitstream_data) };

	memcpy(msg.data, &bitstream_data, sizeof(bitstream_data));

	/* Send CAN message */
	queue_send(&can_outgoing, &msg);
}

int init_statemachine(void) {
	/* Create TS Rising Timer. */
	int status = tx_timer_create(
            &ts_rising_timer,   		  /* Timer Instance */
            "TS Rising Timer",  		  /* Timer Name */
            _rising_ts_cb,    			  /* Timer Expiration Callback */
            0,                  		  /* Callback Input */
            TS_RISING_BLOCK_TIMEOUT,      /* Ticks until timer expiration. */
            0,                  		  /* Number of ticks for all timer expirations after the first (0 makes this a one-shot timer). */
            TX_NO_ACTIVATE      		  /* Make the timer dormant until it is activated. */
        );
        if(status != TX_SUCCESS) {
            DEBUG_PRINTLN("ERROR: Failed to create TS Rising timer (Status: %d/%s).", status, tx_status_toString(status));
            return U_ERROR;
        }
	DEBUG_PRINTLN("Ran init_statemachine().");

	return U_SUCCESS;
}

func_state_t get_func_state()
{
	return cerberus_state.functional;
}

bool get_active()
{
	return cerberus_state.functional == F_EFFICIENCY ||
	       cerberus_state.functional == F_PERFORMANCE ||
	       cerberus_state.functional == F_PIT ||
	       cerberus_state.functional == F_REVERSE;
}

nero_state_t get_nero_state()
{
	return cerberus_state.nero;
}

static int transition_functional_state(func_state_t new_state)
{
	/* Special case: should be able to fault no matter what conditions */
	if (new_state == FAULTED) {
		/* Turn off high power peripherals */
		cerberus_state.nero = (nero_state_t){ .nero_index = OFF, .home_mode = true };
		write_mcu_fault(true);
		printf("FAULTED\r\n");
	}

	/* Make sure wheels are not spinning before changing modes */
	bool brake_state;
	rtds_stopReverseSound();

	/* Catching state transitions */
	switch (new_state) {
	case READY:
		/* Turn off high power peripherals */
		write_mcu_fault(false);
		printf("READY\n");
		break;
	case F_REVERSE:
#ifdef DISABLE_REVERSE
		printf("Reverse is disabled.");
		return 4;
#endif
		rtds_startReverseSound();
		break;
	case F_PIT:
	case F_PERFORMANCE:
	case F_EFFICIENCY:

		brake_state = pedals_getBrakeState();
#ifdef TSMS_OVERRIDE
		if (_get_tsms() && (!brake_state || cerberus_state.functional == FAULTED)) { // only enforce brake / fault if tsms is actually on
			return 3;
		}
		printf("Ignoring tsms\n\n");
#else
		if (cerberus_state.functional == FAULTED) {
			printf("Cannot drive from a fault!\n");
			return 3;
		}

		if (!enter_drive_enabled) {
			printf("Must wait before entering drive!");
			return 3;
		}

		/* Only turn on motor if brakes engaged and tsms is on */
		if (!brake_state || !_get_tsms()) {
			return 3;
		}
#endif

		if (_get_tsms()) {
			rtds_soundRTDS();
		}

		printf("ACTIVE STATE\r\n");
		break;
	default:
		// Do Nothing
		break;
	}

	cerberus_state.functional = new_state;

	return 0;
}

static int transition_nero_state(nero_state_t new_state)
{
	nero_state_t current_nero_state = get_nero_state();

	// If we are not in home mode, we should not change the nero index
	if (!new_state.home_mode)
		new_state.nero_index = current_nero_state.nero_index;

	// Checks for when we are in home mode and the nero index is out of bounds
	if (new_state.nero_index < 0)
		new_state.nero_index = 0;
	if (new_state.nero_index >= MAX_NERO_STATES)
		new_state.nero_index = MAX_NERO_STATES - 1;

	// Selecting a mode on NERO
	if (current_nero_state.home_mode && !new_state.home_mode) {
		if (new_state.nero_index < GAMES && new_state.nero_index > OFF) {
			if (transition_functional_state(new_state.nero_index)) {
				DEBUG_PRINTLN("ERROR: Failed to transition functional state.");
				return 1;
			}
		}

		/* TSMS OFF and MPH = 0 to enter games */
		if (new_state.nero_index == GAMES) {
#ifndef TSMS_OVERRIDE
			if (_get_tsms() || dti_get_mph() >= 1) {
				return 1;
			}
#endif
			new_state.home_mode = false;
		}
	}

	// Entering home mode
	if (get_active() && !current_nero_state.home_mode &&
	    new_state.home_mode) {
		if (transition_functional_state(READY))
			return 1;
	}

	cerberus_state.nero = new_state;

	return 0;
}

static int check_state_change(state_req_t new_state)
{
	// check if nero state has changed
	if (new_state.id == NERO) {
		nero_state_t new_nero_state = new_state.state.nero;
		nero_state_t current_nero_state = get_nero_state();
		if (new_nero_state.home_mode == current_nero_state.home_mode &&
		    new_nero_state.nero_index ==
			    current_nero_state.nero_index) {
			return 0;
		}
	}

	// check if functional state has changed
	if (new_state.id == FUNCTIONAL) {
		func_state_t new_func_state = new_state.state.functional;
		func_state_t current_func_state = get_func_state();
		if (new_func_state == current_func_state) {
			return 0;
		}
	}

	return 1;
}

static int queue_state_transition(state_req_t new_state)
{
	return queue_send(&state_transition_queue, &new_state);
}

/* HANDLE USER INPUT */
int increment_nero_index()
{
	/* Wrap around if end of menu reached */
	if (get_nero_state().nero_index + 1 >= MAX_NERO_STATES) {
		return queue_state_transition((state_req_t){
			.id = NERO,
			.state.nero = (nero_state_t){
				.home_mode = get_nero_state().home_mode,
				.nero_index = OFF } });
	}
	return queue_state_transition((state_req_t){
		.id = NERO,
		.state.nero = (nero_state_t){
			.home_mode = get_nero_state().home_mode,
			.nero_index = get_nero_state().nero_index + 1 } });
}

int decrement_nero_index()
{
	return queue_state_transition((state_req_t){
		.id = NERO,
		.state.nero = (nero_state_t){
			.home_mode = get_nero_state().home_mode,
			.nero_index = get_nero_state().nero_index - 1 } });
}

int select_nero_index()
{
	return queue_state_transition((state_req_t){
		.id = NERO,
		.state.nero = (nero_state_t){
			.home_mode = false,
			.nero_index = get_nero_state().nero_index } });
}

int set_home_mode()
{
	return queue_state_transition((state_req_t){
		.id = NERO,
		.state.nero = { .nero_index = get_nero_state().nero_index,
				.home_mode = true } });
}

int set_ready_mode()
{
#ifdef IGNORE_FAULT
	return 1;
#endif
	return queue_state_transition(
		(state_req_t){ .id = FUNCTIONAL, .state.functional = READY });
}

int fault()
{
#ifdef IGNORE_FAULT
	return 1;
#endif
	return queue_state_transition(
		(state_req_t){ .id = FUNCTIONAL, .state.functional = FAULTED });
}

void statemachine_process(void) {
	state_req_t new_state_req;
	while(queue_receive(&state_transition_queue, &new_state_req) == U_SUCCESS) {
		if (check_state_change(new_state_req)) {
				if (new_state_req.id == NERO) {
					transition_nero_state(new_state_req.state.nero);
				}
				else if (new_state_req.id == FUNCTIONAL) {
					transition_functional_state(new_state_req.state.functional);
				}
			}
	}

	if (!is_ts_rising && _get_tsms()) {
			is_ts_rising = true;

			/* Deactivate the TS Rising timer. */
    		int status = tx_timer_deactivate(&ts_rising_timer);
    		if(status != TX_SUCCESS) {
        		DEBUG_PRINTLN("ERROR: Failed to deactivate TS Rising timer (in !is_ts_rising && _get_tsms()) (Status: %d/%s).", status, tx_status_toString(status));
        		return;
    		}

    		/* Change the TS Rising timer. */
    		status = tx_timer_change(&ts_rising_timer, TS_RISING_BLOCK_TIMEOUT, 0);
    		if(status != TX_SUCCESS) {
        		DEBUG_PRINTLN("ERROR: Failed to change TS Rising timer (Status: %d/%s).", status, tx_status_toString(status));
        		return;
    		}

    		/* Activate the TS Rising timer. */
    		status = tx_timer_activate(&ts_rising_timer);
    		if(status != TX_SUCCESS) {
        		DEBUG_PRINTLN("ERROR: Failed to activate TS Rising timer (Status: %d/%s).", status, tx_status_toString(status));
        		return;
    		}
	} else if (!_get_tsms()) {
		/* Deactivate the TS Rising timer. */
    	int status = tx_timer_deactivate(&ts_rising_timer);
    	if(status != TX_SUCCESS) {
        	DEBUG_PRINTLN("ERROR: Failed to deactivate TS Rising timer (in !_get_tsms()) (Status: %d/%s).", status, tx_status_toString(status));
        	return;
    	}
		is_ts_rising = false;
		enter_drive_enabled = false;
	}

	// send nero data periodically
	_send_nero_msg();
}