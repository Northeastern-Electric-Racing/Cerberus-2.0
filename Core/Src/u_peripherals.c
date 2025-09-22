#include "sht30.h"
#include "lsm6dso.h"
#include "u_peripherals.h"
#include "u_general.h"

/* Driver instances. */
static sht30_t temperature_sensor = { .dev_address = SHT30_I2C_ADDR };
static LSM6DSO_Object_t imu;

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
        .WriteReg = NULL, // u_TODO
        .ReadReg = NULL,  // u_TODO
        .GetTick = HAL_GetTick,
        .Delay = HAL_Delay
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
        DEBUG_PRINTLN("ERROR: Failed to run LSM6DSO_GYRO_SetOutputDataRate_With_Mode() (Status: %d).");
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