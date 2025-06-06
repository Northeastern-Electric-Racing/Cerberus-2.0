#include "ethernet.h"

uint8_t ethernet_init(NX_IP *ip_pointer, NX_PACKET_POOL *packet_pool) {
    return 0;
}

uint8_t ethernet_process(void) {
    return 0;
}

uint8_t ethernet_send_message(UCHAR *message, UINT message_length, UCHAR *destination_ip_address) {
    return 0;
}

void ethernet_recieve_callback(NX_UDP_SOCKET *socket_ptr) {
    return;
}

