#ifndef __U_ETHERNET_H
#define __U_ETHERNET_H

#include "u_config.h"
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
 * @brief Creates an ethernet message. Can be send with ethernet_send_message(), or added to a queue.
 * @param recipient_id The ID of the recipient node.
 * @param message_id The ID of the message.
 * @param data Pointer to the data to include in the message.
 * @param data_length Length of the data in bytes.
 * @return The created ethernet message.
 */
ethernet_message_t ethernet_create_message(uint8_t message_id, ethernet_node_t recipient_id, uint8_t *data, uint8_t data_length);

/**
 * @brief Sends an ethernet message.
 * @param message The message to send.
 * @return Status.
 */
uint8_t ethernet_send_message(ethernet_message_t *message);

#endif /* u_ethernet.h */