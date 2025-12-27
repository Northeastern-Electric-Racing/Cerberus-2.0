#include "sht30.h"
#include "u_tx_debug.h"
#include "main.h"
#include "u_peripherals.h"
#include "u_mutexes.h"

#define IMU_MAX_BUFFER_SIZE 64 /* Max buffer size for the IMU VLAs. */

/* Wrapper for lsm6dsv SPI reading. */
static int32_t _lsm6dsv_read(void* spi_handle, uint8_t reg, uint8_t* buffer, uint16_t length) {
    /* Prevent stack overflow? */
    if((length + 1) > IMU_MAX_BUFFER_SIZE) {
        PRINTLN_ERROR("IMU buffer length is greater than IMU_MAX_BUFFER_SIZE, so cannot read from IMU (length+1=%d, IMU_MAX_BUFFER_SIZE=%d).", (length+1), IMU_MAX_BUFFER_SIZE);
        return -1;
    }
    
    uint8_t tx_buffer[length + 1];
    uint8_t rx_buffer[length + 1];
    
    tx_buffer[0] = reg | 0x80; // For SPI reads, set MSB = 1 for read operation.
    memset(&tx_buffer[1], 0x00, length);
    
    HAL_GPIO_WritePin(SPI2_NSS_GPIO_Port, SPI2_NSS_Pin, GPIO_PIN_RESET); // Setting the CS pin LOW selects the IMU for SPI.
    
    /* Recieve the data. */
    HAL_StatusTypeDef status = HAL_SPI_TransmitReceive((SPI_HandleTypeDef*)spi_handle, tx_buffer, rx_buffer, length + 1, HAL_MAX_DELAY);
    if(status != HAL_OK) {
        HAL_GPIO_WritePin(SPI2_NSS_GPIO_Port, SPI2_NSS_Pin, GPIO_PIN_SET);
        PRINTLN_ERROR("Failed to recieve data to the IMU over SPI (Status: %d/%s).", status, hal_status_toString(status));
        return -1;
    }

    HAL_GPIO_WritePin(SPI2_NSS_GPIO_Port, SPI2_NSS_Pin, GPIO_PIN_SET); // Setting the CS pin HIGH deselects the IMU for SPI.
    
    memcpy(buffer, &rx_buffer[1], length);
    
    return 0;
}

/* Wrapper for lsm6dsv SPI writing. */
static int32_t _lsm6dsv_write(void* spi_handle, uint8_t reg, const uint8_t* data, uint16_t length) {
    /* Prevent stack overflow? */
    if((length + 1) > IMU_MAX_BUFFER_SIZE) {
        PRINTLN_ERROR("IMU buffer length is greater than IMU_MAX_BUFFER_SIZE, so cannot write to IMU (length+1=%d, IMU_MAX_BUFFER_SIZE=%d).", (length+1), IMU_MAX_BUFFER_SIZE);
        return -1;
    }
    
    uint8_t tx_buffer[length + 1];
    tx_buffer[0] = reg & 0x7F; // For SPI writes, clear MSB = 0 for write operation.
    memcpy(&tx_buffer[1], data, length);
    
    HAL_GPIO_WritePin(SPI2_NSS_GPIO_Port, SPI2_NSS_Pin, GPIO_PIN_RESET); // Setting the CS pin LOW selects the IMU for SPI.
    
    /* Transmit the data. */
    HAL_StatusTypeDef status = HAL_SPI_Transmit((SPI_HandleTypeDef*)spi_handle, tx_buffer, length + 1, HAL_MAX_DELAY);
    if(status != HAL_OK) {
        HAL_GPIO_WritePin(SPI2_NSS_GPIO_Port, SPI2_NSS_Pin, GPIO_PIN_SET);
        PRINTLN_ERROR("Failed to transmit data to the IMU over SPI (Status: %d/%s).", status, hal_status_toString(status));
        return -1;
    }

    HAL_GPIO_WritePin(SPI2_NSS_GPIO_Port, SPI2_NSS_Pin, GPIO_PIN_SET); // Setting the CS pin HIGH deselects the IMU for SPI.
    return 0;
}

