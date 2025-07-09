#include "u_ethernet.h"
#include <string.h>
#include <stdio.h>

/* PRIVATE STRUCTS */
typedef struct {
	bool is_initialized;
	uint8_t node_id;
    Ethernet_MessageHandler function; // Function to process messages when they get recieved.

	/* NetX/ThreadX Stuff */
	NX_IP *ip;
	NX_PACKET_POOL *packet_pool;
} _ethernet_device_t;

/* GLOBALS */
static _ethernet_device_t device = {0};
NX_UDP_SOCKET socket;

/* Callback function. Called when an ethernet message is recieved. */
static void _recieve_message(NX_UDP_SOCKET *socket) {
    NX_PACKET *packet;
    ULONG bytes_copied;
    uint8_t status;
    ethernet_message_t message = {0};

    /* Recieve the packet */
    status = nx_udp_socket_receive(socket, &packet, TX_NO_WAIT);

    if(status == NX_SUCCESS) {
        /* Extract message from packet */
        status = nx_packet_data_extract_offset(
            packet,                         // Packet to extract from
            0,                              // Offset (start of packet)
            &message,                       // Message buffer
            sizeof(ethernet_message_t),     // Size to extract
            &bytes_copied                   // Stores how many bytes were actually copied to &message
        );
        if(bytes_copied < sizeof(ethernet_message_t)) {
            printf("[u_ethernet.c/_recieve_message()] WARNING: Received ethernet message was smaller than expected (only recieved %lu of %u expected bytes).\n", bytes_copied, sizeof(ethernet_message_t));
        }

        /* Process recieved message */
        if(status == NX_SUCCESS) {
            printf("[u_ethernet.c/_recieve_message()] Recieved ethernet message! (Sender ID: %d, Message ID: %d).\n", message.sender_id, message.message_id);
            queue_send(&eth_incoming, &message);
        }
    }

    /* Release the packet */
    nx_packet_release(packet);
}

/* Sends an ethernet message (i.e. sends a UDP packet). */
static uint8_t _send_message(uint8_t message_id, ethernet_node_t recipient_id, uint8_t *data, uint8_t data_length) {
    NX_PACKET *packet;
    uint8_t status;
    ethernet_message_t message = {0};

    /* Check if ethernet is initialized */
    if(!device.is_initialized) {
        printf("[u_ethernet.c/_send_message()] ERROR: Ethernet device is not initialized, so ethernet_send_message() will not work.\n");
        return U_ERROR;
    }

    /* Check data length */
    if (data_length > ETH_MAX_PACKET_SIZE) {
        printf("[u_ethernet.c/_send_message()] ERROR: Data length exceeds maximum.\n");
        return U_ERROR;
    }

    /* Prepare message */
    message.sender_id = device.node_id;
    message.recipient_id = recipient_id;
    message.message_id = message_id;
    message.data_length = data_length;
    memcpy(message.data, data, data_length);

    /* Allocate a packet */
    status = nx_packet_allocate(
        device.packet_pool,         // Packet pool
        &packet,                    // Packet
        NX_UDP_PACKET,              // Packet type
        TX_WAIT_FOREVER             // Wait indefinitely until a packet is available
    );
    if(status != NX_SUCCESS) {
        printf("[u_ethernet.c/_send_message()] ERROR: Failed to allocate packet (Status: %d).\n", status);
        return U_ERROR;
    }

    /* Append message data to packet */
    status = nx_packet_data_append(
        packet,                     // Packet
        &message,                   // Data to append
        sizeof(ethernet_message_t), // Size of data
        device.packet_pool,         // Packet pool
        TX_WAIT_FOREVER             // Wait indefinitely
    );
    if(status != NX_SUCCESS) {
        printf("[u_ethernet.c/_send_message()] ERROR: Failed to append data to packet (Status: %d).\n", status);
        nx_packet_release(packet);
        return U_ERROR;
    }

    /* Send message */
    status = nx_udp_socket_send(
        &socket,
        packet,
        ETH_IP(recipient_id),
        ETH_UDP_PORT
    );
    if(status != NX_SUCCESS) {
        printf("[u_ethernet.c/_send_message()] ERROR: Failed to send packet (Status: %d).\n", status);
        nx_packet_release(packet);
        return U_ERROR;
    }

    printf("[u_ethernet.c/_send_message()] Sent ethernet message (Recipient ID: %d, Message ID: %d).\n", message.sender_id, message.message_id);
    return U_SUCCESS;
}

/* API FUNCTIONS */

