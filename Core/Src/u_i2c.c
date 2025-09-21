#include "sht30.h"
#include "u_i2c.h"
#include "u_general.h"

static sht30_t temperature_sensor = {
    .dev_address = SHT30_I2C_ADDR // u_TODO - This may need to be shifted left? But probably not
};

static int _sht30_read(uint8_t *data, uint16_t command, uint8_t device_address, uint8_t length) {
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(&hi2c2, (uint16_t)device_address, command, I2C_MEMADD_SIZE_16BIT, data, length, HAL_MAX_DELAY);
    if(status != HAL_OK) {
        DEBUG_PRINTLN("ERROR: Failed to call _sht30_read() (Status: %d/%s, Command: %d).", status, hal_status_toString(status), command);
        return status;
    }
    return status;
}

static int _sht30_write(uint8_t *data, uint8_t device_address, uint8_t length) {
    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(&hi2c2, (uint16_t)device_address, data, (uint16_t)length, HAL_MAX_DELAY);
    if(status != HAL_OK) {
        DEBUG_PRINTLN("ERROR: Failed to call _sht30_write() (Status: %d/%s).", status, hal_status_toString(status));
        return status;
    }
    return status;
}

/* Initializes I2C devices. */
int i2c_init(void) {

    int status = sht30_init(&temperature_sensor, _sht30_write, _sht30_read, _sht30_read, temperature_sensor.dev_address);
    if(status != 0) {
        DEBUG_PRINTLN("ERROR: Failed to run sht30_init (Status: %d).", status);
        return U_ERROR;
    }

    DEBUG_PRINTLN("Ran i2c_init().");
    return U_SUCCESS;
}

/* Toggles the status of the temperature sensor's internal heater. */
int tempsensor_toggleHeater(bool enable) {
    HAL_StatusTypeDef status = sht30_toggle_heater(&temperature_sensor, enable);
    if(status != HAL_OK) {
        DEBUG_PRINTLN("ERROR: Failed to toggle SHT30 heater (Status: %d/%s).", status, hal_status_toString(status));
        return U_ERROR;
    }
    return U_SUCCESS;
}

/* Gets the temp sensor's temperature reading. */
int tempsensor_getTemperature(float *temperature) {
    int status = sht30_get_temp_humid(&temperature_sensor);
    if(status != 0) {
        DEBUG_PRINTLN("Failed to read SHT30 temperature/humidity (Status: %d).", status);
        return U_ERROR;
    }
    *temperature = temperature_sensor.humidity;
    return U_SUCCESS;
}

/* Gets the temp sensor's humidity reading. */
int tempsensor_getHumidity(float *humidity) {
    int status = sht30_get_temp_humid(&temperature_sensor);
    if(status != 0) {
        DEBUG_PRINTLN("Failed to read SHT30 temperature/humidity (Status: %d).", status);
        return U_ERROR;
    }
    *humidity = temperature_sensor.humidity;
    return U_SUCCESS;
}