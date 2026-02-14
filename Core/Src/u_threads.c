#include "main.h"
#include "u_tx_debug.h"
#include "can_messages_tx.h"
#include "u_threads.h"
#include "u_queues.h"
#include "u_can.h"
#include "u_nx_ethernet.h"
#include "nxd_ptp_client.h"
#include "u_faults.h"
#include "u_pedals.h"
#include "u_adc.h"
#include "u_debug.h"
#include "u_efuses.h"
#include "u_statemachine.h"
#include "u_tsms.h"
#include "u_peripherals.h"
#include "u_ethernet.h"
#include "bitstream.h"
#include "serial.h"
#include "u_lightning.h"

/* Thread Priority Macros. */
/* (please keep these organized in increasing order) */
#define PRIO_vDefault          0
#define PRIO_vFaultsQueue      0
#define PRIO_vEthernetIncoming 0
#define PRIO_vEthernetOutgoing 0
#define PRIO_vCANIncoming      0
#define PRIO_vCANOutgoing      0
#define PRIO_vStatemachine     0
#define PRIO_vFaults           1
#define PRIO_vPedals           1
#define PRIO_vTSMS             1
#define PRIO_vShutdown         1
#define PRIO_vEFuses           2
#define PRIO_vMux              2
#define PRIO_vTest             2
#define PRIO_vPeripherals      2


// adding a forward declaration since this isn't defined in .h file
void update_lightning_board_status(bool bms_gpio, bool imd_gpio);

/* Test Thread */
static thread_t test_thread = {
        .name       = "Test Thread",    /* Name */
        .size       = 2048,              /* Stack Size (in bytes) */
        .priority   = PRIO_vTest,        /* Priority */
        .threshold  = 0,                 /* Preemption Threshold */
        .time_slice = TX_NO_TIME_SLICE,  /* Time Slice */
        .auto_start = TX_AUTO_START,     /* Auto Start */
        .sleep      = 1000,             /* Sleep (in ticks) */
        .function   = vTest             /* Thread Function */
    };
void vTest(ULONG thread_input) {

    /* Initialize ethernet (you have to do this in a thread for some reason). */
    int status = ethernet1_init();
    if(status != NX_SUCCESS) {
        PRINTLN_ERROR("Failed to call ethernet1_init() (Status: %d/%s).", status, nx_status_toString(status));
    }

    //tx_thread_sleep(5000);

    while(1) {

        //char message[8] = "message";
        uint8_t message = 210;
        ethernet_message_t msg = ethernet_create_message(0x01, TPU, &message, sizeof(message));
        int status = queue_send(&eth_outgoing, &msg, TX_WAIT_FOREVER);
        if(status != U_SUCCESS) {
            PRINTLN_ERROR("Failed to call queue_send when sending ethernet message (Status: %d).", status);
        } else {
            PRINTLN_INFO("Added message to ethernet outgoing queue.");
        }

        PRINTLN_INFO("Ran vTest");

        NX_PTP_DATE_TIME date = ethernet_get_time();
        PRINTLN_INFO("TIME: %2u/%02u/%u %02u:%02u:%02u.%09lu\r\n", date.day, date.month, date.year, date.hour, date.minute, date.second, date.nanosecond);

        tx_thread_sleep(test_thread.sleep);
    }
}

/* Default Thread */
static thread_t default_thread = {
        .name       = "Default Thread",  /* Name */
        .size       = 2048,              /* Stack Size (in bytes) */
        .priority   = PRIO_vDefault,     /* Priority */
        .threshold  = 0,                 /* Preemption Threshold */
        .time_slice = TX_NO_TIME_SLICE,  /* Time Slice */
        .auto_start = TX_AUTO_START,     /* Auto Start */
        .sleep      = 50,                /* Sleep (in ticks) */
        .function   = vDefault           /* Thread Function */
    };
void vDefault(ULONG thread_input) {

    PRINTLN_INFO("Starting default thread...");

    while(1) {

        /* Kick the watchdogs (sad) )*/
        HAL_IWDG_Refresh(&hiwdg); // Internal Watchdog
        HAL_GPIO_TogglePin(WATCHDOG_GPIO_Port, WATCHDOG_Pin); // External Watchdog

        static bool state_green = false;
        if(state_green) {
            debug_enableGreenLED();
            state_green = !state_green;
        } else {
            debug_disableGreenLED();
            state_green = !state_green;
        }

        static bool state_red = true;
        if(state_red) {
            debug_enableRedLED();
            state_red = !state_red;
        } else {
            debug_disableRedLED();
            state_red = !state_red;
        }

        PRINTLN_INFO("Ran default thread");

        /* Sleep Thread for specified number of ticks. */
        tx_thread_sleep(default_thread.sleep);
    }
}

