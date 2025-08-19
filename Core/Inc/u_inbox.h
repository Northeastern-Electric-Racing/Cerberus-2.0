#ifndef __U_INBOX_H
#define __U_INBOX_H

#include "u_ethernet.h"
#include "fdcan.h"

void inbox_ethernet(ethernet_message_t *message); /* Function to process received ethernet messages. */
void inbox_can(can_msg_t *message); /* Function to process received CAN messages. */

#endif /* u_inbox.h */