#include "ethernet.h"

/* PRIVATE STRUCTS */
typedef struct {
	bool is_initialized;
	uint8_t node_id;
    Ethernet_MessageHandler function; // Function to process messages when they get recieved.

	/* NetX/ThreadX Stuff */
	NX_IP *ip;
	NX_PACKET_POOL *packet_pool;
} _ethernet_device_t;

typedef struct {
	ethernet_message_t messages[ETH_QUEUE_SIZE];
	uint8_t head;
	uint8_t tail;
	uint8_t count;
    TX_MUTEX mutex;
} _ethernet_queue_t;

/* GLOBALS */
static _ethernet_device_t device = {0};
static _ethernet_queue_t incoming = {0};
static _ethernet_queue_t outgoing = {0}; // TODO - maybe add a third "priority" outgoing queue
NX_UDP_SOCKET socket;

/* Adds an ethernet message to a queue. Not intended for use outside of this file. */
static uint8_t _ethernet_queue_message(_ethernet_queue_t *queue, ethernet_message_t *message) {
    
    /* Get queue mutex */
    uint8_t status = tx_mutex_get(&queue->mutex, TX_WAIT_FOREVER);
    if(status != TX_SUCCESS) {
        printf("[ethernet.c/_ethernet_queue_message()] ERROR: Failed to get queue mutex (Status: %d).\n", status);
        return ETH_STATUS_ERROR;
    }

    /* Check if queue is full. */
    if(queue->count >= ETH_QUEUE_SIZE) {
        printf("[ethernet.c/_ethernet_queue_message()] ERROR: Ethernet queue is full!\n");
        tx_mutex_put(&queue->mutex); // (Release the queue mutex.)
        return ETH_STATUS_ERROR;
    }

    /* Copy the message into the queue. */
    memcpy(&queue->messages[queue->tail], message, sizeof(ethernet_message_t));
    queue->tail = (queue->tail + 1) % ETH_QUEUE_SIZE;
    queue->count++;
    printf("[ethernet.c/_ethernet_queue_message()] Queued ethernet message (Message ID: %d).\n", message->message_id);
    tx_mutex_put(&queue->mutex); // (Release the queue mutex.)
    return ETH_STATUS_OK;
}

/* Removes an ethernet message from a queue. Not intended for use outside of this file. */
static uint8_t _ethernet_dequeue_message(_ethernet_queue_t *queue, ethernet_message_t *message) {
    
    /* Get queue mutex */
    uint8_t status = tx_mutex_get(&queue->mutex, TX_WAIT_FOREVER);
    if(status != TX_SUCCESS) {
        printf("[ethernet.c/_ethernet_dequeue_message()] ERROR: Failed to get queue mutex (Status: %d).\n", status);
        return ETH_STATUS_ERROR;
    }

    /* Check if queue is empty. */
    if (queue->count == 0) {
        printf("[ethernet.c/_ethernet_dequeue_message()] Queue is empty. No messages to dequeue.\n");
        tx_mutex_put(&queue->mutex); // (Release the queue mutex.)
        return ETH_STATUS_QUEUEEMPTY;
    }

    /* Remove the message from the queue and copy it to the message buffer. */
    memcpy(message, &queue->messages[queue->head], sizeof(ethernet_message_t));
    queue->head = (queue->head + 1) % ETH_QUEUE_SIZE;
    queue->count--;
    printf("[ethernet.c/_ethernet_dequeue_message()] Dequeued ethernet message (Message ID: %d).\n", message->message_id);
    tx_mutex_put(&queue->mutex); // (Release queue mutex.)
    return ETH_STATUS_QUEUENOTEMPTY;
}

/* Called when an ethernet message is recieved. */
static void _ethernet_recieve_callback(NX_UDP_SOCKET *socket) {
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
            printf("[ethernet.c/_ethernet_recieve_callback()] WARNING: Received ethernet message was smaller than expected (only recieved %lu of %u expected bytes).\n", bytes_copied, sizeof(ethernet_message_t));
        }

        /* Process recieved message */
        if(status == NX_SUCCESS) {
            printf("[ethernet.c/_ethernet_recieve_callback()] Recieved ethernet message! (Sender ID: %d, Message ID: %d).\n", message.sender_id, message.message_id);
            _ethernet_queue_message(&incoming, &message);
        }
    }

    /* Release the packet */
    nx_packet_release(packet);
}

/* Sends a UDP packet. */
static uint8_t _ethernet_send_message(uint8_t message_id, ethernet_node_t recipient_id, uint8_t *data, uint8_t data_length) {
    NX_PACKET *packet;
    uint8_t status;
    ethernet_message_t message = {0};

    /* Check if ethernet is initialized */
    if(!device.is_initialized) {
        printf("[ethernet.c/_ethernet_send_message()] ERROR: Ethernet device is not initialized, so ethernet_send_message() will not work.\n");
        return ETH_STATUS_ERROR;
    }

    /* Check data length */
    if (data_length > ETH_MAX_PACKET_SIZE) {
        printf("[ethernet.c/_ethernet_send_message()] ERROR: Data length exceeds maximum.\n");
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
        printf("[ethernet.c/_ethernet_send_message()] ERROR: Failed to allocate packet (Status: %d).\n", status);
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
        printf("[ethernet.c/_ethernet_send_message()] ERROR: Failed to append data to packet (Status: %d).\n", status);
        nx_packet_release(packet);
        return ETH_STATUS_ERROR;
    }

    /* Send message */
    status = nx_udp_socket_send(
        &socket,
        packet,
        ETH_IP(recipient_id),
        ETH_UDP_PORT
    );
    if(status != NX_SUCCESS) {
        printf("[ethernet.c/_ethernet_send_message()] ERROR: Failed to send packet (Status: %d).\n", status);
        nx_packet_release(packet);
        return ETH_STATUS_ERROR;
    }

    printf("[ethernet.c/_ethernet_send_message()] Sent ethernet message (Recipient ID: %d, Message ID: %d).\n", message.sender_id, message.message_id);
    return ETH_STATUS_OK;
}

