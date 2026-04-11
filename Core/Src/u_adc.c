#include <stdint.h>
#include "u_tx_debug.h"
#include "main.h"
#include "u_mutexes.h"
#include "u_adc.h"
#include "serial.h"

/* ADC1 Config. */
/* For mux'd inputs: SELx=HIGH corresponds to the A input. SELx=LOW corresponds to the B input. */
typedef enum {
    /* ADC Input Channel Ranks. */
    /* The order these values MUST match how the ADC ranks are set up in CubeMX. */
    ADC1_CHANNEL3,  // EF_DASH_ADC
    ADC1_CHANNEL0,  // Mux 1
    ADC1_CHANNEL5,  // Mux 3
    ADC1_CHANNEL9,  // Mux 4
    ADC1_CHANNEL6,  // EF_FANBATT_ADC
    ADC1_CHANNEL2,  // EF_PUMP1_ADC
    ADC1_CHANNEL13, // EF_PUMP2_ADC
    ADC1_CHANNEL18, // EF_MC_ADC
    ADC1_CHANNEL15, // Mux 2

    /* Total number of channels/indexes for ADC1. */
    ADC1_SIZE,
} _adc1_t;
static volatile uint16_t _adc1_buffer[ADC1_SIZE]; // Buffer for the ADC DMA readings (note: has to be uint16_t to correspond with the "Half Word" GPDMA setting in CubeMX).

/* ADC2 Config. */
typedef enum {
    /* The order of items in this enum MUST match how the ADC ranks are set up in CubeMX. */

    ADC2_CHANNEL12, // APPS_1_ADC
    ADC2_CHANNEL10, // APPS_2_ADC
    ADC2_CHANNEL2,  // BSE_1_ADC
    ADC2_CHANNEL6,  // BSE_2_ADC

    /* Total number of indexes for ADC2. */
    ADC2_SIZE
} _adc2_t;
static volatile uint16_t _adc2_buffer[ADC2_SIZE]; // Buffer for the ADC DMA readings (note: has to be uint16_t to correspond with the "Half Word" GPDMA setting in CubeMX).

/* Multiplexer buffer. */
typedef enum {
    /* SEL1 */
    SEL1_HIGH, // BREAKLIGHT_ADC
    SEL1_LOW,  // BATTBOX_ADC

    /* SEL2 */
    SEL2_HIGH,  // LFIU_CURRENT_1
    SEL2_LOW,   // LFIU_CURRENT_2

    /* SEL3 */
    SEL3_HIGH,  // LV_ADC
    SEL3_LOW,   // SHUTDOWN_ADC

    /* SEL4 */
    SEL4_HIGH,  // RADFAN_ADC
    SEL4_LOW,   // LV_BATT_ADC

    /* Total number of indexes for the multiplexer buffer. */
    MUX_SIZE
} _mux_t;
static volatile uint16_t _mux_buffer[MUX_SIZE] = { 0 };

