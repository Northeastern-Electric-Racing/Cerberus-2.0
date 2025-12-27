#include <stdint.h>
#include "tx_api.h"
#include "main.h"
#include "u_tx_timers.h"
#include "u_faults.h"
#include "u_statemachine.h"
#include "u_tx_debug.h"
#include "u_mutexes.h"

typedef enum {
    CRITICAL,
    NON_CRITICAL
} _severity;

typedef struct {
    const char* name;   /* Fault name. */
    _severity severity; /* Fault severity. */
    int timeout;        /* Fault timeout (in ticks). */
} _metadata;

/* Fault Table */
/* This table should be kept in the same order as the fault_t enum in the header file. */
static const _metadata faults[] = {
    /* Critical Faults */
    [ONBOARD_BRAKE_OPEN_CIRCUIT_FAULT] = {"ONBOARD_BRAKE_OPEN_CIRCUIT_FAULT", CRITICAL, .timeout = 400},
    [ONBOARD_ACCEL_OPEN_CIRCUIT_FAULT] = {"ONBOARD_ACCEL_OPEN_CIRCUIT_FAULT", CRITICAL, .timeout = 400},
    [ONBOARD_BRAKE_SHORT_CIRCUIT_FAULT] = {"ONBOARD_BRAKE_SHORT_CIRCUIT_FAULT", CRITICAL, .timeout = 400},
    [ONBOARD_ACCEL_SHORT_CIRCUIT_FAULT] = {"ONBOARD_ACCEL_SHORT_CIRCUIT_FAULT", CRITICAL, .timeout = 400},
    [ONBOARD_PEDAL_DIFFERENCE_FAULT] = {"ONBOARD_PEDAL_DIFFERENCE_FAULT", CRITICAL, .timeout = 400},
    [CAN_OUTGOING_FAULT] = {"CAN_OUTGOING_FAULT", CRITICAL, .timeout = 400},
    [CAN_INCOMING_FAULT] = {"CAN_INCOMING_FAULT", CRITICAL, .timeout = 400},
    [BMS_CAN_MONITOR_FAULT] = {"BMS_CAN_MONITOR_FAULT", CRITICAL, .timeout = 400},

    /* Non-critical Faults */
    [ONBOARD_TEMP_FAULT] = {"ONBOARD_TEMP_FAULT", NON_CRITICAL, .timeout = 400},
    [IMU_ACCEL_FAULT] = {"IMU_ACCEL_FAULT", NON_CRITICAL, .timeout = 400},
    [IMU_GYRO_FAULT] = {"IMU_GYRO_FAULT", NON_CRITICAL, .timeout = 400},
    [BSPD_PREFAULT] = {"BSPD_PREFAULT", NON_CRITICAL, .timeout = 400},
    [RTDS_FAULT] = {"RTDS_FAULT", NON_CRITICAL, .timeout = 400},
    [PUMP_SENSORS_FAULT] = {"PUMP_SENSORS_FAULT", NON_CRITICAL, .timeout = 400},
    [PDU_CURRENT_FAULT] = {"PDU_CURRENT_FAULT", NON_CRITICAL, .timeout = 400},
};

/* Fault Globals*/
static timer_t timers[NUM_FAULTS]; // Array of fault timers. One timer per fault.
static uint64_t severity_mask; // Mask that stores the severity configuration for each fault (0=NON_CRITICAL, 1=CRITICAL).
static volatile uint64_t fault_flags; // Each bit is a separate fault (0=Not Faulted, 1=Faulted).

/* Getter function for accessing faults in other files. */
uint64_t get_faults(void) {
    return fault_flags;
}

