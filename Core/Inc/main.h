/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h5xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
extern IWDG_HandleTypeDef hiwdg;
extern ADC_HandleTypeDef  hadc1;
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define CKPT_GPIO_Pin GPIO_PIN_2
#define CKPT_GPIO_GPIO_Port GPIOE
#define INERTIA_SW_GPIO_Pin GPIO_PIN_3
#define INERTIA_SW_GPIO_GPIO_Port GPIOE
#define TSMS_GPIO_Pin GPIO_PIN_4
#define TSMS_GPIO_GPIO_Port GPIOE
#define IMU_INT1_Pin GPIO_PIN_5
#define IMU_INT1_GPIO_Port GPIOE
#define IMU_INT2_Pin GPIO_PIN_6
#define IMU_INT2_GPIO_Port GPIOE
#define PC14_OSC32_IN_Pin GPIO_PIN_14
#define PC14_OSC32_IN_GPIO_Port GPIOC
#define PC15_OSC32_OUT_Pin GPIO_PIN_15
#define PC15_OSC32_OUT_GPIO_Port GPIOC
#define EF_BATTBOX_EN_Pin GPIO_PIN_2
#define EF_BATTBOX_EN_GPIO_Port GPIOF
#define EF_BATTBOX_ER_Pin GPIO_PIN_3
#define EF_BATTBOX_ER_GPIO_Port GPIOF
#define EF_MC_EN_Pin GPIO_PIN_4
#define EF_MC_EN_GPIO_Port GPIOF
#define EF_MC_ER_Pin GPIO_PIN_5
#define EF_MC_ER_GPIO_Port GPIOF
#define BMS_GPIO_Pin GPIO_PIN_6
#define BMS_GPIO_GPIO_Port GPIOF
#define BOTS_GPIO_Pin GPIO_PIN_7
#define BOTS_GPIO_GPIO_Port GPIOF
#define SPARE_GPIO_Pin GPIO_PIN_8
#define SPARE_GPIO_GPIO_Port GPIOF
#define BSPD_GPIO_Pin GPIO_PIN_9
#define BSPD_GPIO_GPIO_Port GPIOF
#define HV_C_GPIO_Pin GPIO_PIN_10
#define HV_C_GPIO_GPIO_Port GPIOF
#define PH0_OSC_IN_Pin GPIO_PIN_0
#define PH0_OSC_IN_GPIO_Port GPIOH
#define PH1_OSC_OUT_Pin GPIO_PIN_1
#define PH1_OSC_OUT_GPIO_Port GPIOH
#define ADC12_INP10_Pin GPIO_PIN_0
#define ADC12_INP10_GPIO_Port GPIOC
#define ADC12_INP12_Pin GPIO_PIN_2
#define ADC12_INP12_GPIO_Port GPIOC
#define ADC12_INP13_Pin GPIO_PIN_3
#define ADC12_INP13_GPIO_Port GPIOC
#define ADC12_INP0_Pin GPIO_PIN_0
#define ADC12_INP0_GPIO_Port GPIOA
#define ETH_RMII_REF_CLK_Pin GPIO_PIN_1
#define ETH_RMII_REF_CLK_GPIO_Port GPIOA
#define ADC12_INP15_Pin GPIO_PIN_3
#define ADC12_INP15_GPIO_Port GPIOA
#define ADC12_INP18_Pin GPIO_PIN_4
#define ADC12_INP18_GPIO_Port GPIOA
#define ADC12_INP19_Pin GPIO_PIN_5
#define ADC12_INP19_GPIO_Port GPIOA
#define ADC12_INP3_Pin GPIO_PIN_6
#define ADC12_INP3_GPIO_Port GPIOA
#define ETH_RMII_RXD0_Pin GPIO_PIN_4
#define ETH_RMII_RXD0_GPIO_Port GPIOC
#define ETH_RMI_RXD1_Pin GPIO_PIN_5
#define ETH_RMI_RXD1_GPIO_Port GPIOC
#define ADC12_INP9_Pin GPIO_PIN_0
#define ADC12_INP9_GPIO_Port GPIOB
#define ADC12_INP5_Pin GPIO_PIN_1
#define ADC12_INP5_GPIO_Port GPIOB
#define HVD_GPIO_Pin GPIO_PIN_15
#define HVD_GPIO_GPIO_Port GPIOF
#define PHY_IRQ_Pin GPIO_PIN_7
#define PHY_IRQ_GPIO_Port GPIOE
#define PHY_RX_ER_Pin GPIO_PIN_8
#define PHY_RX_ER_GPIO_Port GPIOE
#define PHY_RX_DV_Pin GPIO_PIN_9
#define PHY_RX_DV_GPIO_Port GPIOE
#define PHY_RESET_Pin GPIO_PIN_10
#define PHY_RESET_GPIO_Port GPIOE
#define PHY_GPIO_Pin GPIO_PIN_11
#define PHY_GPIO_GPIO_Port GPIOE
#define EF_SPARE_EN_Pin GPIO_PIN_12
#define EF_SPARE_EN_GPIO_Port GPIOE
#define EF_SPARE_ER_Pin GPIO_PIN_13
#define EF_SPARE_ER_GPIO_Port GPIOE
#define RTDS_GPIO_Pin GPIO_PIN_14
#define RTDS_GPIO_GPIO_Port GPIOE
#define ETH_RMII_TXD0_Pin GPIO_PIN_12
#define ETH_RMII_TXD0_GPIO_Port GPIOB
#define ETH_RMII_TXD1_Pin GPIO_PIN_15
#define ETH_RMII_TXD1_GPIO_Port GPIOB
#define EF_RADFAN_EN_Pin GPIO_PIN_8
#define EF_RADFAN_EN_GPIO_Port GPIOD
#define EF_RADFAN_ER_Pin GPIO_PIN_9
#define EF_RADFAN_ER_GPIO_Port GPIOD
#define EF_FANBATT_EN_Pin GPIO_PIN_10
#define EF_FANBATT_EN_GPIO_Port GPIOD
#define EF_FANBATT_ER_Pin GPIO_PIN_11
#define EF_FANBATT_ER_GPIO_Port GPIOD
#define EF_PUMP1_EN_Pin GPIO_PIN_12
#define EF_PUMP1_EN_GPIO_Port GPIOD
#define EF_PUMP1_ER_Pin GPIO_PIN_13
#define EF_PUMP1_ER_GPIO_Port GPIOD
#define EF_PUMP2_EN_Pin GPIO_PIN_14
#define EF_PUMP2_EN_GPIO_Port GPIOD
#define EF_PUMP2_ER_Pin GPIO_PIN_15
#define EF_PUMP2_ER_GPIO_Port GPIOD
#define LTC4376_FAULT_Pin GPIO_PIN_6
#define LTC4376_FAULT_GPIO_Port GPIOC
#define JTMS_Pin GPIO_PIN_13
#define JTMS_GPIO_Port GPIOA
#define JTCK_Pin GPIO_PIN_14
#define JTCK_GPIO_Port GPIOA
#define JTDI_Pin GPIO_PIN_15
#define JTDI_GPIO_Port GPIOA
#define EF_DASH_EN_Pin GPIO_PIN_0
#define EF_DASH_EN_GPIO_Port GPIOD
#define EF_DASH_ER_Pin GPIO_PIN_1
#define EF_DASH_ER_GPIO_Port GPIOD
#define EF_BREAK_EN_Pin GPIO_PIN_2
#define EF_BREAK_EN_GPIO_Port GPIOD
#define EF_BREAK_ER_Pin GPIO_PIN_3
#define EF_BREAK_ER_GPIO_Port GPIOD
#define EF_SHUTDOWN_EN_Pin GPIO_PIN_4
#define EF_SHUTDOWN_EN_GPIO_Port GPIOD
#define EF_SHUTDOWN_ER_Pin GPIO_PIN_5
#define EF_SHUTDOWN_ER_GPIO_Port GPIOD
#define EF_LV_EN_Pin GPIO_PIN_6
#define EF_LV_EN_GPIO_Port GPIOD
#define EF_LV_ER_Pin GPIO_PIN_7
#define EF_LV_ER_GPIO_Port GPIOD
#define ETH_RMII_TX_EN_Pin GPIO_PIN_11
#define ETH_RMII_TX_EN_GPIO_Port GPIOG
#define WATCHDOG_Pin GPIO_PIN_14
#define WATCHDOG_GPIO_Port GPIOG
#define FAULT_MCU_Pin GPIO_PIN_15
#define FAULT_MCU_GPIO_Port GPIOG
#define JTDO_Pin GPIO_PIN_3
#define JTDO_GPIO_Port GPIOB
#define IMD_GPIO_Pin GPIO_PIN_0
#define IMD_GPIO_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
