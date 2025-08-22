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

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define PE2_IO_Pin GPIO_PIN_2
#define PE2_IO_GPIO_Port GPIOE
#define PE3_IO_Pin GPIO_PIN_3
#define PE3_IO_GPIO_Port GPIOE
#define PE4_IO_Pin GPIO_PIN_4
#define PE4_IO_GPIO_Port GPIOE
#define PE5_IO_Pin GPIO_PIN_5
#define PE5_IO_GPIO_Port GPIOE
#define PE6_IO_Pin GPIO_PIN_6
#define PE6_IO_GPIO_Port GPIOE
#define PC14_OSC32_IN_Pin GPIO_PIN_14
#define PC14_OSC32_IN_GPIO_Port GPIOC
#define PC15_OSC32_OUT_Pin GPIO_PIN_15
#define PC15_OSC32_OUT_GPIO_Port GPIOC
#define PF2_IO_Pin GPIO_PIN_2
#define PF2_IO_GPIO_Port GPIOF
#define PF3_IO_Pin GPIO_PIN_3
#define PF3_IO_GPIO_Port GPIOF
#define PF4_IO_Pin GPIO_PIN_4
#define PF4_IO_GPIO_Port GPIOF
#define PF5_IO_Pin GPIO_PIN_5
#define PF5_IO_GPIO_Port GPIOF
#define PF6_IO_Pin GPIO_PIN_6
#define PF6_IO_GPIO_Port GPIOF
#define PF7_IO_Pin GPIO_PIN_7
#define PF7_IO_GPIO_Port GPIOF
#define PF8_IO_Pin GPIO_PIN_8
#define PF8_IO_GPIO_Port GPIOF
#define PF9_IO_Pin GPIO_PIN_9
#define PF9_IO_GPIO_Port GPIOF
#define PF10_IO_Pin GPIO_PIN_10
#define PF10_IO_GPIO_Port GPIOF
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
#define PF15_IO_Pin GPIO_PIN_15
#define PF15_IO_GPIO_Port GPIOF
#define PHY_IRQ_Pin GPIO_PIN_7
#define PHY_IRQ_GPIO_Port GPIOE
#define PE8_IO_Pin GPIO_PIN_8
#define PE8_IO_GPIO_Port GPIOE
#define PE9_IO_Pin GPIO_PIN_9
#define PE9_IO_GPIO_Port GPIOE
#define PE10_IO_Pin GPIO_PIN_10
#define PE10_IO_GPIO_Port GPIOE
#define PE11_IO_Pin GPIO_PIN_11
#define PE11_IO_GPIO_Port GPIOE
#define PE12_IO_Pin GPIO_PIN_12
#define PE12_IO_GPIO_Port GPIOE
#define PE13_IO_Pin GPIO_PIN_13
#define PE13_IO_GPIO_Port GPIOE
#define RTDS_GPIO_Pin GPIO_PIN_14
#define RTDS_GPIO_GPIO_Port GPIOE
#define ETH_RMII_TXD0_Pin GPIO_PIN_12
#define ETH_RMII_TXD0_GPIO_Port GPIOB
#define ETH_RMII_TXD1_Pin GPIO_PIN_15
#define ETH_RMII_TXD1_GPIO_Port GPIOB
#define PD8_IO_Pin GPIO_PIN_8
#define PD8_IO_GPIO_Port GPIOD
#define PD9_IO_Pin GPIO_PIN_9
#define PD9_IO_GPIO_Port GPIOD
#define PD10_IO_Pin GPIO_PIN_10
#define PD10_IO_GPIO_Port GPIOD
#define PD11_IO_Pin GPIO_PIN_11
#define PD11_IO_GPIO_Port GPIOD
#define PD12_IO_Pin GPIO_PIN_12
#define PD12_IO_GPIO_Port GPIOD
#define PD13_IO_Pin GPIO_PIN_13
#define PD13_IO_GPIO_Port GPIOD
#define PD14_IO_Pin GPIO_PIN_14
#define PD14_IO_GPIO_Port GPIOD
#define PD15_IO_Pin GPIO_PIN_15
#define PD15_IO_GPIO_Port GPIOD
#define JTMS_Pin GPIO_PIN_13
#define JTMS_GPIO_Port GPIOA
#define JTCK_Pin GPIO_PIN_14
#define JTCK_GPIO_Port GPIOA
#define JTDI_Pin GPIO_PIN_15
#define JTDI_GPIO_Port GPIOA
#define PD0_IO_Pin GPIO_PIN_0
#define PD0_IO_GPIO_Port GPIOD
#define PD1_IO_Pin GPIO_PIN_1
#define PD1_IO_GPIO_Port GPIOD
#define PD2_IO_Pin GPIO_PIN_2
#define PD2_IO_GPIO_Port GPIOD
#define PD3_IO_Pin GPIO_PIN_3
#define PD3_IO_GPIO_Port GPIOD
#define PD4_IO_Pin GPIO_PIN_4
#define PD4_IO_GPIO_Port GPIOD
#define PD5_IO_Pin GPIO_PIN_5
#define PD5_IO_GPIO_Port GPIOD
#define PD6_IO_Pin GPIO_PIN_6
#define PD6_IO_GPIO_Port GPIOD
#define PD7_IO_Pin GPIO_PIN_7
#define PD7_IO_GPIO_Port GPIOD
#define ETH_RMII_TX_EN_Pin GPIO_PIN_11
#define ETH_RMII_TX_EN_GPIO_Port GPIOG
#define WATCHDOG_Pin GPIO_PIN_14
#define WATCHDOG_GPIO_Port GPIOG
#define FAULT_MCU_Pin GPIO_PIN_15
#define FAULT_MCU_GPIO_Port GPIOG
#define JTDO_Pin GPIO_PIN_3
#define JTDO_GPIO_Port GPIOB
#define PE0_IO_Pin GPIO_PIN_0
#define PE0_IO_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
