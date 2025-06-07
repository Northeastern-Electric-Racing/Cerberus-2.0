#include "ethernet.h"

/* GLOBALS */
ethernet_device_t device = {0};
NX_UDP_SOCKET socket;

/* Called when an ethernet message is recieved. */
static void ethernet_recieve_callback(NX_UDP_SOCKET *socket) {
    NX_PACKET *packet;
    ULONG bytes_copied;
    uint8_t status;

    /* Recieve the packet */
    status = nx_udp_socket_receive(socket, &packet, TX_NO_WAIT);

    if(status == NX_SUCCESS) {
        /* Extract message from packet */
        ethernet_message_t message = {0};
        status = nx_packet_data_extract_offset(
            packet,                         // Packet to extract from
            0,                              // Offset (start of packet)
            &message,                       // Message buffer
            sizeof(ethernet_message_t),     // Size to extract
            &bytes_copied                   // Stores how many bytes were actually copied to &message
        );
        if(bytes_copied < sizeof(ethernet_message_t)) {
            printf("[ethernet.c/ethernet_recieve_callback()] WARNING: Received ethernet message was smaller than expected (only recieved %lu of %u expected bytes).\n", bytes_copied, sizeof(ethernet_message_t));
        }

        /* Process recieved message */
        if(status == NX_SUCCESS) {
            printf("[ethernet.c/ethernet_recieve_callback()] Recieved ethernet message! (Sender ID: %d, Message ID: %d).\n", message.sender_id, message.message_id);
            if(message.recipient_id == device.node_id) {
                printf("[ethernet.c/ethernet_recieve_callback()] Ethernet message was intended for this device! Processing...\n");
                // TODO - process stuff
            }
        }
    }

    /* Release the packet */
    nx_packet_release(packet);
}

uint8_t ethernet_init(NX_IP *ip, NX_PACKET_POOL *packet_pool, ethernet_node_t node_id) {
    
    uint8_t status;

    /* Make sure pointers are real instead of fake */
    if(!ip || !packet_pool) {
        printf("[ethernet.c/ethernet_init()] ERROR: Invalid pointer parameters.\n");
        return ETH_STATUS_ERROR;
    }

    /* Make sure device isn't already initialized */
    if(device.is_initialized) {
        printf("[ethernet.c/ethernet_init()] ERROR: Ethernet is already initialized.\n");
        return ETH_STATUS_ERROR;
    }

    /* Store device info */
    device.node_id = node_id;
    device.ip = ip;
    device.packet_pool = packet_pool;

    /* Create UDP socket for broadcasting */
    status = nx_udp_socket_create(
        ip,                             // IP instance
        &socket,                        // Socket to create
        "Ethernet Broadcast",           // Socket name
        NX_IP_NORMAL,                   // Type of service
        NX_FRAGMENT_OKAY,               // Fragment flag
        5,                              // Time to live - TODO - i do not know what this is so prolly should double check
        ETHERNET_RECEIVE_QUEUE_SIZE     // Queue size
    );
    if(status != NX_SUCCESS) {
        printf("[ethernet.c/ethernet_init()] ERROR: Failed to create UDP socket (Status: %d).\n", status);
        return ETH_STATUS_ERROR;
    }

    /* Bind socket to broadcast port */
    status = nx_udp_socket_bind(
        &socket,                        // Socket to bind
        ETHERNET_UDP_PORT,              // Port
        TX_WAIT_FOREVER                 // Wait forever
    );
    if(status != NX_SUCCESS) {
        printf("[ethernet.c/ethernet_init()] ERROR: Failed to bind UDP socket (Status: %d).\n", status);
        nx_udp_socket_delete(&socket);
        return ETH_STATUS_ERROR;
    }

    /* Enable UDP recieve callback */
    status = nx_udp_socket_receive_notify(
        &socket,                        // Socket to set callback for
        &ethernet_recieve_callback      // Callback function
    );
    if(status != NX_SUCCESS) {
        printf("[ethernet.c/ethernet_init()] ERROR: Failed to set recieve callback (Status: %d).\n", status);
        nx_udp_socket_unbind(&socket);
        nx_udp_socket_delete(&socket);
        return ETH_STATUS_ERROR;
    }

    /* Mark device as initialized. */
    device.is_initialized = true;

    printf("[ethernet.c/ethernet_init()] Ethernet initialized successfully!\n");
    return ETH_STATUS_OK;
}

uint8_t ethernet_process(void) {
    // TODO - i gotta implement this
    return ETH_STATUS_OK;
}

uint8_t ethernet_send_message(uint8_t message_id, uint8_t recipient_id, uint8_t *data, uint8_t data_length) {
    NX_PACKET *packet;
    uint8_t status;
    ethernet_message_t message = {0};

    /* Check if ethernet is initialized */
    if(!device.is_initialized) {
        printf("[ethernet.c/ethernet_send_message()] ERROR: Ethernet device is not initialized, so ethernet_send_message() will not work.\n");
        return ETH_STATUS_ERROR;
    }

    /* Check data length */
    if (data_length > ETHERNET_MAX_PACKET_SIZE) {
        printf("[ethernet.c/ethernet_send_message()] ERROR: Data length exceeds maximum.\n");
        return ETH_STATUS_ERROR;
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
        printf("[ethernet.c/ethernet_send_message()] ERROR: Failed to allocate packet (Status: %d).\n", status);
        return ETH_STATUS_ERROR;
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
        printf("[ethernet.c/ethernet_send_message()] ERROR: Failed to append data to packet (Status: %d).\n", status);
        nx_packet_release(packet);
        return ETH_STATUS_ERROR;
    }

    /* Send message */
    status = nx_udp_socket_send(
        &socket,
        packet,
        ETHERNET_BROADCAST_IP,
        ETHERNET_UDP_PORT
    );
    if(status != NX_SUCCESS) {
        printf("[ethernet.c/ethernet_send_message()] ERROR: Failed to send packet (Status: %d).\n", status);
        nx_packet_release(packet);
        return ETH_STATUS_ERROR;
    }

    printf("[ethernet.c/ethernet_send_message()] Sent ethernet message (Recipient ID: %d, Message ID: %d).\n", message.sender_id, message.message_id);
    return ETH_STATUS_OK;
}
