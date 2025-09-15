#ifndef __U_MUTEX_H
#define __U_MUTEX_H

#include "tx_api.h"
#include "u_general.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    /* PUBLIC: Mutex Configuration Settings */
    /* Set these when defining an instance of this struct. */
    const CHAR *name;            /* Name of Mutex */
    const UINT priority_inherit; /* Specifies if the mutex supports priority inheritance. See page 55 of the "Azure RTOS ThreadX User Guide". */

    /* PRIVATE: Internal implementation - DO NOT ACCESS DIRECTLY */
    /* (should only be accessed by functions in u_mutexes.c) */
    TX_MUTEX _TX_MUTEX;
} mutex_t;

/* Mutex List */
extern mutex_t faults_mutex;       // Faults Mutex
extern mutex_t brake_state_mutex;  // Brake State Mutex
extern mutex_t pedal_data_mutex;   // Pedal Data Mutex
extern mutex_t bms_mutex;          // BMS Mutex
extern mutex_t torque_limit_mutex; // Torque Limit Mutex
extern mutex_t dti_mutex;          // DTI Mutex
extern mutex_t adc1_mutex;         // ADC1 Mutex
extern mutex_t adc2_mutex;         // ADC2 Mutex
// add more as necessary...

/* API */
uint8_t mutexes_init(); // Initializes all mutexes set up in u_mutexes.c
uint8_t mutex_get(mutex_t *mutex); // Gets a mutex.
uint8_t mutex_put(mutex_t *mutex); // Puts a mutex.

#endif /* u_mutex.h */