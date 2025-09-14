#include "main.h"
#include "u_threads.h"
#include "u_queues.h"
#include "u_inbox.h"
#include "u_can.h"
#include "u_ethernet.h"
#include "u_faults.h"
#include "u_efuses.h"
#include "bitstream.h"

/* Default Thread */
static thread_t _default_thread = {
        .name       = "Default Thread",  /* Name */
        .size       = 512,               /* Stack Size (in bytes) */
        .priority   = 9,                 /* Priority */
        .threshold  = 0,                 /* Preemption Threshold */
        .time_slice = TX_NO_TIME_SLICE,  /* Time Slice */
        .auto_start = TX_AUTO_START,     /* Auto Start */
        .sleep      = 50,                /* Sleep (in ticks) */
        .function   = default_thread     /* Thread Function */
    };
void default_thread(ULONG thread_input) {
    
    while(1) {

        /* Kick the watchdogs (sad) )*/
        HAL_IWDG_Refresh(&hiwdg); // Internal Watchdog
        HAL_GPIO_TogglePin(WATCHDOG_GPIO_Port, WATCHDOG_Pin); // External Watchdog

        /* Sleep Thread for specified number of ticks. */
        tx_thread_sleep(_default_thread.sleep);
    }
}

/* Ethernet Thread. Sends outgoing messages and processes incoming messages. */
static thread_t _ethernet_thread = {
        .name       = "Ethernet Thread", /* Name */
        .size       = 512,               /* Stack Size (in bytes) */
        .priority   = 3,                 /* Priority */
        .threshold  = 0,                 /* Preemption Threshold */
        .time_slice = TX_NO_TIME_SLICE,  /* Time Slice */
        .auto_start = TX_AUTO_START,     /* Auto Start */
        .sleep      =  1,                /* Sleep (in ticks) */
        .function   = ethernet_thread    /* Thread Function */
    };
void ethernet_thread(ULONG thread_input) {
    
    while(1) {

        ethernet_message_t message;
        uint8_t status;

        /* Process outgoing messages */
        while(queue_receive(&eth_outgoing, &message) == U_SUCCESS) {
            status = ethernet_send_message(&message);
            if(status != U_SUCCESS) {
                DEBUG_PRINTLN("WARNING: Failed to send Ethernet message after removing from outgoing queue (Message ID: %d).", message.message_id);
                // u_TODO - maybe add the message back into the queue if it fails to send? not sure if this is a good idea tho
                }
        }

        /* Process incoming messages */
        while(queue_receive(&eth_incoming, &message) == U_SUCCESS) {
            inbox_ethernet(&message);
        }

        /* Sleep Thread for specified number of ticks. */
        tx_thread_sleep(_ethernet_thread.sleep);
    }
}

/* CAN Thread. Sends outgoing messages and processes incoming messages. */
static thread_t _can_thread = {
        .name       = "CAN Thread",     /* Name */
        .size       = 512,              /* Stack Size (in bytes) */
        .priority   = 0,                /* Priority */
        .threshold  = 0,                /* Preemption Threshold */
        .time_slice = TX_NO_TIME_SLICE, /* Time Slice */
        .auto_start = TX_AUTO_START,    /* Auto Start */
        .sleep      = 1,                /* Sleep (in ticks) */
        .function   = can_thread        /* Thread Function */
    };
void can_thread(ULONG thread_input) {
    
    while(1) {

        can_msg_t message;
        uint8_t status;

        /* Process outgoing messages */
        while(queue_receive(&can_outgoing, &message) == U_SUCCESS) {
            status = can_send_msg(&can1, &message);
            if(status != U_SUCCESS) {
                DEBUG_PRINTLN("WARNING: Failed to send message (on can1) after removing from outgoing queue (Message ID: %ld).", message.id);
                // u_TODO - maybe add the message back into the queue if it fails to send? not sure if this is a good idea tho
                }
        }

        /* Process incoming messages */
        while(queue_receive(&can_incoming, &message) == U_SUCCESS) {
            inbox_can(&message);
        }

        /* Sleep Thread for specified number of ticks. */
        tx_thread_sleep(_can_thread.sleep);
    }
}

