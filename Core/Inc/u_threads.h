#ifndef __U_THREADS_H
#define __U_THREADS_H

#include "tx_api.h"
#include "u_tx_threads.h"
#include <stdint.h>
#include <stdio.h>

/*
*   Basically just a wrapper for ThreadX stuff. Lets you create/configure threads.
*   
*   Author: Blake Jackson
*/

/* Initializes all threads. Called from app_threadx.c */
uint8_t threads_init(TX_BYTE_POOL *byte_pool);

/* Thread Functions */
void default_thread(ULONG thread_input);
void ethernet_thread(ULONG thread_input);
void can_thread(ULONG thread_input);
void faults_thread(ULONG thread_input);
void shutdown_thread(ULONG thread_input);
void statemachine_thread(ULONG thread_input);
void pedals_thread(ULONG thread_input);
void efuse_thread(ULONG thread_input);
void tsms_thread(ULONG thread_input);

#endif /* u_threads.h */