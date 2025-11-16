#pragma once

#include "nx_stm32_eth_driver.h"
#include "u_nx_ethernet.h"
#include "main.h"
#include "u_queues.h"

/* API */
int ethernet1_init(void);
void ethernet_inbox(ethernet_message_t *message);