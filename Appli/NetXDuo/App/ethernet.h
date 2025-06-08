#ifndef __ETHERNET_H
#define __ETHERNET_H

#include "nx_api.h"
#include <stdint.h>
#include <stdbool.h>

/*
*   Basically just a wrapper for NetX stuff.
*   
*   Author: Blake Jackson
*/

/* CONFIG
*  (This stuff applies to the whole network, and can be edited if needed.)
*/
#define ETHERNET_UDP_PORT  			2006 /* UDP port for communication */
#define ETHERNET_BROADCAST_IP 		IP_ADDRESS(255,255,255,255) /* IP address for broadcasting */
#define ETHERNET_MAX_PACKET_SIZE	8 /* Maximum ethernet packet size */
#define ETHERNET_QUEUE_SIZE 		10 /* Number of packets to queue */
#define ETHERNET_THREAD_DELAY  		10 /* Delay for the NetX thread (in ticks) */

typedef enum {
	ALL = 0,
	VCU = 1,
	Compute = 2,
	TPU = 3,
} ethernet_node_t;
/*
*	TODO - ethernet doesn't have hardware filtering like CAN, so i've made it so the intended recipient of messages can be specified when they're sent.
*	If this ends up being jank here's some other ideas:
*	1. Add 'uint8_t filter[10]' to __ethernet_device_t, which would store any CAN IDs that need to be recieved. This could be configured in ethernet_init(). Any recieved messages would be checked against this array (in a for loop), and would be added to the queue if their IDs match.
*	2. No filtering at all. Every message recieved gets added to the inbound queue since all dequeued messages get their IDs checked anyway.
*/
/* END CONFIG
*/

typedef struct {
	uint8_t sender_id;
	uint8_t recipient_id;
	uint8_t message_id;
	uint8_t data_length;
	uint8_t data[ETHERNET_MAX_PACKET_SIZE];
} ethernet_message_t;

typedef void (*Ethernet_MessageHandler)(ethernet_message_t *message);

/* API FUNCTIONS */
uint8_t ethernet_init(NX_IP *ip, NX_PACKET_POOL *packet_pool, ethernet_node_t node_id, Ethernet_MessageHandler function);
uint8_t ethernet_queue_message(uint8_t message_id, uint8_t recipient_id, uint8_t *data, uint8_t data_length);
uint8_t ethernet_process(void);

#define ETH_STATUS_OK 				0
#define ETH_STATUS_ERROR 			1
#define ETH_STATUS_WARNING 			2
#define ETH_STATUS_QUEUEEMPTY 		3
#define ETH_STATUS_QUEUENOTEMPTY 	4

#endif /* ethernet.h */