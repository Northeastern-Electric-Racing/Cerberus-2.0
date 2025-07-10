#ifndef __U_ETHERNET_H
#define __U_ETHERNET_H

#include "u_config.h"
#include "u_queues.h"
#include <stdint.h>
#include <stdbool.h>

/*
*   Basically just a wrapper for NetX stuff.
*   
*   Author: Blake Jackson
*/

/* CONFIG */
#define ETH_UDP_PORT  		2006 /* UDP port for communication */
#define ETH_MESSAGE_SIZE	8    /* Maximum ethernet message size */ // u_TODO - i made this 8 bytes for consistency with CAN, but obv it can be a lot larger.
#define ETH_MAX_PACKETS     10   /* Maximum number of packets we wanna handle simultaneously */

typedef enum {
    VCU         = (1 << 0), // 0b00000001
    COMPUTE     = (1 << 1), // 0b00000010
    TPU         = (1 << 2), // 0b00000100
    NODE4       = (1 << 3), // 0b00001000
    NODE5       = (1 << 4), // 0b00010000
    NODE6       = (1 << 5), // 0b00100000
    NODE7       = (1 << 6), // 0b01000000
    NODE8       = (1 << 7), // 0b10000000
} ethernet_node_t;
#define ETH_IP(node) IP_ADDRESS(239,0,0,node) // u_TODO - you can configure ethernet IPs in CubeMX apparently. probably should look into that, not sure how that works w/ this
/* END CONFIG */

typedef struct {
	uint8_t sender_id;
	uint8_t recipient_id;
	uint8_t message_id;
	uint8_t data_length;
	uint8_t data[ETH_MESSAGE_SIZE];
} ethernet_message_t;

/**
 * @brief Initializes the NetX ethernet system in a repo.
 * @param node_id The ID (ethernet_node_t) of this node.
 * @return Status.
 */
uint8_t ethernet_init(ethernet_node_t node_id);

/**
 * @brief Sends an ethernet message.
 * @param message_id The ID of the ethernet message.
 * @param recipient_id The ID(s) of the intended recipients.
 * @param data The data to be sent in the message.
 * @param data_length The length of the data, in bytes.
 * @return Status.
 */
uint8_t ethernet_send_message(uint8_t message_id, ethernet_node_t recipient_id, uint8_t *data, uint8_t data_length);

/**
 * @brief Places an ethernet message in the outgoing queue (which will send the message).
 * @param message_id The ID of the ethernet message.
 * @param recipient_id The ID(s) of the intended recipients.
 * @param data The data to be sent in the message.
 * @param data_length The length of the data, in bytes.
 * @return Status.
 */
uint8_t ethernet_queue_message(uint8_t message_id, ethernet_node_t recipient_id, uint8_t *data, uint8_t data_length);

/**
 * @brief Handles all ethernet processing (sending outgoing messages, recieving incoming messages, etc.).
 *        This function is intended to be called repeatetly by the NetX thread.
 */
uint8_t ethernet_process(void);

#endif /* u_ethernet.h */