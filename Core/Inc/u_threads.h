#ifndef __U_THREADS_H
#define __U_THREADS_H

#include "tx_api.h"
#include "u_config.h"
#include <stdint.h>
#include <stdio.h>

/*
*   Basically just a wrapper for ThreadX stuff. Lets you create/configure threads.
*   
*   Author: Blake Jackson
*/

/* Initializes all threads. Called from app_threadx.c */
uint8_t threads_init(TX_BYTE_POOL *byte_pool);

typedef struct {
    /* Thread configuration settings. Set when defining an instance of this struct. */
    CHAR            *name;                /* Name of Thread */
    VOID            (*function)(ULONG);   /* Thread Function */
    const ULONG     thread_input;         /* Thread Input. You can put whatever you want in here. Defaults to zero. */
    const ULONG     size;                 /* Stack Size (in bytes) */
    const UINT      priority;             /* Priority */
    const UINT      threshold;            /* Preemption Threshold */
    const ULONG     time_slice;           /* Time Slice */
    const UINT      auto_start;           /* Auto Start */
    const UINT      sleep;                /* Sleep (in ticks) */

    /* The actual thread instance. */
    /* Since this is an internal ThreadX thing, it should only be modified using the ThreadX API functions. */
    TX_THREAD _TX_THREAD;
} thread_t;

/* Thread Functions */
void default_thread(ULONG thread_input);
void ethernet_thread(ULONG thread_input);
void can_thread(ULONG thread_input);
void faults_thread(ULONG thread_input);

#endif /* u_threads.h */