#include <stdint.h>
#include <stdatomic.h>
#include "tx_api.h"
#include "main.h"
#include "u_tx_timers.h"
#include "u_faults.h"
#include "u_statemachine.h"
#include "u_tx_debug.h"
#include "u_mutexes.h"

typedef enum { CRITICAL, NON_CRITICAL } _severity;

typedef struct {
    const char *name; /* Fault name. */
    _severity severity; /* Fault severity. */
    int timeout; /* Fault timeout (in ticks). */
} _metadata;

/* Fault Table */
/* This table should be kept in the same order as the fault_t enum in the header file. */
static const _metadata faults[] = {
    /* Critical Faults */
    [CAN_OUTGOING_FAULT] = {"CAN_OUTGOING_FAULT", CRITICAL, .timeout = 5000},
    [CAN_INCOMING_FAULT] = {"CAN_INCOMING_FAULT", CRITICAL, .timeout = 5000},
    [BMS_CAN_MONITOR_FAULT] = {"BMS_CAN_MONITOR_FAULT", CRITICAL, .timeout = 5000},
    [LIGHTNING_CAN_MONITOR_FAULT] = {"LIGHTNING_CAN_MONITOR_FAULT", CRITICAL, .timeout = 5000},
    [SHUTDOWN_FAULT] = {"SHUTDOWN_FAULT", CRITICAL, .timeout = 5000},

    /* Non-critical Faults */
    [ONBOARD_TEMP_FAULT] = {"ONBOARD_TEMP_FAULT", NON_CRITICAL, .timeout = 5000},
    [IMU_ACCEL_FAULT] = {"IMU_ACCEL_FAULT", NON_CRITICAL, .timeout = 5000},
    [IMU_GYRO_FAULT] = {"IMU_GYRO_FAULT", NON_CRITICAL, .timeout = 5000},
    [BSPD_PREFAULT] = {"BSPD_PREFAULT", NON_CRITICAL, .timeout = 5000},
    [ONBOARD_BRAKE_OPEN_CIRCUIT_FAULT] = {"ONBOARD_BRAKE_OPEN_CIRCUIT_FAULT", NON_CRITICAL, .timeout = 5000},
    [ONBOARD_ACCEL_OPEN_CIRCUIT_FAULT] = {"ONBOARD_ACCEL_OPEN_CIRCUIT_FAULT", NON_CRITICAL, .timeout = 5000},
    [ONBOARD_BRAKE_SHORT_CIRCUIT_FAULT] = {"ONBOARD_BRAKE_SHORT_CIRCUIT_FAULT", NON_CRITICAL, .timeout = 5000},
    [ONBOARD_ACCEL_SHORT_CIRCUIT_FAULT] = {"ONBOARD_ACCEL_SHORT_CIRCUIT_FAULT", NON_CRITICAL, .timeout = 5000},
    [ONBOARD_PEDAL_DIFFERENCE_FAULT] = {"ONBOARD_PEDAL_DIFFERENCE_FAULT", NON_CRITICAL, .timeout = 5000},
    [RTDS_FAULT] = {"RTDS_FAULT", NON_CRITICAL, .timeout = 5000},
    [LV_LOW_VOLTAGE_FAULT] = {"LV_LOW_VOLTS_FAULT", NON_CRITICAL, .timeout = 5000},
};

/* Fault Globals*/
static timer_t timers[NUM_FAULTS];         // Array of fault timers. One timer per fault.
static _Atomic uint32_t severity_mask = 0; // Mask that stores the severity configuration for each fault (0=NON_CRITICAL, 1=CRITICAL).
static _Atomic uint32_t fault_flags = 0;   // Each bit is a separate fault (0=Not Faulted, 1=Faulted).

/* Getter function. Returns ALL faults. */
uint32_t get_faults(void) {
    return fault_flags;
}

/* Returns whether or not a specific fault is active. */
bool get_fault(fault_t fault) {
    return (fault_flags & (1 << fault)) != 0;
}

/* Returns whether or not any critical faults are active. */
bool are_critical_faults_active(void) {
    return (fault_flags & severity_mask) == 0;
}

