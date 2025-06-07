#ifndef __ETHERNET_H
#define __ETHERNET_H

#include "nx_api.h"
#include <stdint.h>
#include <stdbool.h>

/*
*   This is a CUSTOM FILE (i.e. not CubeMX-generated).
*   It's basically just a wrapper for NetX stuff.
*   
*   Author: Blake Jackson
*/

/* CONFIG
*  (This stuff applies to the whole network, and can be edited if needed.)
*/
#define ETHERNET_UDP_PORT  			2006 /* UDP port for communication */
#define ETHERNET_BROADCAST_IP 		IP_ADDRESS(255,255,255,255) /* IP address for broadcasting */
#define ETHERNET_MAX_PACKET_SIZE	8 /* Maximum ethernet packet size */
#define ETHERNET_RECEIVE_QUEUE_SIZE 10 /* Number of packets to queue */
#define ETHERNET_THREAD_DELAY  		10 /* Delay for the NetX thread (in ticks) */

typedef enum {
	VCU = 0,
	BMS = 1,
	MSB = 2,
} ethernet_node_t;
/* END CONFIG
*/

#define ETH_STATUS_OK 0
#define ETH_STATUS_ERROR 1

typedef struct {
	bool is_initialized;
	uint8_t node_id;

	/* NetX Stuff */
	NX_IP *ip;
	NX_PACKET_POOL *packet_pool;
} ethernet_device_t;

typedef struct {
	uint8_t sender_id;
	uint8_t recipient_id;
	uint8_t message_id;
	uint8_t data_length;
	uint8_t data[ETHERNET_MAX_PACKET_SIZE];
} ethernet_message_t;

uint8_t ethernet_init(NX_IP *ip, NX_PACKET_POOL *packet_pool, ethernet_node_t node_id);
uint8_t ethernet_process(void);
uint8_t ethernet_send_message(uint8_t message_id, uint8_t recipient_id, uint8_t *data, uint8_t data_length);

#endif /* ethernet.h */