/* Incoming Ethernet Thread. Processes incoming messages. */
static thread_t ethernet_incoming_thread = {
        .name       = "Incoming Ethernet Thread",  /* Name */
        .size       = 2048,                        /* Stack Size (in bytes) */
        .priority   = PRIO_vEthernetIncoming,      /* Priority */
        .threshold  = 0,                           /* Preemption Threshold */
        .time_slice = TX_NO_TIME_SLICE,            /* Time Slice */
        .auto_start = TX_AUTO_START,               /* Auto Start */
        .sleep      =  0,                          /* Sleep (in ticks) */
        .function   = vEthernetIncoming            /* Thread Function */
    };
void vEthernetIncoming(ULONG thread_input) {

    while(1) {

        PRINTLN_INFO("thread ran");

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
        .size       = 2048,                        /* Stack Size (in bytes) */
        .priority   = PRIO_vEthernetOutgoing,      /* Priority */
        .threshold  = 0,                           /* Preemption Threshold */
        .time_slice = TX_NO_TIME_SLICE,            /* Time Slice */
        .auto_start = TX_AUTO_START,               /* Auto Start */
        .sleep      =  0,                          /* Sleep (in ticks) */
        .function   = vEthernetOutgoing            /* Thread Function */
    };
void vEthernetOutgoing(ULONG thread_input) {

    while(1) {

        PRINTLN_INFO("thread ran");

        ethernet_message_t message;
        uint8_t status;

        /* Send outgoing messages */
        while(queue_receive(&eth_outgoing, &message, TX_WAIT_FOREVER) == U_SUCCESS) {
            status = ethernet_send_message(&message);
            if(status != U_SUCCESS) {
                PRINTLN_WARNING("Failed to send Ethernet message after removing from outgoing queue (Message ID: %d).", message.message_id);
                // u_TODO - maybe add the message back into the queue if it fails to send? not sure if this is a good idea tho
            } else {
                PRINTLN_INFO("Sent ethernet message!");
            }
        }

        /* No sleep. Thread timing is controlled completely by the queue timeout. */
    }
}

/* Incoming CAN Thread. Processes incoming messages. */
static thread_t can_incoming_thread = {
        .name       = "Incoming CAN Thread",     /* Name */
        .size       = 2048,                      /* Stack Size (in bytes) */
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

        PRINTLN_INFO("thread ran");

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
        .size       = 2048,                      /* Stack Size (in bytes) */
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

        PRINTLN_INFO("thread ran");

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
        .size       = 2048,                   /* Stack Size (in bytes) */
        .priority   = PRIO_vFaultsQueue,      /* Priority */
        .threshold  = 0,                      /* Preemption Threshold */
        .time_slice = TX_NO_TIME_SLICE,       /* Time Slice */
        .auto_start = TX_AUTO_START,          /* Auto Start */
        .sleep      = 0,                      /* Sleep (in ticks) */
        .function   = vFaultsQueue            /* Thread Function */
    };
void vFaultsQueue(ULONG thread_input) {

    while(1) {

        PRINTLN_INFO("thread ran");

        /* Process queued faults */
        fault_t fault_id;
        while(queue_receive(&faults, &fault_id, TX_WAIT_FOREVER) == U_SUCCESS) {
            trigger_fault(fault_id);
        }

        /* No sleep. Thread timing is controlled completely by the queue timeout. */
    }
}

/* State Machine Thread. */
static thread_t statemachine_thread = {
        .name       = "State Machine Thread", /* Name */
        .size       = 2048,                   /* Stack Size (in bytes) */
        .priority   = PRIO_vStatemachine,     /* Priority */
        .threshold  = 0,                      /* Preemption Threshold */
        .time_slice = TX_NO_TIME_SLICE,       /* Time Slice */
        .auto_start = TX_AUTO_START,          /* Auto Start */
        .sleep      = 0,                      /* Sleep (in ticks) */
        .function   = vStatemachine           /* Thread Function */
    };
void vStatemachine(ULONG thread_input) {

    while(1) {

        PRINTLN_INFO("thread ran");

        state_req_t new_state_req;
        while(queue_receive(&state_transition_queue, &new_state_req, TX_WAIT_FOREVER) == U_SUCCESS) {
            statemachine_process(new_state_req);
	    }

        /* No sleep. Thread timing is controlled completely by the queue timeout. */    }
}

/* Faults Thread. */
static thread_t faults_thread = {
        .name       = "Faults Thread",        /* Name */
        .size       = 2048,                   /* Stack Size (in bytes) */
        .priority   = PRIO_vFaults,           /* Priority */
        .threshold  = 0,                      /* Preemption Threshold */
        .time_slice = TX_NO_TIME_SLICE,       /* Time Slice */
        .auto_start = TX_AUTO_START,          /* Auto Start */
        .sleep      = 500,                    /* Sleep (in ticks) */
        .function   = vFaults                 /* Thread Function */
    };
void vFaults(ULONG thread_input) {

    while(1) {

        PRINTLN_INFO("thread ran");

        /* Send a CAN message containing the current fault statuses. */
        send_faults(
            get_fault(CAN_OUTGOING_FAULT),
            get_fault(CAN_INCOMING_FAULT),
            get_fault(BMS_CAN_MONITOR_FAULT),
            get_fault(ONBOARD_TEMP_FAULT),
            get_fault(IMU_ACCEL_FAULT),
            get_fault(IMU_GYRO_FAULT),
            get_fault(BSPD_PREFAULT),
            get_fault(ONBOARD_BRAKE_OPEN_CIRCUIT_FAULT),
            get_fault(ONBOARD_ACCEL_OPEN_CIRCUIT_FAULT),
            get_fault(ONBOARD_BRAKE_SHORT_CIRCUIT_FAULT),
            get_fault(ONBOARD_ACCEL_SHORT_CIRCUIT_FAULT),
            get_fault(ONBOARD_PEDAL_DIFFERENCE_FAULT),
            get_fault(RTDS_FAULT),
            get_fault(LV_LOW_VOLTAGE_FAULT),
            0
        );

        /* Sleep Thread for specified number of ticks. */
        tx_thread_sleep(faults_thread.sleep);
    }
}

/* Shutdown Thread. Reads the shutdown (aka. "External Faults") pins and sends them in a CAN message. */
static thread_t shutdown_thread = {
        .name       = "Shutdown Thread",  /* Name */
        .size       = 2048,               /* Stack Size (in bytes) */
        .priority   = PRIO_vShutdown,     /* Priority */
        .threshold  = 0,                  /* Preemption Threshold */
        .time_slice = TX_NO_TIME_SLICE,   /* Time Slice */
        .auto_start = TX_AUTO_START,      /* Auto Start */
        .sleep      = 500,                /* Sleep (in ticks) */
        .function   = vShutdown           /* Thread Function */
    };
void vShutdown(ULONG thread_input) {

    while(1) {

        bool bms_gpio = (HAL_GPIO_ReadPin(BMS_GPIO_GPIO_Port, BMS_GPIO_Pin) == GPIO_PIN_SET);
        bool bots_gpio = (HAL_GPIO_ReadPin(BOTS_GPIO_GPIO_Port, BOTS_GPIO_Pin) == GPIO_PIN_SET);
        bool spare_gpio = (HAL_GPIO_ReadPin(SPARE_GPIO_GPIO_Port, SPARE_GPIO_Pin) == GPIO_PIN_SET);
        bool bspd_gpio = (HAL_GPIO_ReadPin(BSPD_GPIO_GPIO_Port, BSPD_GPIO_Pin) == GPIO_PIN_SET);
        bool hv_c = (HAL_GPIO_ReadPin(HV_C_GPIO_GPIO_Port, HV_C_GPIO_Pin) == GPIO_PIN_SET);
        bool hvd_gpio = (HAL_GPIO_ReadPin(HVD_GPIO_GPIO_Port, HVD_GPIO_Pin) == GPIO_PIN_SET);
        bool imd_gpio = (HAL_GPIO_ReadPin(IMD_GPIO_GPIO_Port, IMD_GPIO_Pin) == GPIO_PIN_SET);
        bool ckpt_gpio = (HAL_GPIO_ReadPin(CKPT_GPIO_GPIO_Port, CKPT_GPIO_Pin) == GPIO_PIN_SET);
        bool inertia_sw_gpio = (HAL_GPIO_ReadPin(INERTIA_SW_GPIO_GPIO_Port, INERTIA_SW_GPIO_Pin) == GPIO_PIN_SET);
        bool tsms_gpio = (HAL_GPIO_ReadPin(TSMS_GPIO_GPIO_Port, TSMS_GPIO_Pin) == GPIO_PIN_SET);

        update_lightning_board_status(bms_gpio, imd_gpio);
        osDelay(100); //100ms debounce


        /* Send Shutdown Pins CAN message. */
        send_shutdown_pins(
            bms_gpio,
            bots_gpio,
            spare_gpio,
            bspd_gpio,
            hv_c,
            hvd_gpio,
            imd_gpio,
            ckpt_gpio,
            inertia_sw_gpio,
            tsms_gpio,
            0
        );

        bool shutdown_active = bms_gpio || bots_gpio || spare_gpio || bspd_gpio
                                || hv_c || hvd_gpio || imd_gpio || ckpt_gpio
                                || inertia_sw_gpio || tsms_gpio;

        if (shutdown_active && tsms_get() == true) { // if tsms is still on when shutdown is active, trigger fault
            queue_send(&faults, &(fault_t){SHUTDOWN_FAULT}, TX_NO_WAIT);
        }

        /* Sleep Thread for specified number of ticks. */
        tx_thread_sleep(shutdown_thread.sleep);
    }
}

/* Pedals Thread. */
static thread_t pedals_thread = {
        .name       = "Pedals Thread",        /* Name */
        .size       = 2048,                   /* Stack Size (in bytes) */
        .priority   = PRIO_vPedals,           /* Priority */
        .threshold  = 0,                      /* Preemption Threshold */
        .time_slice = TX_NO_TIME_SLICE,       /* Time Slice */
        .auto_start = TX_AUTO_START,          /* Auto Start */
        .sleep      = 1,                      /* Sleep (in ticks) */
        .function   = vPedals                 /* Thread Function */
    };
void vPedals(ULONG thread_input) {

    while(1) {

        PRINTLN_INFO("thread ran");

        pedals_process();

        /* Sleep Thread for specified number of ticks. */
        tx_thread_sleep(pedals_thread.sleep);
    }
}

/* eFuses Thread. */
static thread_t efuses_thread = {
        .name       = "eFuses Thread",        /* Name */
        .size       = 2048,                   /* Stack Size (in bytes) */
        .priority   = PRIO_vEFuses,           /* Priority */
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

        PRINTLN_INFO("thread ran");

        /* Get data. */
        efuse_data_t data = efuse_getData();

        /* Send dashboard eFuse message. */
        send_dashboard_efuse(
            data.raw[EFUSE_DASHBOARD],
            data.voltage[EFUSE_DASHBOARD],
            data.current[EFUSE_DASHBOARD],
            data.faulted[EFUSE_DASHBOARD],
            data.enabled[EFUSE_DASHBOARD]
        );
        // serial_monitor("dashboard_efuse", "raw", "%d", data.raw[EFUSE_DASHBOARD]);
        // serial_monitor("dashboard_efuse", "voltage", "%f", data.voltage[EFUSE_DASHBOARD]);
        // serial_monitor("dashboard_efuse", "current", "%f", data.current[EFUSE_DASHBOARD]);
        // serial_monitor("dashboard_efuse", "faulted?", "%d", data.faulted[EFUSE_DASHBOARD]);
        // serial_monitor("dashboard_efuse", "enabled?", "%d", data.enabled[EFUSE_DASHBOARD]);
        efuse_enable(EFUSE_DASHBOARD);


        /* Send brake eFuse message. */
        send_brake_efuse(
            data.raw[EFUSE_BRAKE],
            data.voltage[EFUSE_BRAKE],
            data.current[EFUSE_BRAKE],
            data.faulted[EFUSE_BRAKE],
            data.enabled[EFUSE_BRAKE]
        );
        // serial_monitor("brake_efuse", "raw", "%d", data.raw[EFUSE_BRAKE]);
        // serial_monitor("brake_efuse", "voltage", "%f", data.voltage[EFUSE_BRAKE]);
        // serial_monitor("brake_efuse", "current", "%f", data.current[EFUSE_BRAKE]);
        // serial_monitor("brake_efuse", "faulted?", "%d", data.faulted[EFUSE_BRAKE]);
        // serial_monitor("brake_efuse", "enabled?", "%d", data.enabled[EFUSE_BRAKE]);

        /* Send shutdown eFuse message. */
        send_shutdown_efuse(
            data.raw[EFUSE_SHUTDOWN],
            data.voltage[EFUSE_SHUTDOWN],
            data.current[EFUSE_SHUTDOWN],
            data.faulted[EFUSE_SHUTDOWN],
            data.enabled[EFUSE_SHUTDOWN]
        );
        // serial_monitor("shutdown_efuse", "raw", "%d", data.raw[EFUSE_SHUTDOWN]);
        // serial_monitor("shutdown_efuse", "voltage", "%f", data.voltage[EFUSE_SHUTDOWN]);
        // serial_monitor("shutdown_efuse", "current", "%f", data.current[EFUSE_SHUTDOWN]);
        // serial_monitor("shutdown_efuse", "faulted?", "%d", data.faulted[EFUSE_SHUTDOWN]);
        // serial_monitor("shutdown_efuse", "enabled?", "%d", data.enabled[EFUSE_SHUTDOWN]);

        /* Send LV eFuse message. */
        send_lv_efuse(
            data.raw[EFUSE_LV],
            data.voltage[EFUSE_LV],
            data.current[EFUSE_LV],
            data.faulted[EFUSE_LV],
            data.enabled[EFUSE_LV]
        );
        efuse_enable(EFUSE_LV);
        serial_monitor("lv_efuse", "raw", "%d", data.raw[EFUSE_LV]);
        serial_monitor("lv_efuse", "voltage", "%f", data.voltage[EFUSE_LV]);
        serial_monitor("lv_efuse", "current", "%f", data.current[EFUSE_LV]);
        serial_monitor("lv_efuse", "faulted?", "%d", data.faulted[EFUSE_LV]);
        serial_monitor("lv_efuse", "enabled?", "%d", data.enabled[EFUSE_LV]);


        /* Send radfan eFuse message. */
        send_radfan_efuse(
            data.raw[EFUSE_RADFAN],
            data.voltage[EFUSE_RADFAN],
            data.current[EFUSE_RADFAN],
            data.faulted[EFUSE_RADFAN],
            data.enabled[EFUSE_RADFAN]
        );
        efuse_enable(EFUSE_RADFAN);
        serial_monitor("radfan_efuse", "raw", "%d", data.raw[EFUSE_RADFAN]);
        serial_monitor("radfan_efuse", "voltage", "%f", data.voltage[EFUSE_RADFAN]);
        serial_monitor("radfan_efuse", "current", "%f", data.current[EFUSE_RADFAN]);
        serial_monitor("radfan_efuse", "faulted?", "%d", data.faulted[EFUSE_RADFAN]);
        serial_monitor("radfan_efuse", "enabled?", "%d", data.enabled[EFUSE_RADFAN]);

        /* Send fanbatt eFuse message. */
        send_fanbatt_efuse(
            data.raw[EFUSE_FANBATT],
            data.voltage[EFUSE_FANBATT],
            data.current[EFUSE_FANBATT],
            data.faulted[EFUSE_FANBATT],
            data.enabled[EFUSE_FANBATT]
        );

        /* Send pump1 eFuse message. */
        send_pumpone_efuse(
            data.raw[EFUSE_PUMP1],
            data.voltage[EFUSE_PUMP1],
            data.current[EFUSE_PUMP1],
            data.faulted[EFUSE_PUMP1],
            data.enabled[EFUSE_PUMP1]
        );

        /* Send pump2 eFuse message. */
        send_pumptwo_efuse(
            data.raw[EFUSE_PUMP2],
            data.voltage[EFUSE_PUMP2],
            data.current[EFUSE_PUMP2],
            data.faulted[EFUSE_PUMP2],
            data.enabled[EFUSE_PUMP2]
        );

        /* Send battbox eFuse message. */
        send_battbox_efuse(
            data.raw[EFUSE_BATTBOX],
            data.voltage[EFUSE_BATTBOX],
            data.current[EFUSE_BATTBOX],
            data.faulted[EFUSE_BATTBOX],
            data.enabled[EFUSE_BATTBOX]
        );

        /* Send MC eFuse message. */
        send_mc_efuse(
            data.raw[EFUSE_MC],
            data.voltage[EFUSE_MC],
            data.current[EFUSE_MC],
            data.faulted[EFUSE_MC],
            data.enabled[EFUSE_MC]
        );

        /* Sleep Thread for specified number of ticks. */
        tx_thread_sleep(efuses_thread.sleep);
    }
}

