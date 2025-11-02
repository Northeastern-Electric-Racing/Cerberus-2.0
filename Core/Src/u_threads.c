#include "main.h"
#include "u_tx_debug.h"
#include "u_threads.h"
#include "u_queues.h"
#include "u_inbox.h"
#include "u_can.h"
#include "u_nx_ethernet.h"
#include "u_faults.h"
#include "u_pedals.h"
#include "u_adc.h"
#include "u_efuses.h"
#include "u_statemachine.h"
#include "u_tsms.h"
#include "bitstream.h"

/* Default Thread */
static thread_t default_thread = {
        .name       = "Default Thread",  /* Name */
        .size       = 512,               /* Stack Size (in bytes) */
        .priority   = 9,                 /* Priority */
        .threshold  = 0,                 /* Preemption Threshold */
        .time_slice = TX_NO_TIME_SLICE,  /* Time Slice */
        .auto_start = TX_AUTO_START,     /* Auto Start */
        .sleep      = 50,                /* Sleep (in ticks) */
        .function   = vDefault           /* Thread Function */
    };
void vDefault(ULONG thread_input) {
    
    while(1) {

        /* Kick the watchdogs (sad) )*/
        HAL_IWDG_Refresh(&hiwdg); // Internal Watchdog
        HAL_GPIO_TogglePin(WATCHDOG_GPIO_Port, WATCHDOG_Pin); // External Watchdog

        /* Sleep Thread for specified number of ticks. */
        tx_thread_sleep(default_thread.sleep);
    }
}

/* Ethernet Thread. Sends outgoing messages and processes incoming messages. */
static thread_t ethernet_thread = {
        .name       = "Ethernet Thread", /* Name */
        .size       = 512,               /* Stack Size (in bytes) */
        .priority   = 3,                 /* Priority */
        .threshold  = 0,                 /* Preemption Threshold */
        .time_slice = TX_NO_TIME_SLICE,  /* Time Slice */
        .auto_start = TX_AUTO_START,     /* Auto Start */
        .sleep      =  1,                /* Sleep (in ticks) */
        .function   = vEthernet          /* Thread Function */
    };
void vEthernet(ULONG thread_input) {
    
    /* PHY_RESET Pin has to be set HIGH for the PHY to function. */
    HAL_GPIO_WritePin(PHY_RESET_GPIO_Port, PHY_RESET_Pin, GPIO_PIN_SET);

    while(1) {

        ethernet_message_t message;
        uint8_t status;

        /* Process outgoing messages */
        while(queue_receive(&eth_outgoing, &message) == U_SUCCESS) {
            status = ethernet_send_message(&message);
            if(status != U_SUCCESS) {
                PRINTLN_WARNING("Failed to send Ethernet message after removing from outgoing queue (Message ID: %d).", message.message_id);
                // u_TODO - maybe add the message back into the queue if it fails to send? not sure if this is a good idea tho
                }
        }

        /* Process incoming messages */
        while(queue_receive(&eth_incoming, &message) == U_SUCCESS) {
            inbox_ethernet(&message);
        }

        /* Sleep Thread for specified number of ticks. */
        tx_thread_sleep(ethernet_thread.sleep);
    }
}

/* CAN Thread. Sends outgoing messages and processes incoming messages. */
static thread_t can_thread = {
        .name       = "CAN Thread",     /* Name */
        .size       = 512,              /* Stack Size (in bytes) */
        .priority   = 0,                /* Priority */
        .threshold  = 0,                /* Preemption Threshold */
        .time_slice = TX_NO_TIME_SLICE, /* Time Slice */
        .auto_start = TX_AUTO_START,    /* Auto Start */
        .sleep      = 1,                /* Sleep (in ticks) */
        .function   = vCAN              /* Thread Function */
    };
void vCAN(ULONG thread_input) {
    
    while(1) {

        can_msg_t message;
        uint8_t status;

        /* Process outgoing messages */
        while(queue_receive(&can_outgoing, &message) == U_SUCCESS) {
            status = can_send_msg(&can1, &message);
            if(status != U_SUCCESS) {
                PRINTLN_WARNING("Failed to send message (on can1) after removing from outgoing queue (Message ID: %ld).", message.id);
                // u_TODO - maybe add the message back into the queue if it fails to send? not sure if this is a good idea tho
                }
        }

        /* Process incoming messages */
        while(queue_receive(&can_incoming, &message) == U_SUCCESS) {
            inbox_can(&message);
        }

        /* Sleep Thread for specified number of ticks. */
        tx_thread_sleep(can_thread.sleep);
    }
}

