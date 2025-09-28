#ifndef __U_GENERAL_H
#define __U_GENERAL_H

#include <string.h>
#include <stdio.h>
#include "tx_port.h"
#include "stm32h5xx_hal.h"

/* Converts a NetX status macro to a printable string. */
const char* nx_status_toString(UINT status); // u_TODO - move this to embedded-base once a netX/ directory is made.

#endif /* u_general.h */