/* TSMS Thread. */
static thread_t tsms_thread = {
        .name       = "TSMS Thread",          /* Name */
        .size       = 2048,                   /* Stack Size (in bytes) */
        .priority   = PRIO_vTSMS,             /* Priority */
        .threshold  = 0,                      /* Preemption Threshold */
        .time_slice = TX_NO_TIME_SLICE,       /* Time Slice */
        .auto_start = TX_AUTO_START,          /* Auto Start */
        .sleep      = 50,                     /* Sleep (in ticks) */
        .function   = vTSMS                   /* Thread Function */
    };
void vTSMS(ULONG thread_input) {

    while(1) {

        PRINTLN_INFO("thread ran");

        tsms_update();

        /* Sleep Thread for specified number of ticks. */
        tx_thread_sleep(tsms_thread.sleep);
    }
}

/* Mux Thread (for the ADC multiplexer). */
static thread_t mux_thread = {
        .name       = "Mux Thread",           /* Name */
        .size       = 2048,                   /* Stack Size (in bytes) */
        .priority   = PRIO_vMux,              /* Priority */
        .threshold  = 0,                      /* Preemption Threshold */
        .time_slice = TX_NO_TIME_SLICE,       /* Time Slice */
        .auto_start = TX_AUTO_START,          /* Auto Start */
        .sleep      = 100,                    /* Sleep (in ticks) */
        .function   = vMux                    /* Thread Function */
    };