/* Wrapper for sht30 I2C reading. */
static int _sht30_read(uint8_t *data, uint16_t command, uint8_t device_address, uint8_t length) {
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(&hi2c2, (uint16_t)device_address, command, I2C_MEMADD_SIZE_16BIT, data, length, HAL_MAX_DELAY);
    if(status != HAL_OK) {
        PRINTLN_ERROR("Failed to call _sht30_read() (Status: %d/%s, Command: %d).", status, hal_status_toString(status), command);
        return status;
    }
    return status;
}

/* Wrapper for sht30 I2C writing. */
static int _sht30_write(uint8_t *data, uint8_t device_address, uint8_t length) {
    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(&hi2c2, (uint16_t)device_address, data, (uint16_t)length, HAL_MAX_DELAY);
    if(status != HAL_OK) {
        PRINTLN_ERROR("Failed to call _sht30_write() (Status: %d/%s).", status, hal_status_toString(status));
        return status;
    }
    return status;
}

/* Driver instances. */
static sht30_t temperature_sensor = { .dev_address = SHT30_I2C_ADDR };
static const stmdev_ctx_t imu = {
    .handle = &hspi2,
    .read_reg = _lsm6dsv_read,
    .write_reg = _lsm6dsv_write
};

/* Initializes I2C devices. */
int peripherals_init(void) {

    /* Initialize temperature sensor. */
    int status = sht30_init(&temperature_sensor, _sht30_write, _sht30_read, _sht30_read, temperature_sensor.dev_address);
    if(status != 0) {
        PRINTLN_ERROR("Failed to run sht30_init() (Status: %d).", status);
        return U_ERROR;
    }

    /* Make sure IMU is set up correctly. */
    uint8_t id;
    status = lsm6dsv_device_id_get(&imu, &id);
    if(status != 0) {
        PRINTLN_ERROR("Failed to call lsm6dsv_device_id_get() (Status: %d).", status);
        return U_ERROR;
    }
    if(id != LSM6DSV_ID) {
        PRINTLN_ERROR("lsm6dsv_device_id_get() returned an unexpected ID (id=%d, expected=%d). This means that the IMU is not configured correctly.", id, LSM6DSV_ID);
        return U_ERROR;
    }

    /* Reset IMU. */
    status = lsm6dsv_reset_set(&imu, LSM6DSV_GLOBAL_RST);
    if(status != 0) {
        PRINTLN_ERROR("Failed to reset the IMU via lsm6dsv_reset_set() (Status: %d).", status);
        return U_ERROR;
    }
    HAL_Delay(30); // This is probably overkill, but the datasheet lists the gyroscope's "Turn-on time" as 30ms, and I can't find anything else that specifies how long resets take.

    /* Enable Block Data Update. */
    status = lsm6dsv_block_data_update_set(&imu, PROPERTY_ENABLE); // Makes it so "output registers are not updated until LSB and MSB have been read". Datasheet says this is enabled by default but figured it was better to be explicit.
    if(status != 0) {
        PRINTLN_ERROR("Failed to enable Block Data Update via lsm6dsv_block_data_update_set() (Status: %d).", status);
        return U_ERROR;
    }

    /* Set Accelerometer Full Scale. */
    status = lsm6dsv_xl_full_scale_set(&imu, LSM6DSV_2g);
    if(status != 0) {
        PRINTLN_ERROR("Failed to set IMU Accelerometer Full Scale via lsm6dsv_xl_full_scale_set() (Status: %d).", status);
        return U_ERROR;
    }

    /* Set gyroscope full scale. */
    status = lsm6dsv_gy_full_scale_set(&imu, LSM6DSV_2000dps);
    if(status != 0) {
        PRINTLN_ERROR("Failed to set IMU Gyroscope Full Scale via lsm6dsv_gy_full_scale_set() (Status: %d).", status);
        return U_ERROR;
    }

    /* Set accelerometer output data rate. */
    status = lsm6dsv_xl_data_rate_set(&imu, LSM6DSV_ODR_AT_120Hz);
    if(status != 0) {
        PRINTLN_ERROR("Failed to set IMU Accelerometer Datarate via lsm6dsv_xl_data_rate_set() (Status: %d).", status);
        return U_ERROR;
    }

    /* Set gyroscope output data rate. */
    status = lsm6dsv_gy_data_rate_set(&imu, LSM6DSV_ODR_AT_120Hz);
    if(status != 0) {
        PRINTLN_ERROR("Failed to set IMU Gyroscope Datarate via lsm6dsv_gy_data_rate_set() (Status: %d).", status);
        return U_ERROR;
    }

    PRINTLN_INFO("Ran peripherals_init().");
    return U_SUCCESS;
}