/* Faults Thread. */
static thread_t faults_thread = {
        .name       = "Faults Thread",  /* Name */
        .size       = 512,              /* Stack Size (in bytes) */
        .priority   = 4,                /* Priority */
        .threshold  = 0,                /* Preemption Threshold */
        .time_slice = TX_NO_TIME_SLICE, /* Time Slice */
        .auto_start = TX_AUTO_START,    /* Auto Start */
        .sleep      = 500,              /* Sleep (in ticks) */
        .function   = vFaults           /* Thread Function */
    };
void vFaults(ULONG thread_input) {
    
    while(1) {

        /* Process queued faults */
        fault_t fault_id;
        while(queue_receive(&faults, &fault_id) == U_SUCCESS) {
            trigger_fault(fault_id);
        }

        /* Send a CAN message containing the current fault statuses. */
        uint64_t faults = get_faults();
        can_msg_t msg = {.id = CANID_FAULT_MSG, .len = 8, .data = {0}};
        memcpy(msg.data, &faults, sizeof(faults));
        queue_send(&can_outgoing, &msg);

        /* Sleep Thread for specified number of ticks. */
        tx_thread_sleep(faults_thread.sleep);
    }
}

/* Shutdown Thread. Reads the shutdown (aka. "External Faults") pins and sends them in a CAN message. */
static thread_t shutdown_thread = {
        .name       = "Shutdown Thread", /* Name */
        .size       = 512,               /* Stack Size (in bytes) */
        .priority   = 5,                 /* Priority */
        .threshold  = 0,                 /* Preemption Threshold */
        .time_slice = TX_NO_TIME_SLICE,  /* Time Slice */
        .auto_start = TX_AUTO_START,     /* Auto Start */
        .sleep      = 500,               /* Sleep (in ticks) */
        .function   = vShutdown          /* Thread Function */
    };
void vShutdown(ULONG thread_input) {
    
    while(1) {

        /* Create bitstream. */
        bitstream_t bitstream;
        uint8_t bitstream_data[2];
        bitstream_init(&bitstream, bitstream_data, 2);

        /* Read the shutdown pins and add them to the bitstream. */
        bitstream_add(&bitstream, (HAL_GPIO_ReadPin(BMS_GPIO_GPIO_Port, BMS_GPIO_Pin) == GPIO_PIN_SET), 1);               // Read BMS_GPIO pin.
        bitstream_add(&bitstream, (HAL_GPIO_ReadPin(BOTS_GPIO_GPIO_Port, BOTS_GPIO_Pin) == GPIO_PIN_SET), 1);             // Read BOTS_GPIO pin.
        bitstream_add(&bitstream, (HAL_GPIO_ReadPin(SPARE_GPIO_GPIO_Port, SPARE_GPIO_Pin) == GPIO_PIN_SET), 1);           // Read SPARE_GPIO pin.
        bitstream_add(&bitstream, (HAL_GPIO_ReadPin(BSPD_GPIO_GPIO_Port, BSPD_GPIO_Pin) == GPIO_PIN_SET), 1);             // Read BSPD_GPIO pin.
        bitstream_add(&bitstream, (HAL_GPIO_ReadPin(HV_C_GPIO_GPIO_Port, HV_C_GPIO_Pin) == GPIO_PIN_SET), 1);             // Read HV_C_GPIO pin.
        bitstream_add(&bitstream, (HAL_GPIO_ReadPin(HVD_GPIO_GPIO_Port, HVD_GPIO_Pin) == GPIO_PIN_SET), 1);               // Read HVD_GPIO pin.
        bitstream_add(&bitstream, (HAL_GPIO_ReadPin(IMD_GPIO_GPIO_Port, IMD_GPIO_Pin) == GPIO_PIN_SET), 1);               // Read IMD_GPIO pin.
        bitstream_add(&bitstream, (HAL_GPIO_ReadPin(CKPT_GPIO_GPIO_Port, CKPT_GPIO_Pin) == GPIO_PIN_SET), 1);             // Read CKPT_GPIO pin.
        bitstream_add(&bitstream, (HAL_GPIO_ReadPin(INERTIA_SW_GPIO_GPIO_Port, INERTIA_SW_GPIO_Pin) == GPIO_PIN_SET), 1); // Read INERTIA_SW_GPIO pin.
        bitstream_add(&bitstream, (HAL_GPIO_ReadPin(TSMS_GPIO_GPIO_Port, TSMS_GPIO_Pin) == GPIO_PIN_SET), 1);             // Read TSMS_GPIO pin.
        bitstream_add(&bitstream, 0, 6); // Extra (6 bits).

        /* Send CAN message. */
        can_msg_t msg = {.id = CANID_SHUTDOWN_MSG, .len = 2, .data = {0}};
        memcpy(msg.data, &bitstream_data, sizeof(bitstream_data));
        queue_send(&can_outgoing, &msg);

        /* Sleep Thread for specified number of ticks. */
        tx_thread_sleep(shutdown_thread.sleep);
    }
}