/* Callback function. Clears fault after timer expires. */
static void _timer_callback(ULONG args) {
    fault_t fault_id = (fault_t)args;

    /* Clear the fault. */
    atomic_fetch_and(&fault_flags, ~((uint32_t)(1 << fault_id))); // This is the _Atomic version of: fault_faults &= ~((uint32_t)(1 << fault_id));
    PRINTLN_INFO("Cleared fault (Fault: %s).", faults[fault_id].name);

    /* Check if there are any active critical faults. If not, unfault the car. */
    if (are_critical_faults_active()) {
        if (get_func_state() == FAULTED) {
            set_ready_mode();
        }
    }
}

/* Initializes the fault seveity mask, and creates all timers. */
int faults_init(void) {
    for (int fault_id = 0; fault_id < NUM_FAULTS; fault_id++) {
        /* Initialize severity_mask. */
        if (faults[fault_id].severity == CRITICAL) {
            atomic_fetch_or(&severity_mask, ((uint32_t)1 << fault_id)); // This is the _Atomic version of: severity_mask |= ((uint32_t)1 << fault_id);
        }

        /* Initialize all timers. */
        timers[fault_id].name = "Fault Timer";
        timers[fault_id].callback = _timer_callback;
        timers[fault_id].callback_input = fault_id;
        timers[fault_id].duration = faults[fault_id].timeout;
        timers[fault_id].type = ONESHOT;
        timers[fault_id].auto_activate = false;
        int status = timer_init(&timers[fault_id]);
        if (status != U_SUCCESS) {
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
    /* Set the relevant fault bit in the fault flags list. */
    atomic_fetch_or(&fault_flags, (uint32_t)(1 << fault_id)); // This is the _Atomic version of: fault_flags |= (uint32_t)(1 << fault_id);

    switch (faults[fault_id].severity) {
        case CRITICAL:
            PRINTLN_INFO("Triggered CRITICAL FAULT (Fault: %s).", faults[fault_id].name);
            fault();
            PRINTLN_INFO("got past fault()");
            break;
        case NON_CRITICAL:
            PRINTLN_INFO("Triggered non-critical fault (Fault: %s).", faults[fault_id].name); // If the fault is non-critical, the car doesn't need to be put in its faulted state.
        break;
    }

    /* Restart fault timer. */
    int status = timer_restart(&timers[fault_id]);
    if (status != U_SUCCESS) {
        PRINTLN_ERROR("Failed to restart fault timer (Status: %d, Fault: %s).", status, faults[fault_id].name);
        return U_ERROR;
    }

    PRINTLN_INFO("got past timer restart part");

    return U_SUCCESS;
}

/* Write the VCU FAULT line (from the microcontroller to the car). */
void write_mcu_fault(bool status) {
    // The MCU Fault pin is kind of "swapped".
    // Setting the pin to HIGH indicates that there is no fault.
    // Setting the pin to LOW indicates that there is a fault.
    // The pin has a default state of HIGH (i.e. no fault).
    if (status) {
        // If there is a fault, set the fault pin to LOW.
        PRINTLN_INFO("Turned on MCU fault.");
        HAL_GPIO_WritePin(FAULT_MCU_GPIO_Port, FAULT_MCU_Pin, GPIO_PIN_RESET);
    } else {
        // If there is not a fault, set the pin to HIGH.
        PRINTLN_INFO("Turned off MCU fault.");
        HAL_GPIO_WritePin(FAULT_MCU_GPIO_Port, FAULT_MCU_Pin, GPIO_PIN_SET);
    }
}

/* Static Asserts */
/* (These throw compile-time errors if certain rules are broken.) */
/* Probably keep these at the bottom of this file */
_Static_assert(NUM_FAULTS <= 32, "This project does not (currently) support more than 32 faults."); // Ensures there aren't more than 32 faults.
_Static_assert(sizeof(faults) / sizeof(faults[0]) == NUM_FAULTS, "Fault table size must match NUM_FAULTS. Make sure the fault table is consistent with the enum."); // Ensures the fault table size matches NUM_FAULTS.
