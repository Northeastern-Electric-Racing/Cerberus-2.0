#include "tx_api.h"
#include <stdint.h>
#include <stdio.h>

/* STATUS MACROS */
#define THREADS_STATUS_OK    0
#define THREADS_STATUS_ERROR 1

/* Initializes all threads. Called from app_threadx.c */
uint8_t threads_init(TX_BYTE_POOL *byte_pool);

typedef struct {
    TX_THREAD *thread;                    /* Thread */
    CHAR      *name;                      /* Instance */
    VOID      (*function)(ULONG);         /* Thread Function */
    ULONG     thread_input;               /* Thread Input. Defaults to zero. */
    ULONG     size;                       /* Stack Size (in bytes) */
    UINT      priority;                   /* Priority */
    UINT      threshold;                  /* Preemption Threshold */
    ULONG     time_slice;                 /* Time Slice */
    UINT      auto_start;                 /* Auto Start */
    UINT      sleep;                      /* Sleep (in ticks) */
} thread_t;

/* Thread Functions */
VOID thread_template(ULONG thread_input);