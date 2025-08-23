#include "u_faults.h"

typedef enum {
    CRITICAL,
    NON_CRITICAL
} severity;

typedef struct {
    const char* name;   /* Fault name. */
    severity severity;  /* Fault severity. */
    int timeout;        /* Fault timeout (in ticks). */
} metadata;

/* Fault Table */
static const metadata faults[] = {
    /* Critical Faults */
    [ONBOARD_PEDAL_OPEN_CIRCUIT_FAULT] = {"ONBOARD_PEDAL_OPEN_CIRCUIT_FAULT", CRITICAL, 400},
    [ONBOARD_PEDAL_SHORT_CIRCUIT_FAULT] = {"ONBOARD_PEDAL_SHORT_CIRCUIT_FAULT", CRITICAL, 400},
    [ONBOARD_PEDAL_DIFFERENCE_FAULT] = {"ONBOARD_PEDAL_DIFFERENCE_FAULT", CRITICAL, 400},
    [CAN_DISPATCH_FAULT] = {"CAN_DISPATCH_FAULT", CRITICAL, 400},
    [CAN_ROUTING_FAULT] = {"CAN_ROUTING_FAULT", CRITICAL, 400},
    [BMS_CAN_MONITOR_FAULT] = {"BMS_CAN_MONITOR_FAULT", CRITICAL, 400},

    /* Non-critical Faults */
    [ONBOARD_TEMP_FAULT] = {"ONBOARD_TEMP_FAULT", NON_CRITICAL, 400},
    [IMU_FAULT] = {"IMU_FAULT", NON_CRITICAL, 400},
    [FUSE_MONITOR_FAULT] = {"FUSE_MONITOR_FAULT", NON_CRITICAL, 400},
    [SHUTDOWN_MONITOR_FAULT] = {"SHUTDOWN_MONITOR_FAULT", NON_CRITICAL, 400},
    [LV_MONITOR_FAULT] = {"LV_MONITOR_FAULT", NON_CRITICAL, 400},
    [BSPD_PREFAULT] = {"BSPD_PREFAULT", NON_CRITICAL, 400},
    [RTDS_FAULT] = {"RTDS_FAULT", NON_CRITICAL, 400},
    [PUMP_SENSORS_FAULT] = {"PUMP_SENSORS_FAULT", NON_CRITICAL, 400},
    [PDU_CURRENT_FAULT] = {"PDU_CURRENT_FAULT", NON_CRITICAL, 400},
};

/* Static Asserts */
/* (These throw compile-time errors if certain rules are broken.) */
/* Probably keep these at the bottom of this file */
_Static_assert(NUM_FAULTS <= 32, "This project does not (currently) support more than 32 faults."); // Ensures there aren't more than 32 faults.
_Static_assert(sizeof(faults) / sizeof(faults[0]) == NUM_FAULTS, "Fault table size must match NUM_FAULTS. Make sure the fault table is consistent with the enum."); // Ensures the fault table size matches NUM_FAULTS.
