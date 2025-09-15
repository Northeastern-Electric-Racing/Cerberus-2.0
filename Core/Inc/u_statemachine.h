#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

// #define TSMS_OVERRIDE //Uncomment to remove all checks for state machine
// #define IGNORE_FAULT

#include "u_dti.h"
#include <stdbool.h>

/**
 * @brief Enum defining the functional states of the car.
 * 
 */
typedef enum {
	READY,
	/* F means functional */
	F_PIT,
	F_REVERSE, // can only enter reverse from PIT
	F_PERFORMANCE,
	F_EFFICIENCY,
	FAULTED,
	MAX_FUNC_STATES
} func_state_t;

/**
 * @brief Emum that maps to NERO indexes to the menu on the NERO screen.
 * Cannot be above 15 due to CAN send limits
 */
typedef enum {
	OFF,
	PIT, //SPEED_LIMITIED
	REVERSE,
	PERFORMANCE, //AUTOCROSS
	EFFICIENCY, //ENDURANCE
	GAMES,
	EXIT,
	MAX_NERO_STATES
} nero_menu_t;

typedef struct {
	nero_menu_t nero_index;
	bool home_mode;
} nero_state_t;

/**
 * @brief Struct for defining the state of the car.
 * 
 */
typedef struct {
	func_state_t functional;
	nero_state_t nero;
} state_t;

typedef struct {
	enum { FUNCTIONAL, NERO } id;
	union {
		func_state_t functional;
		nero_state_t nero;
	} state;
} state_req_t;

/**
 * @brief Retrieve the current functional state.
 * 
 * @return func_state_t Struct containing the current functional state
 */
func_state_t get_func_state();

/**
 * @brief Returns true if car is in active state (pit, performance, efficiency)
 * 
 * @return Whether or not the car is in an active state.
 */
bool get_active();

/**
 * @brief Retrieves the current NERO state.
 * 
 * @return nero_state_t The current NERO state
 */
nero_state_t get_nero_state();

/**
 * @brief Increments the nero index in the order of nero_menu_t which will be used to select a drive mode.
 * 
 * @return int Error code resulting from queueing a state transition
 */
int increment_nero_index();

/**
 * @brief Decrements the nero index int the order of nero_menu_t which will be used to select a drive mode.
 * 
 * @return int Error code resulting from queueing a state transition
 */
int decrement_nero_index();

/**
 * @brief Enter the mode defined by the NERO index.
 * 
 * @return int Error code resulting from queueing a state transition
 */
int select_nero_index();

/**
 * Sets the home mode to be true, which will turn off the car and set the functional state to ready
 */

/**
 * @brief Queue a state transition to enter home mode.
 * 
 * @return int Error code resulting from queueing a state transition
 */
int set_home_mode();

/**
 * @brief Queue a state transition to enter ready mode.
 * 
 * @return int Error code resulting from queueing a state transition
 */
int set_ready_mode();

/**
 * @brief Queue a state transition to set the functinoal mode of the car to the faulted state.
 * 
 * @return int Error code resulting from queueing a state transition
 */
int fault();

/* Process the state machine */
void statemachine_process(void);
int init_statemachine(void);

#endif