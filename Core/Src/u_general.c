#include "u_general.h"
#include "tx_api.h"
#include "nx_api.h"
#include "stm32h5xx_hal.h"
#include <stdio.h>

/* Converts a NetX status macro to a printable string. */
/* This function is intended to be used with DEBUG_PRINTLN(), and shouldn't really ever be used outside of debugging purposes. */
/* (these macros are defined in nx_api.h) */
const char* nx_status_toString(UINT status) {
    switch(status) {
        case NX_SUCCESS: return "NX_SUCCESS";
        case NX_NO_PACKET: return "NX_NO_PACKET";
        case NX_UNDERFLOW: return "NX_UNDERFLOW";
        case NX_OVERFLOW: return "NX_OVERFLOW";
        case NX_NO_MAPPING: return "NX_NO_MAPPING";
        case NX_DELETED: return "NX_DELETED";
        case NX_POOL_ERROR: return "NX_POOL_ERROR";
        case NX_PTR_ERROR: return "NX_PTR_ERROR";
        case NX_WAIT_ERROR: return "NX_WAIT_ERROR";
        case NX_SIZE_ERROR: return "NX_SIZE_ERROR";
        case NX_OPTION_ERROR: return "NX_OPTION_ERROR";
        case NX_DELETE_ERROR: return "NX_DELETE_ERROR";
        case NX_CALLER_ERROR: return "NX_CALLER_ERROR";
        case NX_INVALID_PACKET: return "NX_INVALID_PACKET";
        case NX_INVALID_SOCKET: return "NX_INVALID_SOCKET";
        case NX_NOT_ENABLED: return "NX_NOT_ENABLED";
        case NX_ALREADY_ENABLED: return "NX_ALREADY_ENABLED";
        case NX_ENTRY_NOT_FOUND: return "NX_ENTRY_NOT_FOUND";
        case NX_NO_MORE_ENTRIES: return "NX_NO_MORE_ENTRIES";
        case NX_ARP_TIMER_ERROR: return "NX_ARP_TIMER_ERROR";
        case NX_RESERVED_CODE0: return "NX_RESERVED_CODE0";
        case NX_WAIT_ABORTED: return "NX_WAIT_ABORTED";
        case NX_IP_INTERNAL_ERROR: return "NX_IP_INTERNAL_ERROR";
        case NX_IP_ADDRESS_ERROR: return "NX_IP_ADDRESS_ERROR";
        case NX_ALREADY_BOUND: return "NX_ALREADY_BOUND";
        case NX_PORT_UNAVAILABLE: return "NX_PORT_UNAVAILABLE";
        case NX_NOT_BOUND: return "NX_NOT_BOUND";
        case NX_RESERVED_CODE1: return "NX_RESERVED_CODE1";
        case NX_SOCKET_UNBOUND: return "NX_SOCKET_UNBOUND";
        case NX_NOT_CREATED: return "NX_NOT_CREATED";
        case NX_SOCKETS_BOUND: return "NX_SOCKETS_BOUND";
        case NX_NO_RESPONSE: return "NX_NO_RESPONSE";
        case NX_POOL_DELETED: return "NX_POOL_DELETED";
        case NX_ALREADY_RELEASED: return "NX_ALREADY_RELEASED";
        case NX_RESERVED_CODE2: return "NX_RESERVED_CODE2";
        case NX_MAX_LISTEN: return "NX_MAX_LISTEN";
        case NX_DUPLICATE_LISTEN: return "NX_DUPLICATE_LISTEN";
        case NX_NOT_CLOSED: return "NX_NOT_CLOSED";
        case NX_NOT_LISTEN_STATE: return "NX_NOT_LISTEN_STATE";
        case NX_IN_PROGRESS: return "NX_IN_PROGRESS";
        case NX_NOT_CONNECTED: return "NX_NOT_CONNECTED";
        case NX_WINDOW_OVERFLOW: return "NX_WINDOW_OVERFLOW";
        case NX_ALREADY_SUSPENDED: return "NX_ALREADY_SUSPENDED";
        case NX_DISCONNECT_FAILED: return "NX_DISCONNECT_FAILED";
        case NX_STILL_BOUND: return "NX_STILL_BOUND";
        case NX_NOT_SUCCESSFUL: return "NX_NOT_SUCCESSFUL";
        case NX_UNHANDLED_COMMAND: return "NX_UNHANDLED_COMMAND";
        case NX_NO_FREE_PORTS: return "NX_NO_FREE_PORTS";
        case NX_INVALID_PORT: return "NX_INVALID_PORT";
        case NX_INVALID_RELISTEN: return "NX_INVALID_RELISTEN";
        case NX_CONNECTION_PENDING: return "NX_CONNECTION_PENDING";
        case NX_TX_QUEUE_DEPTH: return "NX_TX_QUEUE_DEPTH";
        case NX_NOT_IMPLEMENTED: return "NX_NOT_IMPLEMENTED";
        case NX_NOT_SUPPORTED: return "NX_NOT_SUPPORTED";
        case NX_INVALID_INTERFACE: return "NX_INVALID_INTERFACE";
        case NX_INVALID_PARAMETERS: return "NX_INVALID_PARAMETERS";
        case NX_NOT_FOUND: return "NX_NOT_FOUND";
        case NX_CANNOT_START: return "NX_CANNOT_START";
        case NX_NO_INTERFACE_ADDRESS: return "NX_NO_INTERFACE_ADDRESS";
        case NX_INVALID_MTU_DATA: return "NX_INVALID_MTU_DATA";
        case NX_DUPLICATED_ENTRY: return "NX_DUPLICATED_ENTRY";
        case NX_PACKET_OFFSET_ERROR: return "NX_PACKET_OFFSET_ERROR";
        case NX_OPTION_HEADER_ERROR: return "NX_OPTION_HEADER_ERROR";
        case NX_CONTINUE: return "NX_CONTINUE";
        case NX_TCPIP_OFFLOAD_ERROR: return "NX_TCPIP_OFFLOAD_ERROR";
        default: return "UNKNOWN_STATUS";
    }
}