/* Manage muxs and updates the mux buffer. */
typedef enum { HIGH, LOW } _mux_state_t;
static _mux_state_t mux_state_debug = LOW;
int adc_switchMuxState(void) {
    static _mux_state_t mux_state = LOW;

    if(mux_state == LOW) {
        /* Mux is currently LOW, so update LOW values. */
        _mux_buffer[SEL1_LOW] = _adc1_buffer[ADC1_CHANNEL0];
        _mux_buffer[SEL2_LOW] = _adc1_buffer[ADC1_CHANNEL15];
        _mux_buffer[SEL3_LOW] = _adc1_buffer[ADC1_CHANNEL5];
        _mux_buffer[SEL4_LOW] = _adc1_buffer[ADC1_CHANNEL9];

        /* Mux is currently LOW, so switch to HIGH. */
        HAL_GPIO_WritePin(MUX_SEL1_GPIO_Port, MUX_SEL1_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(MUX_SEL2_GPIO_Port, MUX_SEL2_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(MUX_SEL3_GPIO_Port, MUX_SEL3_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(MUX_SEL4_GPIO_Port, MUX_SEL4_Pin, GPIO_PIN_SET);

        tx_thread_sleep(10); // Sleep for 10 ticks so the mux settles.
        mux_state = HIGH;   // Update the mux state

        PRINTLN_INFO("Switched mux_state to HIGH.");
    }
    else if(mux_state == HIGH) {

        /* Mux is currently HIGH, so update HIGH values. */
        _mux_buffer[SEL1_HIGH] = _adc1_buffer[ADC1_CHANNEL0];
        _mux_buffer[SEL2_HIGH] = _adc1_buffer[ADC1_CHANNEL15];
        _mux_buffer[SEL3_HIGH] = _adc1_buffer[ADC1_CHANNEL5];
        _mux_buffer[SEL4_HIGH] = _adc1_buffer[ADC1_CHANNEL9];

        /* Mux is currently HIGH, so switch to LOW. */
        HAL_GPIO_WritePin(MUX_SEL1_GPIO_Port, MUX_SEL1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(MUX_SEL2_GPIO_Port, MUX_SEL2_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(MUX_SEL3_GPIO_Port, MUX_SEL3_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(MUX_SEL4_GPIO_Port, MUX_SEL4_Pin, GPIO_PIN_RESET);

        tx_thread_sleep(10); // Sleep for 10 ticks so the mux settles.
        mux_state = LOW;   // Update the mux state

        PRINTLN_INFO("Switched mux_state to LOW.");
    }
    mux_state_debug = mux_state;

    PRINTLN_INFO("Ran adc_switchMuxState()");

    return U_SUCCESS;
}

/* Start ADC DMA. */
int adc_init(void) {
    /* Start DMA for ADC1. */
    int status = HAL_ADC_Start_DMA(&hadc1, (uint32_t *) _adc1_buffer, ADC1_SIZE);
    if(status != HAL_OK) {
        PRINTLN_ERROR("Failed to start ADC DMA for ADC1 (Status: %d/%s).", status, hal_status_toString(status));
        return U_ERROR;
    }

    /* Start DMA for ADC2. */
    status = HAL_ADC_Start_DMA(&hadc2, (uint32_t *) _adc2_buffer, ADC2_SIZE);
    if(status != HAL_OK) {
        PRINTLN_ERROR("Failed to start ADC DMA for ADC2 (Status: %d/%s).", status, hal_status_toString(status));
        return U_ERROR;
    }

    PRINTLN_INFO("Ran adc_init().");
    return U_SUCCESS;
}

/* Get raw eFuse ADC Data. */
raw_efuse_adc_t adc_getEFuseData(void) {
    raw_efuse_adc_t efuses = { 0 };

    efuses.data[EFUSE_DASHBOARD] = _adc1_buffer[ADC1_CHANNEL3];
    efuses.data[EFUSE_BRAKE] = _mux_buffer[SEL1_HIGH];
    efuses.data[EFUSE_SHUTDOWN] = _mux_buffer[SEL3_LOW];
    efuses.data[EFUSE_LV] = _mux_buffer[SEL3_HIGH];
    efuses.data[EFUSE_RADFAN] = _mux_buffer[SEL4_HIGH];
    efuses.data[EFUSE_FANBATT] = _adc1_buffer[ADC1_CHANNEL6];
    efuses.data[EFUSE_PUMP1] = _adc1_buffer[ADC1_CHANNEL2];
    efuses.data[EFUSE_PUMP2] = _adc1_buffer[ADC1_CHANNEL13];
    efuses.data[EFUSE_BATTBOX] = _mux_buffer[SEL1_LOW];
    efuses.data[EFUSE_MC] = _adc1_buffer[ADC1_CHANNEL18];

    // serial_monitor("adc1", "mux_state_debug (0=HIGH, 1=LOW)", "%d", mux_state_debug);
    // serial_monitor("efuse_fanbatt", "_adc1_buffer[ADC1_CHANNEL6]=", "%d", _adc1_buffer[ADC1_CHANNEL6]);

    // serial_monitor("adc1", "ADC1_CHANNEL3", "%d", _adc1_buffer[ADC1_CHANNEL3]);
    // serial_monitor("adc1", "ADC1_CHANNEL0 (mux)", "%d", _adc1_buffer[ADC1_CHANNEL0]);
    // serial_monitor("adc1", "ADC1_CHANNEL5 (mux)", "%d", _adc1_buffer[ADC1_CHANNEL5]);
    // serial_monitor("adc1", "ADC1_CHANNEL9 (mux)", "%d", _adc1_buffer[ADC1_CHANNEL9]);
    // serial_monitor("adc1", "ADC1_CHANNEL6", "%d", _adc1_buffer[ADC1_CHANNEL6]);
    // serial_monitor("adc1", "ADC1_CHANNEL2", "%d", _adc1_buffer[ADC1_CHANNEL2]);
    // serial_monitor("adc1", "ADC1_CHANNEL13", "%d", _adc1_buffer[ADC1_CHANNEL13]);
    // serial_monitor("adc1", "ADC1_CHANNEL18", "%d", _adc1_buffer[ADC1_CHANNEL18]);
    // serial_monitor("adc1", "ADC1_CHANNEL15 (mux)", "%d", _adc1_buffer[ADC1_CHANNEL15]);
    // serial_monitor("adc1", "mux_state (0=HIGH, 1=LOW)", "%d", mux_state_debug);

    // serial_monitor("mux_debug", "SEL1_LOW", "%d", _mux_buffer[SEL1_LOW]);
    // serial_monitor("mux_debug", "SEL1_HIGH", "%d", _mux_buffer[SEL1_HIGH]);
    // serial_monitor("mux_debug", "SEL2_LOW", "%d", _mux_buffer[SEL2_LOW]);
    // serial_monitor("mux_debug", "SEL2_HIGH", "%d", _mux_buffer[SEL2_HIGH]);
    // serial_monitor("mux_debug", "SEL3_LOW", "%d", _mux_buffer[SEL3_LOW]);
    // serial_monitor("mux_debug", "SEL3_HIGH", "%d", _mux_buffer[SEL3_HIGH]);
    // serial_monitor("mux_debug", "SEL4_LOW", "%d", _mux_buffer[SEL4_LOW]);
    // serial_monitor("mux_debug", "SEL4_HIGH", "%d", _mux_buffer[SEL4_HIGH]);

    // serial_monitor("raw_efuse", "EFUSE_DASHBOARD", "%d", efuses.data[EFUSE_DASHBOARD]);
    // serial_monitor("raw_efuse", "EFUSE_BRAKE", "%d", efuses.data[EFUSE_BRAKE]);
    // serial_monitor("raw_efuse", "EFUSE_SHUTDOWN", "%d", efuses.data[EFUSE_SHUTDOWN]);
    // serial_monitor("raw_efuse", "EFUSE_LV", "%d", efuses.data[EFUSE_LV]);
    // serial_monitor("raw_efuse", "EFUSE_RADFAN", "%d", efuses.data[EFUSE_RADFAN]);
    // serial_monitor("raw_efuse", "EFUSE_FANBATT", "%d", efuses.data[EFUSE_FANBATT]);
    // serial_monitor("raw_efuse", "EFUSE_PUMP1", "%d", efuses.data[EFUSE_PUMP1]);
    // serial_monitor("raw_efuse", "EFUSE_PUMP2", "%d", efuses.data[EFUSE_PUMP2]);
    // serial_monitor("raw_efuse", "EFUSE_BATTBOX", "%d", efuses.data[EFUSE_BATTBOX]);
    // serial_monitor("raw_efuse", "EFUSE_MC", "%d", efuses.data[EFUSE_MC]);

    return efuses;
}

/* Get raw pedal sensor ADC Data. */
raw_pedal_adc_t adc_getPedalData(void) {
    raw_pedal_adc_t sensors = { 0 };

    sensors.data[PEDAL_ACCEL1] = _adc2_buffer[ADC2_CHANNEL12];
    sensors.data[PEDAL_ACCEL2] = _adc2_buffer[ADC2_CHANNEL10];
    sensors.data[PEDAL_BRAKE1] = _adc2_buffer[ADC2_CHANNEL2];
    sensors.data[PEDAL_BRAKE2] = _adc2_buffer[ADC2_CHANNEL6];

    return sensors;
}

/* Get raw LFIU sensor ADC Data. */
lfiu_adc_t adc_getLfiuData(void) {
    lfiu_adc_t sensors = { 0 };

    /* Get the raw ADC values. */
    sensors.raw[LFIU_1] = _mux_buffer[SEL2_HIGH];
    sensors.raw[LFIU_2] = _mux_buffer[SEL2_LOW];

    /* Calculate the ADC voltage. */
    const float V_REF = 3.3f;
    sensors.voltage[LFIU_1] = (sensors.raw[LFIU_1] / 4095.0) * V_REF;
    sensors.voltage[LFIU_2] = (sensors.raw[LFIU_2] / 4095.0) * V_REF;

    /* Calculate the LFIU_1 current. */
    sensors.current[LFIU_1] = ((13.333f * sensors.voltage[LFIU_1]) - 20.0f);
    // This conversion is based on the linear fit function: f(x) = 13.333x - 20
    // This fit was created from these three datapoints provided by the electrical team:
    // 0.0V : -20A
    // 1.5V : 0A
    // 3.0V : 20A

    /* Calculate the LFIU_2 current. */
    sensors.current[LFIU_2] = ((133.333f * sensors.voltage[LFIU_2]) - 200.0f);
    // This conversion is based on the linear fit function: f(x) = 133.333x - 200
    // This fit was created from these three datapoints provided by the electrical team:
    // 0.0V : -200A
    // 1.5V : 0A
    // 3.0V : 200A

    return sensors;
}

/* Get LV_BATT Voltage ADC data. */
lvread_adc_t adc_getLVData(void) {
    lvread_adc_t data = { 0 };

    /* Get the raw ADC reading. */
    data.raw = _mux_buffer[SEL4_LOW];

    /* Calcualte the ADC voltage. */
    const float v_ref = 3.3f; // VREF is 3V3 for VCU.
    float adc_voltage = (data.raw / 4095.0) * v_ref; // adc_voltage = (adc_raw / 4095.0) * 3.3

    /* Scale the ADC Voltage back up to 24V. */
    // Before the ADC, there's a voltage divider that scales 24V down to 2.18V for the ADC. Here's the relavent math: Vout = Vin*(R2/(R1+R2)) = 24*(10,000/(100,000 + 10,000)) = 2.18V
    // This means that 2.18V is the value corresponding to 24V (even though v_ref=3.3V).
    // The adc_voltage should thus be scaled up by around *11 (since 24V/2.18V = 11.0):
    data.voltage = adc_voltage * 11.0;

    /* temporary correction scaling since the ADC  is messed up rn */
    // double x = adc_voltage;
    // float actual_voltage = -39.079629 + 98.844514 * x - 66.973026 * x * x + 17.188363 * x * x * x;
    // data.voltage = actual_voltage;

    return data;
}