void vMux(ULONG thread_input) {

    while(1) {

        PRINTLN_INFO("thread ran");

        /* Switches the multiplexer state and updates the buffer. */
        adc_switchMuxState();

        /* Sleep Thread for specified number of ticks. */
        tx_thread_sleep(mux_thread.sleep);
    }
}

/* Peripherals Thread. */
static thread_t peripherals_thread = {
        .name       = "Peripherals Thread",   /* Name */
        .size       = 2048,                   /* Stack Size (in bytes) */
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

        PRINTLN_INFO("thread ran");

        /* SECTION 1: Read the temperature sensor data and send it over CAN. */
        do {
            /* Get the temp sensor data. */
            float temperature = 0;
            float humidity = 0;
            int status = tempsensor_getTemperatureAndHumidity(&temperature, &humidity);
            if(status != U_SUCCESS) {
                PRINTLN_ERROR("Failed to call tempsensor_getTemperatureAndHumidity() in the peripherals thread (Status: %d). This means that SECTION 1 of vPeripherals has been skipped.", status);
                queue_send(&faults, &(fault_t){ONBOARD_TEMP_FAULT}, TX_NO_WAIT);
                break; // Break from SECTION 1. We don't want to send the CAN message if reading the data failed.
            }

            PRINTLN_INFO("SHT30 Temp: %f", temperature);

            serial_monitor("peripherals", "sht30 temp", "%f", temperature);

            /* Send the temp sensor message. */
            send_temperature_sensor(
                temperature,
                humidity
            );
        } while (0);

        /* SECTION 2: Read IMU acceleration data and send it over CAN. */
        do {
            /* Get the IMU acceleration data. */
            vector3_t acceleration;
            int status = imu_getAcceleration(&acceleration);
            if(status != U_SUCCESS) {
                PRINTLN_ERROR("Failed to call imu_getAcceleration() in the peripherals thread (Status: %d). This means that SECTION 2 of vPeripherals has been skipped.", status);
                queue_send(&faults, &(fault_t){IMU_ACCEL_FAULT}, TX_NO_WAIT);
                break; // Break from SECTION 2. We don't want to send the CAN message if reading the data failed.
            }

            serial_monitor("peripherals", "imu_acceleration_x", "%f", acceleration.x);
            serial_monitor("peripherals", "imu_acceleration_y", "%f", acceleration.y);
            serial_monitor("peripherals", "imu_acceleration_z", "%f", acceleration.z);

            /* Send the IMU acceleration message. */
            send_imu_accelerometer(
                acceleration.x,
                acceleration.y,
                acceleration.z
            );
        } while (0);

        /* SECTION 3: Read IMU gyro data and send it over CAN. */
        do {
            /* Get the IMU Gyro data. */
            vector3_t gyro;
            int status = imu_getAngularRate(&gyro);
            if(status != U_SUCCESS) {
                PRINTLN_ERROR("Failed to call imu_getAngularRate() in the peripherals thread (Status: %d). This means that SECTION 3 of vPeripherals has been skipped.", status);
                queue_send(&faults, &(fault_t){IMU_GYRO_FAULT}, TX_NO_WAIT);
                break; // Break from SECTION 3. We don't want to send the CAN message if reading the data failed.
            }

            serial_monitor("peripherals", "imu_gyro_x", "%f", gyro.x);
            serial_monitor("peripherals", "imu_gyro_y", "%f", gyro.y);
            serial_monitor("peripherals", "imu_gyro_z", "%f", gyro.z);

            /* Send the IMU Gyro message. */
            send_imu_gyro(
                gyro.x,
                gyro.y,
                gyro.z
            );
        } while (0);

        /* SECTION 4: Send LV ADC Message. */
        do {
            lvread_adc_t lv_data = adc_getLVData();

            /* Send the LV Voltage message. */
            send_lv_voltage(
                lv_data.raw,
                lv_data.voltage
            );

            static const float LV_LOW_VOLTAGE_THRESHOLD = 23.5f;
            if (lv_data.voltage < LV_LOW_VOLTAGE_THRESHOLD) {
                queue_send(&faults, &(fault_t){LV_LOW_VOLTAGE_FAULT}, TX_NO_WAIT);
            }

        } while (0);

        /* Sleep Thread for specified number of ticks. */
        tx_thread_sleep(peripherals_thread.sleep);
    }
}

