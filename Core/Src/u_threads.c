#include "main.h"
#include "u_tx_debug.h"
#include "u_threads.h"
#include "u_queues.h"
#include "u_can.h"
#include "u_nx_ethernet.h"
#include "u_faults.h"
#include "u_pedals.h"
#include "u_adc.h"
#include "u_efuses.h"
#include "u_statemachine.h"
#include "u_tsms.h"
#include "u_peripherals.h"
#include "u_ethernet.h"
#include "bitstream.h"

/* Thread Priority Macros. */
/* (please keep these organized in increasing order) */
#define PRIO_vDefault          0
#define PRIO_vFaults           0
#define PRIO_vFaultsQueue      0
#define PRIO_vEthernetIncoming 1
#define PRIO_vEthernetOutgoing 1
#define PRIO_vCANIncoming      1
#define PRIO_vCANOutgoing      1
#define PRIO_vPedals           1
#define PRIO_vStatemachine     1
#define PRIO_vShutdown         2
#define PRIO_vEFuses           2
#define PRIO_vTSMS             2
#define PRIO_vMux              2
#define PRIO_vPeripherals      2

/* Default Thread */
static thread_t default_thread = {
        .name       = "Default Thread",  /* Name */
        .size       = 512,               /* Stack Size (in bytes) */
        .priority   = PRIO_vDefault, /* Priority */
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

/* Incoming Ethernet Thread. Processes incoming messages. */
static thread_t ethernet_incoming_thread = {
        .name       = "Incoming Ethernet Thread",  /* Name */
        .size       = 512,                         /* Stack Size (in bytes) */
        .priority   = PRIO_vEthernetIncoming,      /* Priority */
        .threshold  = 0,                           /* Preemption Threshold */
        .time_slice = TX_NO_TIME_SLICE,            /* Time Slice */
        .auto_start = TX_AUTO_START,               /* Auto Start */
        .sleep      =  0,                          /* Sleep (in ticks) */
        .function   = vEthernetIncoming            /* Thread Function */
    };
void vEthernetIncoming(ULONG thread_input) {

    while(1) {

        ethernet_message_t message;

        /* Process incoming messages */
        while(queue_receive(&eth_incoming, &message, TX_WAIT_FOREVER) == U_SUCCESS) {
            ethernet_inbox(&message);
        }

        /* No sleep. Thread timing is controlled completely by the queue timeout. */
    }
}

/* Outgoing Ethernet Thread. Sends outgoing messages. */
static thread_t ethernet_outgoing_thread = {
        .name       = "Outgoing Ethernet Thread",  /* Name */
        .size       = 512,                         /* Stack Size (in bytes) */
        .priority   = PRIO_vEthernetOutgoing,      /* Priority */
        .threshold  = 0,                           /* Preemption Threshold */
        .time_slice = TX_NO_TIME_SLICE,            /* Time Slice */
        .auto_start = TX_AUTO_START,               /* Auto Start */
        .sleep      =  0,                          /* Sleep (in ticks) */
        .function   = vEthernetOutgoing            /* Thread Function */
    };
void vEthernetOutgoing(ULONG thread_input) {

    while(1) {

        ethernet_message_t message;
        uint8_t status;

        /* Send outgoing messages */
        while(queue_receive(&eth_outgoing, &message, TX_WAIT_FOREVER) == U_SUCCESS) {
            status = ethernet_send_message(&message);
            if(status != U_SUCCESS) {
                PRINTLN_WARNING("Failed to send Ethernet message after removing from outgoing queue (Message ID: %d).", message.message_id);
                // u_TODO - maybe add the message back into the queue if it fails to send? not sure if this is a good idea tho
                }
        }

        /* No sleep. Thread timing is controlled completely by the queue timeout. */
    }
}

/* Incoming CAN Thread. Processes incoming messages. */
static thread_t can_incoming_thread = {
        .name       = "Incoming CAN Thread",     /* Name */
        .size       = 512,                       /* Stack Size (in bytes) */
        .priority   = PRIO_vCANIncoming,         /* Priority */
        .threshold  = 0,                         /* Preemption Threshold */
        .time_slice = TX_NO_TIME_SLICE,          /* Time Slice */
        .auto_start = TX_AUTO_START,             /* Auto Start */
        .sleep      = 0,                         /* Sleep (in ticks) */
        .function   = vCANIncoming               /* Thread Function */
    };
void vCANIncoming(ULONG thread_input) {
    
    while(1) {

        can_msg_t message;

        /* Process incoming messages */
        while(queue_receive(&can_incoming, &message, TX_WAIT_FOREVER) == U_SUCCESS) {
            can_inbox(&message);
        }

        /* No sleep. Thread timing is controlled completely by the queue timeout. */
    }
}

/* Outgoing CAN Thread. Sends outgoing messages. */
static thread_t can_outgoing_thread = {
        .name       = "Outgoing CAN Thread",     /* Name */
        .size       = 512,                       /* Stack Size (in bytes) */
        .priority   = PRIO_vCANOutgoing,         /* Priority */
        .threshold  = 0,                         /* Preemption Threshold */
        .time_slice = TX_NO_TIME_SLICE,          /* Time Slice */
        .auto_start = TX_AUTO_START,             /* Auto Start */
        .sleep      = 0,                         /* Sleep (in ticks) */
        .function   = vCANOutgoing               /* Thread Function */
    };
void vCANOutgoing(ULONG thread_input) {
    
    while(1) {

        can_msg_t message;
        HAL_StatusTypeDef status;

        /* Send outgoing messages */
        while(queue_receive(&can_outgoing, &message, TX_WAIT_FOREVER) == U_SUCCESS) {
            status = can_send_msg(&can1, &message);
            if(status != HAL_OK) {
                PRINTLN_WARNING("Failed to send message (on can1) after removing from outgoing queue (Message ID: %ld, Status: %d/%s).", message.id, status, hal_status_toString(status));
                queue_send(&faults, &(fault_t){CAN_OUTGOING_FAULT}, TX_NO_WAIT);
            }
        }

        /* No sleep. Thread timing is controlled completely by the queue timeout. */
    }
}

/* Faults Queue Thread. */
static thread_t faults_queue_thread = {
        .name       = "Faults Queue Thread",  /* Name */
        .size       = 512,                    /* Stack Size (in bytes) */
        .priority   = PRIO_vFaultsQueue,      /* Priority */
        .threshold  = 0,                      /* Preemption Threshold */
        .time_slice = TX_NO_TIME_SLICE,       /* Time Slice */
        .auto_start = TX_AUTO_START,          /* Auto Start */
        .sleep      = 0,                      /* Sleep (in ticks) */
        .function   = vFaultsQueue            /* Thread Function */
    };
void vFaultsQueue(ULONG thread_input) {
    
    while(1) {

        /* Process queued faults */
        fault_t fault_id;
        while(queue_receive(&faults, &fault_id, TX_WAIT_FOREVER) == U_SUCCESS) {
            trigger_fault(fault_id);
        }

        /* No sleep. Thread timing is controlled completely by the queue timeout. */
    }
}

/* Faults Thread. */
static thread_t faults_thread = {
        .name       = "Faults Thread",  /* Name */
        .size       = 512,                    /* Stack Size (in bytes) */
        .priority   = PRIO_vFaults,           /* Priority */
        .threshold  = 0,                      /* Preemption Threshold */
        .time_slice = TX_NO_TIME_SLICE,       /* Time Slice */
        .auto_start = TX_AUTO_START,          /* Auto Start */
        .sleep      = 500,                    /* Sleep (in ticks) */
        .function   = vFaults                 /* Thread Function */
    };
void vFaults(ULONG thread_input) {
    
    while(1) {

        /* Sleep Thread for specified number of ticks. */
        tx_thread_sleep(faults_thread.sleep);

        /* Send a CAN message containing the current fault statuses. */
        uint64_t faults = get_faults();
        can_msg_t msg = {.id = CANID_FAULT_MSG, .len = 8, .data = {0}};
        memcpy(msg.data, &faults, sizeof(faults));
        queue_send(&can_outgoing, &msg, TX_NO_WAIT);
    }
}

/* Shutdown Thread. Reads the shutdown (aka. "External Faults") pins and sends them in a CAN message. */
static thread_t shutdown_thread = {
        .name       = "Shutdown Thread",  /* Name */
        .size       = 512,                /* Stack Size (in bytes) */
        .priority   = PRIO_vShutdown, /* Priority */
        .threshold  = 0,                  /* Preemption Threshold */
        .time_slice = TX_NO_TIME_SLICE,   /* Time Slice */
        .auto_start = TX_AUTO_START,      /* Auto Start */
        .sleep      = 500,                /* Sleep (in ticks) */
        .function   = vShutdown           /* Thread Function */
    };
void vShutdown(ULONG thread_input) {
    
    while(1) {

        /* Sleep Thread for specified number of ticks. */
        tx_thread_sleep(shutdown_thread.sleep);

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
        queue_send(&can_outgoing, &msg, TX_NO_WAIT);
    }
}

/* State Machine Thread. */
static thread_t statemachine_thread = {
        .name       = "State Machine Thread", /* Name */
        .size       = 512,                    /* Stack Size (in bytes) */
        .priority   = PRIO_vStatemachine,     /* Priority */
        .threshold  = 0,                      /* Preemption Threshold */
        .time_slice = TX_NO_TIME_SLICE,       /* Time Slice */
        .auto_start = TX_AUTO_START,          /* Auto Start */
        .sleep      = 1,                      /* Sleep (in ticks) */
        .function   = vStatemachine           /* Thread Function */
    };
void vStatemachine(ULONG thread_input) {
    
    while(1) {

        /* Sleep Thread for specified number of ticks. */
        tx_thread_sleep(statemachine_thread.sleep);

        statemachine_process();
    }
}

/* Pedals Thread. */
static thread_t pedals_thread = {
        .name       = "Pedals Thread",        /* Name */
        .size       = 512,                    /* Stack Size (in bytes) */
        .priority   = PRIO_vPedals,           /* Priority */
        .threshold  = 0,                      /* Preemption Threshold */
        .time_slice = TX_NO_TIME_SLICE,       /* Time Slice */
        .auto_start = TX_AUTO_START,          /* Auto Start */
        .sleep      = 1,                      /* Sleep (in ticks) */
        .function   = vPedals                 /* Thread Function */
    };
void vPedals(ULONG thread_input) {
    
    while(1) {

        /* Sleep Thread for specified number of ticks. */
        tx_thread_sleep(pedals_thread.sleep);

        pedals_process();

    }
}

/* eFuses Thread. */
static thread_t efuses_thread = {
        .name       = "eFuses Thread",        /* Name */
        .size       = 512,                    /* Stack Size (in bytes) */
        .priority   = PRIO_vEFuses,       /* Priority */
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
        queue_send(&can_outgoing, &dashboard_msg, TX_NO_WAIT);

        /* Create and queue brake message. */
        can_msg_t brake_msg = {.id = CANID_EFUSE_BRAKE, .len = 8, .id_is_extended = true};
        memcpy(brake_msg.data, &messages[EFUSE_BRAKE], brake_msg.len);
        queue_send(&can_outgoing, &brake_msg, TX_NO_WAIT);

        /* Create and queue shutdown message. */
        can_msg_t shutdown_msg = {.id = CANID_EFUSE_SHUTDOWN, .len = 8, .id_is_extended = true};
        memcpy(shutdown_msg.data, &messages[EFUSE_SHUTDOWN], shutdown_msg.len);
        queue_send(&can_outgoing, &shutdown_msg, TX_NO_WAIT);

        /* Create and queue LV message. */
        can_msg_t lv_msg = {.id = CANID_EFUSE_LV, .len = 8, .id_is_extended = true};
        memcpy(lv_msg.data, &messages[EFUSE_LV], lv_msg.len);
        queue_send(&can_outgoing, &lv_msg, TX_NO_WAIT);

        /* Create and queue radfan message. */
        can_msg_t radfan_msg = {.id = CANID_EFUSE_RADFAN, .len = 8, .id_is_extended = true};
        memcpy(radfan_msg.data, &messages[EFUSE_RADFAN], radfan_msg.len);
        queue_send(&can_outgoing, &radfan_msg, TX_NO_WAIT);

        /* Create and queue fanbatt message. */
        can_msg_t fanbatt_msg = {.id = CANID_EFUSE_FANBATT, .len = 8, .id_is_extended = true};
        memcpy(fanbatt_msg.data, &messages[EFUSE_FANBATT], fanbatt_msg.len);
        queue_send(&can_outgoing, &fanbatt_msg, TX_NO_WAIT);

        /* Create and queue pump1 message. */
        can_msg_t pump1_msg = {.id = CANID_EFUSE_PUMP1, .len = 8, .id_is_extended = true};
        memcpy(pump1_msg.data, &messages[EFUSE_PUMP1], pump1_msg.len);
        queue_send(&can_outgoing, &pump1_msg, TX_NO_WAIT);

        /* Create and queue pump2 message. */
        can_msg_t pump2_msg = {.id = CANID_EFUSE_PUMP2, .len = 8, .id_is_extended = true};
        memcpy(pump2_msg.data, &messages[EFUSE_PUMP2], pump2_msg.len);
        queue_send(&can_outgoing, &pump2_msg, TX_NO_WAIT);

        /* Create and queue battbox message. */
        can_msg_t battbox_msg = {.id = CANID_EFUSE_BATTBOX, .len = 8, .id_is_extended = true};
        memcpy(battbox_msg.data, &messages[EFUSE_BATTBOX], battbox_msg.len);
        queue_send(&can_outgoing, &battbox_msg, TX_NO_WAIT);

        /* Create and queue MC message. */
        can_msg_t mc_msg = {.id = CANID_EFUSE_MC, .len = 8, .id_is_extended = true};
        memcpy(mc_msg.data, &messages[EFUSE_MC], mc_msg.len);
        queue_send(&can_outgoing, &mc_msg, TX_NO_WAIT);

        /* Sleep Thread for specified number of ticks. */
        tx_thread_sleep(efuses_thread.sleep);
    }
}

