#ifndef __U_MUTEX_H
#define __U_MUTEX_H

#include "tx_api.h"
#include "u_tx_debug.h"
#include "u_tx_mutex.h"
#include <stdint.h>
#include <stdbool.h>

/* Mutex List */
extern mutex_t faults_mutex;       // Faults Mutex
extern mutex_t pedal_data_mutex;   // Pedal Data Mutex
extern mutex_t bms_mutex;          // BMS Mutex
extern mutex_t torque_limit_mutex; // Torque Limit Mutex
extern mutex_t dti_mutex;          // DTI Mutex
extern mutex_t adc1_mutex;         // ADC1 Mutex
extern mutex_t adc2_mutex;         // ADC2 Mutex
extern mutex_t tsms_mutex;         // TSMS Mutex
// add more as necessary...

/* API */
uint8_t mutexes_init(); // Initializes all mutexes set up in u_mutexes.c

#endif /* u_mutex.h */