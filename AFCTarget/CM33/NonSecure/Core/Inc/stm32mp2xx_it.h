/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32mp2xx_it.h
  * @brief   This file contains the headers of the interrupt handlers.
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
#ifndef __STM32MP2xx_IT_H
#define __STM32MP2xx_IT_H

#ifdef __cplusplus
extern "C" {
#endif

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
void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SecureFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);
void HPDMA1_Channel0_IRQHandler(void);
void HPDMA1_Channel1_IRQHandler(void);
void HPDMA1_Channel4_IRQHandler(void);
void HPDMA1_Channel5_IRQHandler(void);
void HPDMA1_Channel6_IRQHandler(void);
void HPDMA1_Channel7_IRQHandler(void);
void HPDMA1_Channel8_IRQHandler(void);
void HPDMA1_Channel9_IRQHandler(void);
void HPDMA1_Channel10_IRQHandler(void);
void HPDMA1_Channel11_IRQHandler(void);
void HPDMA1_Channel12_IRQHandler(void);
void HPDMA1_Channel13_IRQHandler(void);
void TIM5_IRQHandler(void);
void UART5_IRQHandler(void);
void TIM6_IRQHandler(void);
void TIM7_IRQHandler(void);
void USART6_IRQHandler(void);
void UART8_IRQHandler(void);
void SPI4_IRQHandler(void);
void TIM12_IRQHandler(void);
void TIM13_IRQHandler(void);
void TIM14_IRQHandler(void);
void RCC_WAKEUP_IRQHandler(void);
/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __STM32MP2xx_IT_H */