/* Callback function. Clears fault after timer expires. */
static void _timer_callback(ULONG args) {
    fault_t fault_id = (fault_t)args;

    /* Get faults mutex. */
    mutex_get(&faults_mutex);

    /* Clear the fault. */
    fault_flags &= ~((uint64_t)(1 << fault_id));
    PRINTLN_INFO("Cleared fault (Fault: %s).", faults[fault_id].name);

    /* Check if there are any active critical faults. If not, unfault the car. */
    if((fault_flags & severity_mask) == 0) {        
        if(get_func_state() == FAULTED) {
             set_ready_mode();
        }
    }

    /* Put faults mutex. */
    mutex_put(&faults_mutex);
}

/* Initializes the fault seveity mask, and creates all timers. */
int faults_init(void) {
    for(int fault_id = 0; fault_id < NUM_FAULTS; fault_id++) {

        /* Initialize severity_mask. */
        if(faults[fault_id].severity == CRITICAL) {
            severity_mask |= ((uint64_t)1 << fault_id);
        }

        /* Initialize all timers. */
        timers[fault_id].name = "Fault Timer";
        timers[fault_id].callback = _timer_callback;
        timers[fault_id].callback_input = fault_id;
        timers[fault_id].duration = faults[fault_id].timeout;
        timers[fault_id].type = ONESHOT;
        timers[fault_id].auto_activate = false;
        int status = timer_init(&timers[fault_id]);
        if(status != U_SUCCESS) {
            PRINTLN_ERROR("Failed to create fault timer (Status: %d, Fault: %s).", status, faults[fault_id].name);
            return U_ERROR;
        }
    }

    PRINTLN_INFO("Ran faults_init().");

    return U_SUCCESS;
}

/* Triggers a fault. */
/* If the fault is already triggered, this just resets the fault's timer. */
int trigger_fault(fault_t fault_id) {

    /* Get faults mutex. */
    mutex_get(&faults_mutex);

    fault_flags |= (uint64_t)(1 << fault_id); // Set the relevant fault bit.

    /* Put faults mutex. */
    mutex_put(&faults_mutex);

    switch(faults[fault_id].severity) {
        case CRITICAL:
            PRINTLN_INFO("Triggered CRITICAL FAULT (Fault: %s).", faults[fault_id].name);
            fault();
            break;
        case NON_CRITICAL:
            PRINTLN_INFO("Triggered non-critical fault (Fault: %s).", faults[fault_id].name);
            // If the fault is non-critical, the car doesn't need to be put in its faulted state.
            break;
    }

    /* Restart fault timer. */
    int status = timer_restart(&timers[fault_id]);
    if(status != U_SUCCESS) {
        PRINTLN_ERROR("Failed to restart fault timer (Status: %d, Fault: %s).", status, faults[fault_id].name);
        return U_ERROR;
    }

    return U_SUCCESS;
}

/* Write the VCU FAULT line (from the microcontroller to the car). */
void write_mcu_fault(bool status)
{   
    // The MCU Fault pin is kind of "swapped".
    // Setting the pin to HIGH indicates that there is no fault.
    // Setting the pin to LOW indicates that there is a fault.
    // The pin has a default state of HIGH (i.e. no fault).
    if(status) {
        // If there is a fault, set the fault pin to LOW.
        PRINTLN_INFO("Turned on MCU fault.");
        HAL_GPIO_WritePin(FAULT_MCU_GPIO_Port, FAULT_MCU_Pin, GPIO_PIN_RESET);
    }
    else {
        // If there is not a fault, set the pin to HIGH.
        PRINTLN_INFO("Turned off MCU fault.");
        HAL_GPIO_WritePin(FAULT_MCU_GPIO_Port, FAULT_MCU_Pin, GPIO_PIN_SET);
    }
}

/* Static Asserts */
/* (These throw compile-time errors if certain rules are broken.) */
/* Probably keep these at the bottom of this file */
_Static_assert(NUM_FAULTS <= 64, "This project does not (currently) support more than 64 faults."); // Ensures there aren't more than 64 faults.
_Static_assert(sizeof(faults) / sizeof(faults[0]) == NUM_FAULTS, "Fault table size must match NUM_FAULTS. Make sure the fault table is consistent with the enum."); // Ensures the fault table size matches NUM_FAULTS.
