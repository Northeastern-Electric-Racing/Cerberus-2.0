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
void vDefault(ULONG thread_input);
void vEthernetIncoming(ULONG thread_input);
void vEthernetOutgoing(ULONG thread_input);
void vCANIncoming(ULONG thread_input);
void vCANOutgoing(ULONG thread_input);
void vFaults(ULONG thread_input);
void vFaultsQueue(ULONG thread_input);
void vShutdown(ULONG thread_input);
void vStatemachine(ULONG thread_input);
void vPedals(ULONG thread_input);
void vEFuses(ULONG thread_input);
void vTSMS(ULONG thread_input);
void vMux(ULONG thread_input);
void vPeripherals(ULONG thread_input);

#endif /* u_threads.h */