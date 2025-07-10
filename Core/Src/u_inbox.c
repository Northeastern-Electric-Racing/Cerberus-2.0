#include "u_ethernet.h"

/* Processes received ethernet messages. */
void ethernet_inbox(ethernet_message_t *message) {
    switch(message->message_id) {
        case 0x01:
            // do thing
            break;
        case 0x02:
            // do thing
            break;
        case 0x03:
            // etc
            break;
    }
}