/* State Machine Thread. */
static thread_t statemachine_thread = {
        .name       = "State Machine Thread", /* Name */
        .size       = 512,                    /* Stack Size (in bytes) */
        .priority   = 2,                      /* Priority */
        .threshold  = 0,                      /* Preemption Threshold */
        .time_slice = TX_NO_TIME_SLICE,       /* Time Slice */
        .auto_start = TX_AUTO_START,          /* Auto Start */
        .sleep      = 1,                      /* Sleep (in ticks) */
        .function   = vStatemachine           /* Thread Function */
    };
void vStatemachine(ULONG thread_input) {
    
    while(1) {

        statemachine_process();

        /* Sleep Thread for specified number of ticks. */
        tx_thread_sleep(statemachine_thread.sleep);
    }
}

/* Pedals Thread. */
static thread_t pedals_thread = {
        .name       = "Pedals Thread",        /* Name */
        .size       = 512,                    /* Stack Size (in bytes) */
        .priority   = 2,                      /* Priority */
        .threshold  = 0,                      /* Preemption Threshold */
        .time_slice = TX_NO_TIME_SLICE,       /* Time Slice */
        .auto_start = TX_AUTO_START,          /* Auto Start */
        .sleep      = 1,                      /* Sleep (in ticks) */
        .function   = vPedals                 /* Thread Function */
    };
void vPedals(ULONG thread_input) {
    
    while(1) {

        pedals_process();

        /* Sleep Thread for specified number of ticks. */
        tx_thread_sleep(pedals_thread.sleep);
    }
}

/* eFuses Thread. */
static thread_t efuses_thread = {
        .name       = "eFuses Thread",        /* Name */
        .size       = 512,                    /* Stack Size (in bytes) */
        .priority   = 10,                     /* Priority */
        .threshold  = 0,                      /* Preemption Threshold */
        .time_slice = TX_NO_TIME_SLICE,       /* Time Slice */
        .auto_start = TX_AUTO_START,          /* Auto Start */
        .sleep      = 10,                     /* Sleep (in ticks) */
        .function   = vEFuses                 /* Thread Function */
    };
void vEFuses(ULONG thread_input) {

    /* Struct for holding eFuse data. */
    /* Each instance of this struct represents a different eFuse. */
    typedef struct __attribute__((__packed__)) {
		uint16_t raw;
        uint16_t voltage;
		uint16_t current;
		bool faulted;
		bool enabled;
	} efuse_message_t;
    
    while(1) {

        /* Set data. */
        efuse_data_t data = efuse_getData();
        efuse_message_t messages[NUM_EFUSES] = { 0 };
        for(efuse_t efuse = 0; efuse < NUM_EFUSES; efuse++) {
            /* Set the data for each eFuse. */
            messages[efuse].raw = data.raw[efuse];
            messages[efuse].voltage = (uint16_t)(data.voltage[efuse] * 1000);
            messages[efuse].current = (uint16_t)(data.current[efuse] * 1000);
            messages[efuse].faulted = data.faulted[efuse];
            messages[efuse].enabled = data.enabled[efuse];
        }

        /* Create and queue dashboard message. */
        can_msg_t dashboard_msg = {.id = CANID_EFUSE_DASHBOARD, .len = 8, .id_is_extended = true};
        memcpy(dashboard_msg.data, &messages[EFUSE_DASHBOARD], dashboard_msg.len);
        queue_send(&can_outgoing, &dashboard_msg);

        /* Create and queue brake message. */
        can_msg_t brake_msg = {.id = CANID_EFUSE_BRAKE, .len = 8, .id_is_extended = true};
        memcpy(brake_msg.data, &messages[EFUSE_BRAKE], brake_msg.len);
        queue_send(&can_outgoing, &brake_msg);

        /* Create and queue shutdown message. */
        can_msg_t shutdown_msg = {.id = CANID_EFUSE_SHUTDOWN, .len = 8, .id_is_extended = true};
        memcpy(shutdown_msg.data, &messages[EFUSE_SHUTDOWN], shutdown_msg.len);
        queue_send(&can_outgoing, &shutdown_msg);

        /* Create and queue LV message. */
        can_msg_t lv_msg = {.id = CANID_EFUSE_LV, .len = 8, .id_is_extended = true};
        memcpy(lv_msg.data, &messages[EFUSE_LV], lv_msg.len);
        queue_send(&can_outgoing, &lv_msg);

        /* Create and queue radfan message. */
        can_msg_t radfan_msg = {.id = CANID_EFUSE_RADFAN, .len = 8, .id_is_extended = true};
        memcpy(radfan_msg.data, &messages[EFUSE_RADFAN], radfan_msg.len);
        queue_send(&can_outgoing, &radfan_msg);

        /* Create and queue fanbatt message. */
        can_msg_t fanbatt_msg = {.id = CANID_EFUSE_FANBATT, .len = 8, .id_is_extended = true};
        memcpy(fanbatt_msg.data, &messages[EFUSE_FANBATT], fanbatt_msg.len);
        queue_send(&can_outgoing, &fanbatt_msg);

        /* Create and queue pump1 message. */
        can_msg_t pump1_msg = {.id = CANID_EFUSE_PUMP1, .len = 8, .id_is_extended = true};
        memcpy(pump1_msg.data, &messages[EFUSE_PUMP1], pump1_msg.len);
        queue_send(&can_outgoing, &pump1_msg);

        /* Create and queue pump2 message. */
        can_msg_t pump2_msg = {.id = CANID_EFUSE_PUMP2, .len = 8, .id_is_extended = true};
        memcpy(pump2_msg.data, &messages[EFUSE_PUMP2], pump2_msg.len);
        queue_send(&can_outgoing, &pump2_msg);

        /* Create and queue battbox message. */
        can_msg_t battbox_msg = {.id = CANID_EFUSE_BATTBOX, .len = 8, .id_is_extended = true};
        memcpy(battbox_msg.data, &messages[EFUSE_BATTBOX], battbox_msg.len);
        queue_send(&can_outgoing, &battbox_msg);

        /* Create and queue MC message. */
        can_msg_t mc_msg = {.id = CANID_EFUSE_MC, .len = 8, .id_is_extended = true};
        memcpy(mc_msg.data, &messages[EFUSE_MC], mc_msg.len);
        queue_send(&can_outgoing, &mc_msg);

        /* Sleep Thread for specified number of ticks. */
        tx_thread_sleep(efuses_thread.sleep);
    }
}

