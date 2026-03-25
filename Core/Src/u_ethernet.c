#include "u_ethernet.h"
#include "nx_api.h"
#include "pb.h"
#include "pb_encode.h"
#include "serverdata.pb.h"
#include "tx_api.h"
#include "u_queues.h"
#include "nx_stm32_eth_driver.h"
#include "u_nx_ethernet.h"
#include "u_nx_debug.h"
#include "main.h"
#include "u_tx_debug.h"
#include "u_tx_general.h"
#include "u_tx_queues.h"

#define ETH_TYPE_SEND 0
#define ETH_TYPE_RECV 1

/* Callback for when a ethernet message is recieved. */
void _ethernet_recieve(eth_mqtt_queue_message_t message) {
  /* Send the message to the incoming ethernet queue. */
  int status = queue_send(&eth_manager, &message, TX_NO_WAIT);
  if(status != U_SUCCESS) {
    PRINTLN_ERROR("Failed to send message to the incoming ethernet queue (Status: %d).", status);
    return;
  }
}

UINT ethernet1_mqtt_send(char* topic, uint8_t topic_size, char* unit, uint8_t unit_size, float* values, uint8_t values_len, uint64_t time_us) {
    serverdata_v2_ServerData msg =  serverdata_v2_ServerData_init_zero;
    // if (unit_size > sizeof(msg.unit)) {
    //     return U_ERROR;
    // }
    if (topic_size > ETH_MAX_TOPIC_SIZE) {
        return U_ERROR;
    }
    // if (values_len > (sizeof(msg.values) / sizeof(float))) {
    //     return U_ERROR;
    // }
    eth_mqtt_queue_message_t sendable;
    msg.time_us = time_us;
    // COPIES
    *(msg.unit) = *unit;
    *(msg.values) = *values;
    msg.values_count = values_len;
    sendable.type = ETH_TYPE_SEND;
    sendable.msg = msg;
    memcpy(sendable.topic, topic, topic_size);
    sendable.topic_size = topic_size;
    return queue_send(&eth_manager, &sendable, TX_NO_WAIT);
}

/* Initializes ethernet. */
UINT ethernet1_init(void) {
    /* PHY_RESET Pin has to be set HIGH for the PHY to function. */
    HAL_GPIO_WritePin(PHY_RESET_GPIO_Port, PHY_RESET_Pin, GPIO_PIN_SET);

    /* Init the ethernet. */
    return ethernet_init(VCU, nx_stm32_eth_driver, _ethernet_recieve);
}


void vEthernet1Manager(ULONG thread_input) {

    // all calls to the ethernet driver in this thread only to prevent race conditons
    UINT status = ethernet1_init();
    if(status != NX_SUCCESS) {
        PRINTLN_ERROR("Failed to call ethernet1_init() (Status: %d/%s).", status, nx_status_toString(status));
    }
    PRINTLN_INFO("Successfully initialized ethernet 1!");

    eth_mqtt_queue_message_t msg;
    unsigned char buffer[serverdata_v2_ServerData_size];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    while(1) {

        /* Send outgoing messages, recieve incoming messages */
        while(queue_receive(&eth_manager, &msg, TX_WAIT_FOREVER) == U_SUCCESS) {
            if (msg.type == ETH_TYPE_SEND) {
                status = pb_encode(&stream, serverdata_v2_ServerData_fields, &msg.msg);
                if (!status) {
                    PRINTLN_ERROR("Failed to serialize protobuf message: %s", PB_GET_ERROR(&stream));
                    continue;
                }
                status = ethernet_mqtt_publish(msg.topic, msg.topic_size, (char*)buffer, stream.bytes_written);
                stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
                if(status != U_SUCCESS) {
                    PRINTLN_WARNING("Failed to send Ethernet message after removing from outgoing queue: %ud", status);
                    if (status == 0x10002) {
                        do {
                            tx_thread_sleep(MS_TO_TICKS(1000));
                            status = ethernet_mqtt_reconnect();
                            PRINTLN_WARNING("ATTEMPTING MQTT RECONNECTION: Status %d", status);
                        } while (status != NX_SUCCESS);
                        PRINTLN_WARNING("MQTT RECONNECTION SUCCESS");
                    }
                    continue;
                } else {
                    PRINTLN_INFO("Sent ethernet message!");
                }
            }
        }

        /* No sleep. Thread timing is controlled completely by the queue timeout. */
    }
}
