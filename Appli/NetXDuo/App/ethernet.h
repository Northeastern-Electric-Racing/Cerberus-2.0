#ifndef __ETHERNET_H
#define __ETHERNET_H

#include "nx_api.h"
#include <stdint.h>

/*
*   This is a CUSTOM FILE (i.e. not CubeMX-generated).
*   It's basically just a wrapper for NetX stuff.
*   
*   Author: Blake Jackson
*/

/* CONFIG */
#define ETHERNET_UDP_PORT  2006 /* UDP port for communication */
#define MAX_PACKET_SIZE	   1500 /* Maximum ethernet packet size */
#define RECEIVE_QUEUE_SIZE 10 /* Number of packets to queue */
#define NETX_THREAD_DELAY  10 /* Delay for the NetX thread (in ticks) */

uint8_t ethernet_init(NX_IP *ip_pointer, NX_PACKET_POOL *packet_pool);
uint8_t ethernet_process(void);
uint8_t ethernet_send_message(UCHAR *message, UINT message_length,
			      UCHAR *destination_ip_address);
void ethernet_recieve_callback(NX_UDP_SOCKET *socket_ptr);

#endif /* ethernet.h */