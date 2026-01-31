#include "sht30.h"
#include "u_tx_debug.h"
#include "main.h"
#include "u_peripherals.h"
#include "u_mutexes.h"

/* Wrapper for lsm6dsv SPI reading. */
static int32_t _lsm6dsv_read(void* spi_handle, uint8_t reg, uint8_t* buffer, uint16_t length) {
    
    SPI_HandleTypeDef *handle = (SPI_HandleTypeDef *)spi_handle;
    HAL_StatusTypeDef status;

    /* Select the IMU by setting its CS pin LOW. */
    HAL_GPIO_WritePin(IMU_CS_GPIO_Port, IMU_CS_Pin, GPIO_PIN_RESET);
    
    /* Tell the IMU you want to read from 'reg'. */
    uint8_t spi_reg = (uint8_t)(reg | 0b10000000); // Bits 0 through 6 store 'reg' (the register address), while Bit 7 lets you chose if it's a read or write operation (1=read, 0=write).
    status = HAL_SPI_Transmit(handle, &spi_reg, sizeof(spi_reg), HAL_MAX_DELAY);
    if(status != HAL_OK) {
        PRINTLN_ERROR("Failed to call HAL_SPI_Transmit() to write the first SPI command (Status: %d/%s).", status, hal_status_toString(status));
        HAL_GPIO_WritePin(IMU_CS_GPIO_Port, IMU_CS_Pin, GPIO_PIN_SET); // Deselect IMU since error.
        return -1;
    }

    /* Read from 'reg'. */
    status = HAL_SPI_Receive(handle, buffer, length, HAL_MAX_DELAY);
    if(status != HAL_OK) {
        PRINTLN_ERROR("Failed to call HAL_SPI_Receive() to read from 'reg' (Status: %d/%s).", status, hal_status_toString(status));
        HAL_GPIO_WritePin(IMU_CS_GPIO_Port, IMU_CS_Pin, GPIO_PIN_SET); // Deselect IMU since error.
        return -1;
    }

    /* Deselect the IMU by setting its CS pin HIGH. */
    HAL_GPIO_WritePin(IMU_CS_GPIO_Port, IMU_CS_Pin, GPIO_PIN_SET);
    
    return 0;
}

/* Wrapper for lsm6dsv SPI writing. */
static int32_t _lsm6dsv_write(void* spi_handle, uint8_t reg, const uint8_t* data, uint16_t length) {
    
    SPI_HandleTypeDef *handle = (SPI_HandleTypeDef *)spi_handle;
    HAL_StatusTypeDef status;

    /* Select the IMU by setting its CS pin LOW. */
    HAL_GPIO_WritePin(IMU_CS_GPIO_Port, IMU_CS_Pin, GPIO_PIN_RESET);
    
    /* Tell the IMU you want to write to 'reg'. */
    uint8_t spi_reg = (uint8_t)(reg & 0b01111111); // Bits 0 through 6 store 'reg' (the register address), while Bit 7 lets you chose if it's a read or write operation (1=read, 0=write).
    status = HAL_SPI_Transmit(handle, &spi_reg, sizeof(spi_reg), HAL_MAX_DELAY);
    if(status != HAL_OK) {
        PRINTLN_ERROR("Failed to call HAL_SPI_Transmit() to write the first SPI command (Status: %d/%s).", status, hal_status_toString(status));
        HAL_GPIO_WritePin(IMU_CS_GPIO_Port, IMU_CS_Pin, GPIO_PIN_SET); // Deselect IMU since error.
        return -1;
    }

    /* Write to 'reg'. */
    status = HAL_SPI_Transmit(handle, data, length, HAL_MAX_DELAY);
    if(status != HAL_OK) {
        PRINTLN_ERROR("Failed to call HAL_SPI_Transmit() to write to 'reg' (Status: %d/%s).", status, hal_status_toString(status));
        HAL_GPIO_WritePin(IMU_CS_GPIO_Port, IMU_CS_Pin, GPIO_PIN_SET); // Deselect IMU since error.
        return -1;
    }

    /* Deselect the IMU by setting its CS pin HIGH. */
    HAL_GPIO_WritePin(IMU_CS_GPIO_Port, IMU_CS_Pin, GPIO_PIN_SET);

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
    printf("before sht30_init()\n");
    int status = sht30_init(&temperature_sensor, _sht30_write, _sht30_read, _sht30_read, temperature_sensor.dev_address);
    if(status != 0) {
        PRINTLN_ERROR("Failed to run sht30_init() (Status: %d).", status);
        return U_ERROR;
    }
    printf("after sht30_init()\n");

    /* Make sure IMU is set up correctly. */
    uint8_t id;
    printf("before lsm6dsv_device_id_get()\n");
    status = lsm6dsv_device_id_get(&imu, &id);
    if(status != 0) {
        PRINTLN_ERROR("Failed to call lsm6dsv_device_id_get() (Status: %d).", status);
        return U_ERROR;
    }
    if(id != LSM6DSV_ID) {
        PRINTLN_ERROR("lsm6dsv_device_id_get() returned an unexpected ID (id=%d, expected=%d). This means that the IMU is not configured correctly.", id, LSM6DSV_ID);
        return U_ERROR;
    }
    printf("after lsm6dsv_device_id_get()\n");

    /* Reset IMU. */
    printf("before lsm6dsv_reset_set()\n");
    status = lsm6dsv_reset_set(&imu, LSM6DSV_GLOBAL_RST);
    if(status != 0) {
        PRINTLN_ERROR("Failed to reset the IMU via lsm6dsv_reset_set() (Status: %d).", status);
        return U_ERROR;
    }
    printf("after lsm6dsv_reset_set()\n");

    printf("before HAL_DELAY()\n");
    //HAL_Delay(30); // This is probably overkill, but the datasheet lists the gyroscope's "Turn-on time" as 30ms, and I can't find anything else that specifies how long resets take.
    tx_thread_sleep(30);
    printf("after HAL_DELAY()\n");

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
    CATCH_ERROR(mutex_get(&peripherals_mutex), U_SUCCESS);
    HAL_StatusTypeDef status = sht30_toggle_heater(&temperature_sensor, enable);
    CATCH_ERROR(mutex_put(&peripherals_mutex), U_SUCCESS);
    if(status != HAL_OK) {
        PRINTLN_ERROR("Failed to toggle SHT30 heater (Status: %d/%s).", status, hal_status_toString(status));
        return U_ERROR;
    }
    return U_SUCCESS;
}

/* Gets the temp sensor's temperature and humidity readings. */
int tempsensor_getTemperatureAndHumidity(float *temperature, float *humidity) {
    CATCH_ERROR(mutex_get(&peripherals_mutex), U_SUCCESS);
    int status = sht30_get_temp_humid(&temperature_sensor);
    CATCH_ERROR(mutex_put(&peripherals_mutex), U_SUCCESS);
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
    CATCH_ERROR(mutex_get(&peripherals_mutex), U_SUCCESS);
    int status = lsm6dsv_acceleration_raw_get(&imu, raw_data);
    CATCH_ERROR(mutex_put(&peripherals_mutex), U_SUCCESS);
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
    CATCH_ERROR(mutex_get(&peripherals_mutex), U_SUCCESS);
    int status = lsm6dsv_angular_rate_raw_get(&imu, raw_data);
    CATCH_ERROR(mutex_put(&peripherals_mutex), U_SUCCESS);
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