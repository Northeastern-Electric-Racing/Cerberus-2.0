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
    TX_THREAD *thread;                    /* Thread */
    CHAR      *name;                      /* Name of Thread */
    VOID      (*function)(ULONG);         /* Thread Function */
    ULONG     thread_input;               /* Thread Input. You can put whatever you want in here. Defaults to zero. */
    ULONG     size;                       /* Stack Size (in bytes) */
    UINT      priority;                   /* Priority */
    UINT      threshold;                  /* Preemption Threshold */
    ULONG     time_slice;                 /* Time Slice */
    UINT      auto_start;                 /* Auto Start */
    UINT      sleep;                      /* Sleep (in ticks) */
} thread_t;

/* Thread Functions */
void default_thread(ULONG thread_input);
void ethernet_thread(ULONG thread_input);
void can1_thread(ULONG thread_input);

#endif /* u_threads.h */