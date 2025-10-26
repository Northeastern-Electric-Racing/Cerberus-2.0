#include "sht30.h"
#include "u_tx_debug.h"
#include "main.h"
#include "u_peripherals.h"

/* Driver instances. */
static sht30_t temperature_sensor = { .dev_address = SHT30_I2C_ADDR };
static LSM6DSO_Object_t imu;

/* IMU SPI Chip Select Macros. */
#define _SELECT_IMU   GPIO_PIN_RESET // Setting the CS pin LOW selects the IMU for SPI.
#define _DESELECT_IMU GPIO_PIN_SET   // Setting the CS pin HIGH deselects the IMU for SPI.

/* Wrapper for lsm6dso SPI reading. */
static int32_t _lsm6dso_read(uint16_t device_address, uint16_t register_address, uint8_t *data, uint16_t length) {
    /* For SPI reads, set MSB = 1 for read operation. */
    uint8_t spi_reg = (uint8_t)(register_address | 0x80);
    HAL_StatusTypeDef status;
    
    /* Select the IMU device. */
    HAL_GPIO_WritePin(SPI2_NSS_GPIO_Port, SPI2_NSS_Pin, _SELECT_IMU);
    
    /* Send the register address we're trying to read from. */
    status = HAL_SPI_Transmit(&hspi2, &spi_reg, sizeof(spi_reg), HAL_MAX_DELAY);
    if(status != HAL_OK) {
        HAL_GPIO_WritePin(SPI2_NSS_GPIO_Port, SPI2_NSS_Pin, _DESELECT_IMU);
        DEBUG_PRINTLN("ERROR: Failed to send register address to lsm6dso over SPI 
            (Status: %d/%s).", status, hal_status_toString(status));
        return LSM6DSO_ERROR;
    }
    
    /* Recieve the data. */
    status = HAL_SPI_Receive(&hspi2, data, length, HAL_MAX_DELAY);
    if(status != HAL_OK) {
        DEBUG_PRINTLN("ERROR: Failed to read from the lsm6dso over SPI 
            (Status: %d/%s).", status, hal_status_toString(status));
        return LSM6DSO_ERROR;
    }
    
    /* Deselect the IMU device. */
    HAL_GPIO_WritePin(SPI2_NSS_GPIO_Port, SPI2_NSS_Pin, _DESELECT_IMU);
    
    return LSM6DSO_OK;
}

/* Wrapper for lsm6dso SPI writing. */
static int32_t _lsm6dso_write(uint16_t device_address, uint16_t register_address, uint8_t *data, uint16_t length) {
    /* For SPI writes, clear MSB = 0 for write operation. */
    uint8_t spi_reg = (uint8_t)(register_address & 0x7F);
    HAL_StatusTypeDef status;
    
    /* Select the device (CS low). */
    HAL_GPIO_WritePin(SPI2_NSS_GPIO_Port, SPI2_NSS_Pin, _SELECT_IMU);
    
    /* Send register address. */
    status = HAL_SPI_Transmit(&hspi2, &spi_reg, sizeof(spi_reg), HAL_MAX_DELAY);
    if(status != HAL_OK) {
        HAL_GPIO_WritePin(SPI2_NSS_GPIO_Port, SPI2_NSS_Pin, _DESELECT_IMU);
        DEBUG_PRINTLN("ERROR: Failed to send register address to lsm6dso over SPI (Status: %d/%s).", status, hal_status_toString(status));
        return LSM6DSO_ERROR;
    }
    
    /* Send data. */
    status = HAL_SPI_Transmit(&hspi2, data, length, HAL_MAX_DELAY);
    if(status != HAL_OK) {
        DEBUG_PRINTLN("ERROR: Failed to write to the lsm6dso over SPI (Status: %d/%s).", status, hal_status_toString(status));
        return LSM6DSO_ERROR;
    }
    
    /* Deselect the device (CS high). */
    HAL_GPIO_WritePin(SPI2_NSS_GPIO_Port, SPI2_NSS_Pin, _DESELECT_IMU);
    
    return LSM6DSO_OK;
}

/* Wrapper for sht30 I2C reading. */
static int _sht30_read(uint8_t *data, uint16_t command, uint8_t device_address, uint8_t length) {
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(&hi2c2, (uint16_t)device_address, command, I2C_MEMADD_SIZE_16BIT, data, length, HAL_MAX_DELAY);
    if(status != HAL_OK) {
        DEBUG_PRINTLN("ERROR: Failed to call _sht30_read() (Status: %d/%s, Command: %d).", status, hal_status_toString(status), command);
        return status;
    }
    return status;
}

/* Wrapper for sht30 I2C writing. */
static int _sht30_write(uint8_t *data, uint8_t device_address, uint8_t length) {
    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(&hi2c2, (uint16_t)device_address, data, (uint16_t)length, HAL_MAX_DELAY);
    if(status != HAL_OK) {
        DEBUG_PRINTLN("ERROR: Failed to call _sht30_write() (Status: %d/%s).", status, hal_status_toString(status));
        return status;
    }
    return status;
}

/* Wrapper for HAL_GetTick. */
int32_t _get_tick(void) {
    return (int32_t)HAL_GetTick();
}

/* Wrapper for HAL_Delay. */
void _delay(uint32_t delay) {
    return HAL_Delay(delay);
}

/* Initializes I2C devices. */
int peripherals_init(void) {

    /* Initialize temperature sensor. */
    int status = sht30_init(&temperature_sensor, _sht30_write, _sht30_read, _sht30_read, temperature_sensor.dev_address);
    if(status != 0) {
        DEBUG_PRINTLN("ERROR: Failed to run sht30_init() (Status: %d).", status);
        return U_ERROR;
    }

    /* Register LSM6DSO Bus IO (i.e. how it does read/write). */
    LSM6DSO_IO_t io_config = {
        .BusType = LSM6DSO_SPI_4WIRES_BUS,
        .WriteReg = _lsm6dso_write,
        .ReadReg = _lsm6dso_read,
        .GetTick = _get_tick,
        .Delay = _delay
    };
    status = LSM6DSO_RegisterBusIO(&imu, &io_config);
    if(status != LSM6DSO_OK) {
        DEBUG_PRINTLN("ERROR: Failed to call LSM6DSO_RegisterBusIO (Status: %d).", status);
    }

    /* Initialize IMU. */
    status = LSM6DSO_Init(&imu);
    if(status != LSM6DSO_OK) {
        DEBUG_PRINTLN("ERROR: Failed to run LSM6DS0_Init() (Status: %d).", status);
        return U_ERROR;
    }

    /* Setup IMU Accelerometer - default 104Hz */
	status = LSM6DSO_ACC_SetOutputDataRate_With_Mode(&imu, 833.0f, LSM6DSO_ACC_HIGH_PERFORMANCE_MODE);
    if(status != LSM6DSO_OK) {
        DEBUG_PRINTLN("ERROR: Failed to run LSM6DSO_ACC_SetOutputDataRate_With_Mode (Status: %d).", status);
        return U_ERROR;
    }

    /* Enable Accelerometer. */
    status = LSM6DSO_ACC_Enable(&imu);
    if(status != LSM6DSO_OK) {
        DEBUG_PRINTLN("ERROR: Failed to run LSM6DSO_ACC_Enable() (Status: %d).", status);
        return U_ERROR;
    }
	
    /* Set Accelerometer Filter Mode. */
	status = LSM6DSO_ACC_Set_Filter_Mode(&imu, 0, 3); // 3 = 'LSM6DSO_LP_ODR_DIV_45'
    if(status != LSM6DSO_OK) {
        DEBUG_PRINTLN("ERROR: Failed to run LSM6DSO_ACC_Set_Filter_Mode() (Status: %d).", status);
        return U_ERROR;
    }

	/* Setup IMU Gyroscope */
	status = LSM6DSO_GYRO_SetOutputDataRate_With_Mode(&imu, 104.0f, LSM6DSO_GYRO_HIGH_PERFORMANCE_MODE);
    if(status != LSM6DSO_OK) {
        DEBUG_PRINTLN("ERROR: Failed to run LSM6DSO_GYRO_SetOutputDataRate_With_Mode() (Status: %d).", status);
        return U_ERROR;
    }

    /* Enable IMU Gyroscope. */
	status = LSM6DSO_GYRO_Enable(&imu);
    if(status != LSM6DSO_OK) {
        DEBUG_PRINTLN("ERROR: Failed to run LSM6DSO_GYRO_Enable() (Status: %d).", status);
        return U_ERROR;
    }

    /* Set FIFO mode. */
	status = LSM6DSO_FIFO_Set_Mode(&imu, 0); // 0 = 'LSM6DSO_BYPASS_MODE'
    if(status != LSM6DSO_OK) {
        DEBUG_PRINTLN("ERROR: Failed to run LSM6DSO_FIFO_Set_Mode() (Status: %d).", status);
        return U_ERROR;
    }

    /* Disable Accelerometer Inactivity Detection. */
	status = LSM6DSO_ACC_Disable_Inactivity_Detection(&imu);
    if(status != LSM6DSO_OK) {
        DEBUG_PRINTLN("ERROR: Failed to run LSM6DSO_ACC_Disable_Inactivity_Detection() (Status: %d).", status);
        return U_ERROR;
    }

    DEBUG_PRINTLN("Ran peripherals_init().");
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
    *temperature = temperature_sensor.temp;
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

/* Gets the accelerometer axes (x, y, and z). */
int imu_getAccelerometerData(LSM6DSO_Axes_t *axes) {
    int status = LSM6DSO_ACC_GetAxes(&imu, axes);
    if(status != LSM6DSO_OK) {
        DEBUG_PRINTLN("ERROR: Failed to call LSM6DSO_ACC_GetAxes() (Status: %d).", status);
        return U_ERROR;
    }
    return U_SUCCESS;
}

/* Gets the gyroscope axes (x, y, and z). */
int imu_getGyroscopeData(LSM6DSO_Axes_t *axes) {
    int status = LSM6DSO_GYRO_GetAxes(&imu, axes);
    if(status != LSM6DSO_OK) {
        DEBUG_PRINTLN("ERROR: Failed to call LSM6DSO_GYRO_GetAxes() (Status: %d).", status);
        return U_ERROR;
    }
    return U_SUCCESS;
}