/* Faults Thread. */
static thread_t _faults_thread = {
        .name       = "Faults Thread",  /* Name */
        .size       = 512,              /* Stack Size (in bytes) */
        .priority   = 4,                /* Priority */
        .threshold  = 0,                /* Preemption Threshold */
        .time_slice = TX_NO_TIME_SLICE, /* Time Slice */
        .auto_start = TX_AUTO_START,    /* Auto Start */
        .sleep      = 500,              /* Sleep (in ticks) */
        .function   = faults_thread     /* Thread Function */
    };
void faults_thread(ULONG thread_input) {
    
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
        tx_thread_sleep(_faults_thread.sleep);
    }
}

/* Shutdown Thread. Reads the shutdown (aka. "External Faults") pins and sends them in a CAN message. */
static thread_t _shutdown_thread = {
        .name       = "Shutdown Thread", /* Name */
        .size       = 512,               /* Stack Size (in bytes) */
        .priority   = 5,                 /* Priority */
        .threshold  = 0,                 /* Preemption Threshold */
        .time_slice = TX_NO_TIME_SLICE,  /* Time Slice */
        .auto_start = TX_AUTO_START,     /* Auto Start */
        .sleep      = 500,               /* Sleep (in ticks) */
        .function   = shutdown_thread    /* Thread Function */
    };
void shutdown_thread(ULONG thread_input) {
    
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
        tx_thread_sleep(_shutdown_thread.sleep);
    }
}

/* State Machine Thread. */
static thread_t _statemachine_thread = {
        .name       = "State Machine Thread", /* Name */
        .size       = 512,                    /* Stack Size (in bytes) */
        .priority   = 2,                      /* Priority */
        .threshold  = 0,                      /* Preemption Threshold */
        .time_slice = TX_NO_TIME_SLICE,       /* Time Slice */
        .auto_start = TX_AUTO_START,          /* Auto Start */
        .sleep      = 1,                      /* Sleep (in ticks) */
        .function   = statemachine_thread     /* Thread Function */
    };
void statemachine_thread(ULONG thread_input) {
    
    while(1) {

        statemachine_process();

        /* Sleep Thread for specified number of ticks. */
        tx_thread_sleep(_statemachine_thread.sleep);
    }
}

/* Helper function. Creates a ThreadX thread. */
static uint8_t _create_thread(TX_BYTE_POOL *byte_pool, thread_t *thread) {
    CHAR *pointer;
    uint8_t status;

    /* Allocate the stack for the thread. */
    status = tx_byte_allocate(byte_pool, (VOID**) &pointer, thread->size, TX_NO_WAIT);
    if(status != TX_SUCCESS) {
        DEBUG_PRINTLN("ERROR: Failed to allocate stack before creating thread (Status: %d/%s, Thread: %s).", status, tx_status_toString(status), thread->name);
        return U_ERROR;
    }

    /* Create the thread. */
    status = tx_thread_create(&thread->_TX_THREAD, (CHAR*)thread->name, thread->function, thread->thread_input, pointer, thread->size, thread->priority, thread->threshold, thread->time_slice, thread->auto_start);
    if(status != TX_SUCCESS) {
        DEBUG_PRINTLN("ERROR: Failed to create thread (Status: %d/%s, Thread: %s).", status, tx_status_toString(status), thread->name);
        tx_byte_release(pointer); // Free allocated memory if thread creation fails
        return U_ERROR;
    }
    
    return U_SUCCESS;
}

/* Initializes all ThreadX threads. 
*  Calls to _create_thread() should go in here
*/
uint8_t threads_init(TX_BYTE_POOL *byte_pool) {

    /* Create Threads */
    CATCH_ERROR(_create_thread(byte_pool, &_default_thread), U_SUCCESS);  // Create Default thread.
    CATCH_ERROR(_create_thread(byte_pool, &_ethernet_thread), U_SUCCESS); // Create Ethernet thread.
    CATCH_ERROR(_create_thread(byte_pool, &_can_thread), U_SUCCESS);      // Create CAN thread.
    CATCH_ERROR(_create_thread(byte_pool, &_faults_thread), U_SUCCESS);   // Create Faults thread.
    CATCH_ERROR(_create_thread(byte_pool, &_shutdown_thread), U_SUCCESS); // Create Shutdown thread.
    CATCH_ERROR(_create_thread(byte_pool, &_statemachine_thread), U_SUCCESS); // Create Shutdown thread.

    // add more threads here if need

    DEBUG_PRINTLN("Ran threads_init().");
    return U_SUCCESS;
}