/* TSMS Thread. */
static thread_t tsms_thread = {
        .name       = "TSMS Thread",          /* Name */
        .size       = 512,                    /* Stack Size (in bytes) */
        .priority   = 2,                      /* Priority */
        .threshold  = 0,                      /* Preemption Threshold */
        .time_slice = TX_NO_TIME_SLICE,       /* Time Slice */
        .auto_start = TX_AUTO_START,          /* Auto Start */
        .sleep      = 50,                     /* Sleep (in ticks) */
        .function   = vTSMS                   /* Thread Function */
    };
void vTSMS(ULONG thread_input) {
    
    while(1) {

        tsms_update();

        /* Sleep Thread for specified number of ticks. */
        tx_thread_sleep(tsms_thread.sleep);
    }
}

/* Mux Thread (for the ADC multiplexer). */
static thread_t mux_thread = {
        .name       = "Mux Thread",           /* Name */
        .size       = 512,                    /* Stack Size (in bytes) */
        .priority   = 10,                     /* Priority */
        .threshold  = 0,                      /* Preemption Threshold */
        .time_slice = TX_NO_TIME_SLICE,       /* Time Slice */
        .auto_start = TX_AUTO_START,          /* Auto Start */
        .sleep      = 100,                    /* Sleep (in ticks) */
        .function   = vMux                    /* Thread Function */
    };
void vMux(ULONG thread_input) {
    
    while(1) {

        /* Switches the multiplexer state and updates the buffer. */
        adc_switchMuxState();

        /* Sleep Thread for specified number of ticks. */
        tx_thread_sleep(mux_thread.sleep);
    }
}

/* Initializes all ThreadX threads. 
*  Calls to _create_thread() should go in here
*/
uint8_t threads_init(TX_BYTE_POOL *byte_pool) {

    /* Create Threads */
    CATCH_ERROR(create_thread(byte_pool, &default_thread), U_SUCCESS);      // Create Default thread.
    CATCH_ERROR(create_thread(byte_pool, &ethernet_thread), U_SUCCESS);     // Create Ethernet thread.
    CATCH_ERROR(create_thread(byte_pool, &can_thread), U_SUCCESS);          // Create CAN thread.
    CATCH_ERROR(create_thread(byte_pool, &faults_thread), U_SUCCESS);       // Create Faults thread.
    CATCH_ERROR(create_thread(byte_pool, &shutdown_thread), U_SUCCESS);     // Create Shutdown thread.
    CATCH_ERROR(create_thread(byte_pool, &statemachine_thread), U_SUCCESS); // Create State Machine thread.
    CATCH_ERROR(create_thread(byte_pool, &pedals_thread), U_SUCCESS);       // Create Pedals thread.
    CATCH_ERROR(create_thread(byte_pool, &efuses_thread), U_SUCCESS);       // Create eFuses thread.
    CATCH_ERROR(create_thread(byte_pool, &tsms_thread), U_SUCCESS);         // Create TSMS thread.
    CATCH_ERROR(create_thread(byte_pool, &mux_thread), U_SUCCESS);          // Create Mux thread.

    // add more threads here if need

    PRINTLN_INFO("Ran threads_init().");
    return U_SUCCESS;
}
