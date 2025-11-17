#ifndef __U_FAULTS_H
#define __U_FAULTS_H

#include <stdbool.h>

/* List of fault IDs. */
typedef enum {
    /* Critical Faults */
	ONBOARD_PEDAL_OPEN_CIRCUIT_FAULT,
	ONBOARD_PEDAL_SHORT_CIRCUIT_FAULT,
	ONBOARD_PEDAL_DIFFERENCE_FAULT,
	CAN_DISPATCH_FAULT,
	CAN_ROUTING_FAULT,
	BMS_CAN_MONITOR_FAULT,

    /* Non-critical Faults */
	ONBOARD_TEMP_FAULT,
	IMU_FAULT,
	FUSE_MONITOR_FAULT,
	SHUTDOWN_MONITOR_FAULT,
	LV_MONITOR_FAULT,
	BSPD_PREFAULT,
	RTDS_FAULT,
	PUMP_SENSORS_FAULT,
	PDU_CURRENT_FAULT,
	LOW_LV_BATTERY_VOLTS,

    /* Total number of faults (cannot exceed 64.) */
	/* (always keep this at the end) */
    NUM_FAULTS

} fault_t;

int faults_init(void);
int trigger_fault(fault_t fault_id);
uint64_t get_faults(void);
void write_mcu_fault(bool status); /* Write the VCU FAULT line (from the microcontroller to the car). true = faulted, false = unfaulted. */

#endif /* u_faults.h */
