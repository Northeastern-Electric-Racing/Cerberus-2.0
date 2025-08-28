#ifndef __U_MUTEX_H
#define __U_MUTEX_H

#include "tx_api.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    /* Mutex configuration settings. Set when defining an instance of this struct. */
    CHAR *name;                  /* Name of Mutex */
    const UINT priority_inherit; /* Specifies if the mutex supports priority inheritance. See page 55 of the "Azure RTOS ThreadX User Guide". */

    /* The actual mutex instance. */
    /* Since this is an internal ThreadX thing, it should only be modified using the ThreadX API functions. */
    TX_MUTEX _TX_MUTEX;
} mutex_t;

/* Mutex List */
extern mutex_t template_mutex; // Template Mutex
// add more as necessary...

/* API */
uint8_t mutexes_init(); // Initializes all mutexes set up in u_mutexes.c
uint8_t mutex_get(mutex_t *mutex, ULONG wait_option); // Gets a mutex. wait_option can be TX_NO_WAIT, TX_WAIT_FOREVER, or a specific number of ticks.
uint8_t mutex_put(mutex_t *mutex); // Puts a mutex.

#endif /* u_mutex.h */