/* Toggles the status of the temperature sensor's internal heater. */
int tempsensor_toggleHeater(bool enable) {
    mutex_get(&peripherals_mutex);
    HAL_StatusTypeDef status = sht30_toggle_heater(&temperature_sensor, enable);
    mutex_put(&peripherals_mutex);
    if(status != HAL_OK) {
        PRINTLN_ERROR("Failed to toggle SHT30 heater (Status: %d/%s).", status, hal_status_toString(status));
        return U_ERROR;
    }
    return U_SUCCESS;
}

/* Gets the temp sensor's temperature and humidity readings. */
int tempsensor_getTemperatureAndHumidity(float *temperature, float *humidity) {
    mutex_get(&peripherals_mutex);
    int status = sht30_get_temp_humid(&temperature_sensor);
    mutex_put(&peripherals_mutex);
    if(status != 0) {
        PRINTLN_ERROR("Failed to read SHT30 temperature/humidity (Status: %d).", status);
        return U_ERROR;
    }

    *temperature = temperature_sensor.temp;
    *humidity = temperature_sensor.humidity;
    return U_SUCCESS;
}

/* Gets the IMU's acceleration reading. */
int imu_getAcceleration(vector3_t* data) {
    int16_t raw_data[3];
    
    /* Read raw accelerometer data. */
    mutex_get(&peripherals_mutex);
    int status = lsm6dsv_acceleration_raw_get(&imu, raw_data);
    mutex_put(&peripherals_mutex);
    if(status != 0) {
        PRINTLN_ERROR("Failed to read accelerometer data.");
        return U_ERROR;
    }
    
    /* Convert to mg (milligravity). */
    data->x = lsm6dsv_from_fs2_to_mg(raw_data[0]); // Somewhat important: These functions MUST match the full-scale settings configured in peripherals_init(). The conversions will be incorrect if you use the wrong functions.
    data->y = lsm6dsv_from_fs2_to_mg(raw_data[1]);
    data->z = lsm6dsv_from_fs2_to_mg(raw_data[2]);
    
    return U_SUCCESS;
}

/* Gets the IMU's angular rate reading. */
int imu_getAngularRate(vector3_t* data) {
    int16_t raw_data[3];
    
    /* Read raw gyroscope data. */
    mutex_get(&peripherals_mutex);      // u_TODO - All of these functions have to get the peripherals mutex. Since these funtions are (as of rn) being called on-after-another by the peripherals thread, this is inefficient (ideally, the mutex would just need to aquired once and put back once). It could be a good idea to have a convention where it's the caller's responsibility to acquire the mutex for these sorts of functions, and then have some kind of check in this funciton to make sure the mutex is owned by the caller's thread
    int status = lsm6dsv_angular_rate_raw_get(&imu, raw_data);
    mutex_put(&peripherals_mutex);
    if(status != 0) {
        PRINTLN_ERROR("Failed to read gyroscope data.");
        return U_ERROR;
    }
    
    /* Convert to mdps (millidegrees per second). */
    data->x = lsm6dsv_from_fs2000_to_mdps(raw_data[0]); // Somewhat important: These functions MUST match the full-scale settings configured in peripherals_init(). The conversions will be incorrect if you use the wrong functions.
    data->y = lsm6dsv_from_fs2000_to_mdps(raw_data[1]);
    data->z = lsm6dsv_from_fs2000_to_mdps(raw_data[2]);
    
    return U_SUCCESS;
}