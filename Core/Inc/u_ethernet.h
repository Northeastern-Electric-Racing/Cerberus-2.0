#ifndef __U_ETHERNET_H
#define __U_ETHERNET_H

#include "nx_api.h"
#include "u_config.h"
#include "u_queues.h"
#include <stdint.h>
#include <stdbool.h>

/*
*   Basically just a wrapper for NetX stuff.
*   
*   Author: Blake Jackson
*/

/* CONFIG
*  (This stuff should be consistent across the whole network.)
*/
#define ETH_UDP_PORT  		2006 /* UDP port for communication */
#define ETH_MAX_PACKET_SIZE	8 /* Maximum ethernet packet size */ // u_TODO - i made this 8 bytes for consistency with CAN, but obv it can be a lot larger.
#define ETH_QUEUE_SIZE 		10 /* Number of packets to queue */
#define ETH_THREAD_DELAY  	10 /* Delay for the NetX thread (in ticks) */

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
#define ETH_IP(node) IP_ADDRESS(239,0,0,node)
/* END CONFIG
*/

typedef struct {
	uint8_t sender_id;
	uint8_t recipient_id;
	uint8_t message_id;
	uint8_t data_length;
	uint8_t data[ETH_MAX_PACKET_SIZE];
} ethernet_message_t;

typedef void (*Ethernet_MessageHandler)(ethernet_message_t *message);

/**
 * @brief Initializes the NetX ethernet system in a repo. Inteded to be called from nx_app_thread_entry() in app_netxduo.c
 * @param ip Pointer to the NetX IP instance.
 * @param packet_pool Pointer to the NetX packet pool instance.
 * @param node_id The ID (ethernet_node_t) of this node.
 * @param function The function to be called when a message is recieved. See the Ethernet_MessageHandler function prototype above.
 * @return Status.
 */
uint8_t ethernet_init(NX_IP *ip, NX_PACKET_POOL *packet_pool, ethernet_node_t node_id, Ethernet_MessageHandler function);

/**
 * @brief Places an ethernet message in the outgoing queue (which will send the message).
 * @param message_id The ID of the ethernet message.
 * @param recipient_id The ID(s) of the inteded recipients.
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