/* TSMS Thread. */
static thread_t tsms_thread = {
        .name       = "TSMS Thread",          /* Name */
        .size       = 512,                    /* Stack Size (in bytes) */
        .priority   = PRIO_vTSMS,         /* Priority */
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
        .priority   = PRIO_vMux,          /* Priority */
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

/* Peripherals Thread. */
static thread_t peripherals_thread = {
        .name       = "Peripherals Thread",   /* Name */
        .size       = 512,                    /* Stack Size (in bytes) */
        .priority   = PRIO_vPeripherals,      /* Priority */
        .threshold  = 0,                      /* Preemption Threshold */
        .time_slice = TX_NO_TIME_SLICE,       /* Time Slice */
        .auto_start = TX_AUTO_START,          /* Auto Start */
        .sleep      = 100,                    /* Sleep (in ticks) */
        .function   = vPeripherals            /* Thread Function */
    };
void vPeripherals(ULONG thread_input) {

    /* Format for the temp sensor CAN data. */
    typedef struct __attribute__((__packed__)) {
		int16_t temperature;
        uint16_t humidity;
	} tempsensor_CAN_t;

    /* Format for the IMU Acceleration CAN data. */
    typedef struct __attribute__((__packed__)) {
		int16_t x;
        int16_t y;
        int16_t z;
	} acceleration_CAN_t;

    /* Format for the IMU Gyro CAN data. */
    typedef struct __attribute__((__packed__)) {
		int16_t x;
        int16_t y;
        int16_t z;
	} gyro_CAN_t;
    
    while(1) {

        /* Get the temp sensor data. */
        float temperature = 0;
        float humidity = 0;
        int status = tempsensor_getTemperatureAndHumidity(&temperature, &humidity);
        if(status != U_SUCCESS) {
            PRINTLN_ERROR("Failed to called tempsensor_getTemperatureAndHumidity() in the peripherals thread (Status: %d).", status);
            queue_send(&faults, &(fault_t){ONBOARD_TEMP_FAULT}, TX_NO_WAIT);
        }

        /* Fill the temp sensor message and send it over CAN. */
        tempsensor_CAN_t tempsensor_CAN = { 0 };
        tempsensor_CAN.temperature = (int16_t)(temperature * 100);
        tempsensor_CAN.humidity = (uint16_t)(humidity * 100);
        can_msg_t temp_sensor_message = {.id = CANID_TEMP_SENSOR, .len = 4, .id_is_extended = false};
        memcpy(temp_sensor_message.data, &tempsensor_CAN, temp_sensor_message.len);
        queue_send(&can_outgoing, &temp_sensor_message, TX_NO_WAIT);

        /* Get the IMU acceleration data. */
        vector3_t acceleration;
        status = imu_getAcceleration(&acceleration);
        if(status != U_SUCCESS) {
            PRINTLN_ERROR("Failed to call imu_getAcceleration() in the peripherals thread (Status: %d).", status);
            queue_send(&faults, &(fault_t){IMU_ACCEL_FAULT}, TX_NO_WAIT);
        }

        /* Fill the IMU acceleration message and send it over CAN. */
        acceleration_CAN_t acceleration_CAN = { 0 };
        acceleration_CAN.x = (int16_t)(acceleration.x * 100);
        acceleration_CAN.y = (int16_t)(acceleration.y * 100);
        acceleration_CAN.z = (int16_t)(acceleration.z * 100);
        can_msg_t imu_acceleration_message = {.id = CANID_IMU_ACCEL, .len = 6, .id_is_extended = false};
        memcpy(imu_acceleration_message.data, &acceleration_CAN, imu_acceleration_message.len);
        queue_send(&can_outgoing, &imu_acceleration_message, TX_NO_WAIT);

        /* Get the IMU Gyro data. */
        vector3_t gyro;
        status = imu_getAngularRate(&gyro);
        if(status != U_SUCCESS) {
            PRINTLN_ERROR("Failed to call imu_getAngularRate() in the peripherals thread (Status: %d).", status);
            queue_send(&faults, &(fault_t){IMU_GYRO_FAULT}, TX_NO_WAIT);
        }

        /* Fill the IMU Gyro message and send it over CAN. */
        gyro_CAN_t gyro_CAN = { 0 };
        gyro_CAN.x = (int16_t)(gyro.x * 100);
        gyro_CAN.y = (int16_t)(gyro.y * 100);
        gyro_CAN.z = (int16_t)(gyro.z * 100);
        can_msg_t imu_gyro_message = {.id = CANID_IMU_GYRO, .len = 6, .id_is_extended = false};
        memcpy(imu_gyro_message.data, &gyro_CAN, imu_gyro_message.len);
        queue_send(&can_outgoing, &imu_gyro_message, TX_NO_WAIT);

        /* Sleep Thread for specified number of ticks. */
        tx_thread_sleep(peripherals_thread.sleep);
    }
}

/* Initializes all ThreadX threads. 
*  Calls to _create_thread() should go in here
*/
uint8_t threads_init(TX_BYTE_POOL *byte_pool) {

    /* Create Threads */
    CATCH_ERROR(create_thread(byte_pool, &default_thread), U_SUCCESS);           // Create Default thread.
    CATCH_ERROR(create_thread(byte_pool, &ethernet_incoming_thread), U_SUCCESS); // Create Incoming Ethernet thread.
    CATCH_ERROR(create_thread(byte_pool, &ethernet_outgoing_thread), U_SUCCESS); // Create Outgoing Ethernet thread.
    CATCH_ERROR(create_thread(byte_pool, &can_incoming_thread), U_SUCCESS);      // Create Incoming CAN thread.
    CATCH_ERROR(create_thread(byte_pool, &can_outgoing_thread), U_SUCCESS);      // Create Outgoing CAN thread.
    CATCH_ERROR(create_thread(byte_pool, &faults_queue_thread), U_SUCCESS);      // Create Faults Queue thread.
    CATCH_ERROR(create_thread(byte_pool, &faults_thread), U_SUCCESS);            // Create Faults thread.
    CATCH_ERROR(create_thread(byte_pool, &shutdown_thread), U_SUCCESS);          // Create Shutdown thread.
    CATCH_ERROR(create_thread(byte_pool, &statemachine_thread), U_SUCCESS);      // Create State Machine thread.
    CATCH_ERROR(create_thread(byte_pool, &pedals_thread), U_SUCCESS);            // Create Pedals thread.
    CATCH_ERROR(create_thread(byte_pool, &efuses_thread), U_SUCCESS);            // Create eFuses thread.
    CATCH_ERROR(create_thread(byte_pool, &tsms_thread), U_SUCCESS);              // Create TSMS thread.
    CATCH_ERROR(create_thread(byte_pool, &mux_thread), U_SUCCESS);               // Create Mux thread.
    CATCH_ERROR(create_thread(byte_pool, &peripherals_thread), U_SUCCESS);       // Create Peripherals thread.

    // add more threads here if need

    PRINTLN_INFO("Ran threads_init().");
    return U_SUCCESS;
}