uint8_t ethernet_init(NX_IP *ip, NX_PACKET_POOL *packet_pool, ethernet_node_t node_id, Ethernet_MessageHandler function) {
    
    uint8_t status;

    /* Make sure pointers are real instead of fake */
    if(!ip || !packet_pool) {
        printf("[u_ethernet.c/ethernet_init()] ERROR: Invalid pointer parameters.\n");
        return U_ERROR;
    }

    /* Make sure device isn't already initialized */
    if(device.is_initialized) {
        printf("[u_ethernet.c/ethernet_init()] ERROR: Ethernet is already initialized.\n");
        return U_ERROR;
    }

    /* Store device info */
    device.node_id = node_id;
    device.ip = ip;
    device.packet_pool = packet_pool;
    device.function = function;

    /* Enable UDP */
    status = nx_udp_enable(device.ip);
    if (status != NX_SUCCESS) {
        printf("[u_ethernet.c/ethernet_init()] ERROR: Failed to enable UDP (Status: %d).\n", status);
        return U_ERROR;
    }

    /* Enable igmp */
    status = nx_igmp_enable(ip);
    if (status != NX_SUCCESS) {
        printf("[u_ethernet.c/ethernet_init()] ERROR: Failed to enable igmp (Status: %d).\n", status);
        return U_ERROR;
    }

    /* Set up multicast groups. 
    *  (This iterates through every possible node combination between 0b00000001 and 0b11111111.
    *  If any of the combinations include device.node_id, that combination gets added as a multicast group.
    *  This ensures that ethernet messages can be sent to all possible combinations of recipients.)
    * 
    *  Note: This is probably pretty inefficient. I did it so you don't have to manually set up
    *        multicast groups any time you want to send a message to a new combination of nodes,
    *        but if this setup ends up being too slow or something, feel free to get rid of it.
    */
    for(int i = (1 << 0); i < (1 << 8); i++) {
        if((i & device.node_id) == device.node_id) {
            ULONG address = ETH_IP(i);
            status = nx_igmp_multicast_join(device.ip, address);
            if(status != NX_SUCCESS) {
                printf("[u_ethernet.c/ethernet_init()] ERROR: Failed to join multicast group (Status: %d, Address: %lu).\n", status, address);
            }
        }
    }

    /* Create UDP socket for broadcasting */
    status = nx_udp_socket_create(
        ip,                         // IP instance
        &socket,                    // Socket to create
        "Ethernet Broadcast",       // Socket name
        NX_IP_NORMAL,               // Type of service
        NX_FRAGMENT_OKAY,           // Fragment flag
        NX_IP_TIME_TO_LIVE,         // Time to live
        ETH_QUEUE_SIZE + 2          // Queue size (slightly larger than the application-level queue just in case processing takes a while or something.)
    );
    if(status != NX_SUCCESS) {
        printf("[u_ethernet.c/ethernet_init()] ERROR: Failed to create UDP socket (Status: %d).\n", status);
        return U_ERROR;
    }

    /* Bind socket to broadcast port */
    status = nx_udp_socket_bind(
        &socket,                     // Socket to bind
        ETH_UDP_PORT,                // Port
        TX_WAIT_FOREVER              // Wait forever
    );
    if(status != NX_SUCCESS) {
        printf("[u_ethernet.c/ethernet_init()] ERROR: Failed to bind UDP socket (Status: %d).\n", status);
        nx_udp_socket_delete(&socket);
        return U_ERROR;
    }

    /* Enable UDP recieve callback */
    status = nx_udp_socket_receive_notify(
        &socket,                      // Socket to set callback for
        &_recieve_message             // Callback function
    );
    if(status != NX_SUCCESS) {
        printf("[u_ethernet.c/ethernet_init()] ERROR: Failed to set recieve callback (Status: %d).\n", status);
        nx_udp_socket_unbind(&socket);
        nx_udp_socket_delete(&socket);
        return U_ERROR;
    }

    /* Mark device as initialized. */
    device.is_initialized = true;

    printf("[u_ethernet.c/ethernet_init()] Ethernet initialized successfully!\n");
    return U_SUCCESS;
}

uint8_t ethernet_queue_message(uint8_t message_id, ethernet_node_t recipient_id, uint8_t *data, uint8_t data_length) {
    uint8_t status;
    ethernet_message_t message = {0};

    /* Prepare message */
    message.sender_id = device.node_id;
    message.recipient_id = recipient_id;
    message.message_id = message_id;
    message.data_length = data_length;
    memcpy(message.data, data, data_length);

    /* Place message in outgoing queue */
    status = queue_send(&eth_outgoing, &message);
    if(status != U_SUCCESS) {
        printf("[u_ethernet.c/ethernet_queue_message()] ERROR: Failed to place message in outgoing queue (Message ID: %d).\n", message.message_id);
        return U_ERROR;
    }

    return U_SUCCESS;
}

/* This function is basically just the Ethernet thread. It gets called repeatedly by the NetX thread in app_netxduo.c */
uint8_t ethernet_process(void) {
    ethernet_message_t message;
    uint8_t status;

    if(!device.is_initialized) {
        printf("[u_ethernet.c/ethernet_process()] ERROR: Ethernet device is not initialized. Skipping ethernet thread stuff until then...\n");
        return U_ERROR;
    }

    /* Process outgoing messages */
    while(queue_receive(&eth_outgoing, &message) == U_SUCCESS) {
        status = _send_message(message.message_id, message.recipient_id, message.data, message.data_length);
        if(status != U_SUCCESS) {
            printf("[u_ethernet.c/ethernet_process()] WARNING: Failed to send message after removing from outgoing queue (Message ID: %d).\n", message.message_id);
            // u_TODO - maybe add the message back into the queue if it fails to send? not sure if this is a good idea tho
        }
    }

    /* Process incoming messages */
    while(queue_receive(&eth_incoming, &message) == U_SUCCESS) {
        device.function(&message);
    }

    return U_SUCCESS;
}
