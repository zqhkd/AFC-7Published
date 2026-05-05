/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "stm32mp2xx_hal.h"

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
#define ETH2_RSTN_Pin GPIO_PIN_2
#define ETH2_RSTN_GPIO_Port GPIOB
#define GPO6_Pin GPIO_PIN_10
#define GPO6_GPIO_Port GPIOB
#define SPI4_CS2_Pin GPIO_PIN_0
#define SPI4_CS2_GPIO_Port GPIOB
#define GPO3_Pin GPIO_PIN_3
#define GPO3_GPIO_Port GPIOB
#define GPO5_Pin GPIO_PIN_5
#define GPO5_GPIO_Port GPIOD
#define LED_SYSTEM_Pin GPIO_PIN_12
#define LED_SYSTEM_GPIO_Port GPIOD
#define GPO7_Pin GPIO_PIN_1
#define GPO7_GPIO_Port GPIOB
#define PWMIn_PPS_Pin GPIO_PIN_14
#define PWMIn_PPS_GPIO_Port GPIOB
#define GPI1_Pin GPIO_PIN_8
#define GPI1_GPIO_Port GPIOB
#define SPI4_CS1_Pin GPIO_PIN_13
#define SPI4_CS1_GPIO_Port GPIOD
#define SimuComRx_Pin GPIO_PIN_15
#define SimuComRx_GPIO_Port GPIOG
#define GPI0_Pin GPIO_PIN_0
#define GPI0_GPIO_Port GPIOI
#define SimuComTx_Pin GPIO_PIN_14
#define SimuComTx_GPIO_Port GPIOG
#define ETH1_RSTN_Pin GPIO_PIN_4
#define ETH1_RSTN_GPIO_Port GPIOI
#define GPO1_Pin GPIO_PIN_15
#define GPO1_GPIO_Port GPIOF
#define GPO2_Pin GPIO_PIN_5
#define GPO2_GPIO_Port GPIOG
#define LED_USER_Pin GPIO_PIN_5
#define LED_USER_GPIO_Port GPIOZ
#define SPI1_CS_Pin GPIO_PIN_7
#define SPI1_CS_GPIO_Port GPIOG
#define GPI4_Pin GPIO_PIN_8
#define GPI4_GPIO_Port GPIOI
#define GPI6_Pin GPIO_PIN_8
#define GPI6_GPIO_Port GPIOZ
#define GPI7_Pin GPIO_PIN_7
#define GPI7_GPIO_Port GPIOZ
#define GPI2_Pin GPIO_PIN_2
#define GPI2_GPIO_Port GPIOZ
#define GPI3_Pin GPIO_PIN_3
#define GPI3_GPIO_Port GPIOZ
#define ADC1_Pin GPIO_PIN_3
#define ADC1_GPIO_Port GPIOG
#define GPO4_Pin GPIO_PIN_2
#define GPO4_GPIO_Port GPIOH
#define PA1_PMOS_G_CTRL_Pin GPIO_PIN_1
#define PA1_PMOS_G_CTRL_GPIO_Port GPIOA
#define ADC2_Pin GPIO_PIN_4
#define ADC2_GPIO_Port GPIOG
#define PA3_VBUS_IN_3V3_Pin GPIO_PIN_3
#define PA3_VBUS_IN_3V3_GPIO_Port GPIOA
#define GPI5_Pin GPIO_PIN_15
#define GPI5_GPIO_Port GPIOB
#define GPI_GPS_Pin GPIO_PIN_2
#define GPI_GPS_GPIO_Port GPIOA
#define GPO0_Pin GPIO_PIN_6
#define GPO0_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