/* Initializes all ThreadX threads.
*  Calls to _create_thread() should go in here
*/
uint8_t threads_init(TX_BYTE_POOL *byte_pool) {

    /* Create Threads */
    //CATCH_ERROR(create_thread(byte_pool, &default_thread), U_SUCCESS);           // Create Default thread.
    //CATCH_ERROR(create_thread(byte_pool, &can_incoming_thread), U_SUCCESS);      // Create Incoming CAN thread.
    CATCH_ERROR(create_thread(byte_pool, &can_outgoing_thread), U_SUCCESS);      // Create Outgoing CAN thread.
    //CATCH_ERROR(create_thread(byte_pool, &faults_queue_thread), U_SUCCESS);      // Create Faults Queue thread.
    //CATCH_ERROR(create_thread(byte_pool, &faults_thread), U_SUCCESS);            // Create Faults thread.
    //CATCH_ERROR(create_thread(byte_pool, &tsms_thread), U_SUCCESS);              // Create TSMS thread.
    CATCH_ERROR(create_thread(byte_pool, &shutdown_thread), U_SUCCESS);          // Create Shutdown thread.
    //CATCH_ERROR(create_thread(byte_pool, &statemachine_thread), U_SUCCESS);      // Create State Machine thread.
    //CATCH_ERROR(create_thread(byte_pool, &pedals_thread), U_SUCCESS);            // Create Pedals thread.
    //CATCH_ERROR(create_thread(byte_pool, &efuses_thread), U_SUCCESS);              // Create eFuses thread.
    //CATCH_ERROR(create_thread(byte_pool, &mux_thread), U_SUCCESS);               // Create Mux thread.
    CATCH_ERROR(create_thread(byte_pool, &peripherals_thread), U_SUCCESS);       // Create Peripherals thread.
    //CATCH_ERROR(create_thread(byte_pool, &ethernet_incoming_thread), U_SUCCESS); // Create Incoming Ethernet thread.
    CATCH_ERROR(create_thread(byte_pool, &ethernet_outgoing_thread), U_SUCCESS); // Create Outgoing Ethernet thread.
    CATCH_ERROR(create_thread(byte_pool, &test_thread), U_SUCCESS);                // Create Test thread.

    // add more threads here if need

    PRINTLN_INFO("Ran threads_init().");
    return U_SUCCESS;
}