/* API FUNCTIONS */

uint8_t ethernet_init(NX_IP *ip, NX_PACKET_POOL *packet_pool, ethernet_node_t node_id, Ethernet_MessageHandler function) {
    
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
    device.function = function;

    /* Enable UDP */
    status = nx_udp_enable(device.ip);
    if (status != NX_SUCCESS) {
        printf("[ethernet.c/ethernet_init()] ERROR: Failed to enable UDP (Status: %d).\n", status);
        return ETH_STATUS_ERROR;
    }

    /* Enable igmp */
    status = nx_igmp_enable(ip);
    if (status != NX_SUCCESS) {
        printf("[ethernet.c/ethernet_init()] ERROR: Failed to enable igmp (Status: %d).\n", status);
        return ETH_STATUS_ERROR;
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
    for(uint8_t i = (1 << 0); i < (1 << 8); i++) {
        if((i & device.node_id) == device.node_id) {
            ULONG address = ETH_IP(i);
            status = nx_igmp_multicast_join(device.ip, address);
            if(status != NX_SUCCESS) {
                printf("[ethernet.c/ethernet_init()] ERROR: Failed to join multicast group (Status: %d, Address: %lu).\n", status, address);
            }
        }
    }

    /* Create incoming queue mutex. */
    status = tx_mutex_create(&incoming.mutex, "Incoming Queue Mutex", TX_NO_INHERIT);
    if(status != TX_SUCCESS) {
        printf("[ethernet.c/ethernet_init()] ERROR: Failed to create incoming mutex (Status: %d).\n", status);
        return ETH_STATUS_ERROR;
    }

    /* Create outgoing queue mutex. */
    status = tx_mutex_create(&outgoing.mutex, "Outgoing Queue Mutex", TX_NO_INHERIT);
    if(status != TX_SUCCESS) {
        printf("[ethernet.c/ethernet_init()] ERROR: Failed to create outgoing mutex (Status: %d).\n", status);
        return ETH_STATUS_ERROR;
    }

    /* Create UDP socket for broadcasting */
    status = nx_udp_socket_create(
        ip,                             // IP instance
        &socket,                        // Socket to create
        "Ethernet Broadcast",           // Socket name
        NX_IP_NORMAL,                   // Type of service
        NX_FRAGMENT_OKAY,               // Fragment flag
        NX_IP_TIME_TO_LIVE,             // Time to live
        ETH_QUEUE_SIZE + 2         // Queue size (slightly larger than the application-level queue just in case processing takes a while or something.)
    );
    if(status != NX_SUCCESS) {
        printf("[ethernet.c/ethernet_init()] ERROR: Failed to create UDP socket (Status: %d).\n", status);
        return ETH_STATUS_ERROR;
    }

    /* Bind socket to broadcast port */
    status = nx_udp_socket_bind(
        &socket,                        // Socket to bind
        ETH_UDP_PORT,              // Port
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
        &_ethernet_recieve_callback      // Callback function
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
    status = _ethernet_queue_message(&outgoing, &message);
    if(status != ETH_STATUS_OK) {
        printf("[ethernet.c/ethernet_queue_message()] ERROR: Failed to place message in outgoing queue (Message ID: %d).\n", message.message_id);
        return ETH_STATUS_ERROR;
    }

    return ETH_STATUS_OK;
}

uint8_t ethernet_process(void) {
    ethernet_message_t message;
    uint8_t status;

    if(!device.is_initialized) {
        printf("[ethernet.c/ethernet_process()] ERROR: Ethernet device is not initialized. Skipping ethernet thread stuff until then...\n");
        return ETH_STATUS_ERROR;
    }

    /* Process outgoing messages */
    while(_ethernet_dequeue_message(&outgoing, &message) == ETH_STATUS_QUEUENOTEMPTY) {
        status = _ethernet_send_message(message.message_id, message.recipient_id, message.data, message.data_length);
        if(status != ETH_STATUS_OK) {
            printf("[ethernet.c/ethernet_process()] WARNING: Failed to send message after removing from outgoing queue (Message ID: %d).\n", message.message_id);
            // TODO - maybe add the message back into the queue if it fails to send? not sure if this is a good idea tho
        }
    }

    /* Process incoming messages */
    while(_ethernet_dequeue_message(&incoming, &message) == ETH_STATUS_QUEUENOTEMPTY) {
        device.function(&message);
    }

    return ETH_STATUS_OK;
}
