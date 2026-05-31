/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ipcc.c
  * @brief   This file provides code for the configuration
  *          of the IPCC instances.
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
/* Includes ------------------------------------------------------------------*/
#include "ipcc.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

IPCC_HandleTypeDef hipcc1;

/* IPCC1 init function */
void MX_IPCC1_Init(void)
{

  /* USER CODE BEGIN IPCC1_Init 0 */
// // 🚨 ST 标准异构热重启洗涤：直接将句柄的状态复位到 RESET 状态
//   // 这样直接对结构体字段赋值，完美平替了宏定义，且百分之百绕过链接器的 undefined 错误
//   hipcc1.State = HAL_IPCC_STATE_RESET;
  
//   // 🚨 遵循 ST 官方参考手册 RM0511 的外设时钟软复位标准
//   // 强制将 IPCC1 的硬件总线时钟关闭再重新使能，这是 ST 推荐的清除外设物理状态机的正统方法
//   __HAL_RCC_IPCC1_CLK_DISABLE();
//   __asm("nop"); __asm("nop"); __asm("nop");
//   __HAL_RCC_IPCC1_CLK_ENABLE();
  /* USER CODE END IPCC1_Init 0 */

  /* USER CODE BEGIN IPCC1_Init 1 */

  /* USER CODE END IPCC1_Init 1 */
  hipcc1.Instance = IPCC1;
  if (HAL_IPCC_Init(&hipcc1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IPCC1_Init 2 */

  /* USER CODE END IPCC1_Init 2 */

}

void HAL_IPCC_MspInit(IPCC_HandleTypeDef* ipccHandle)
{

  if(ipccHandle->Instance==IPCC1)
  {
  /* USER CODE BEGIN IPCC1_MspInit 0 */

  /* USER CODE END IPCC1_MspInit 0 */

    /* IPCC1 interrupt Init */
    HAL_NVIC_SetPriority(IPCC1_RX_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(IPCC1_RX_IRQn);
    HAL_NVIC_SetPriority(IPCC1_TX_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(IPCC1_TX_IRQn);
  /* USER CODE BEGIN IPCC1_MspInit 1 */

  /* USER CODE END IPCC1_MspInit 1 */
  }
}

void HAL_IPCC_MspDeInit(IPCC_HandleTypeDef* ipccHandle)
{

  if(ipccHandle->Instance==IPCC1)
  {
  /* USER CODE BEGIN IPCC1_MspDeInit 0 */

  /* USER CODE END IPCC1_MspDeInit 0 */

    /* IPCC1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(IPCC1_RX_IRQn);
    HAL_NVIC_DisableIRQ(IPCC1_TX_IRQn);
  /* USER CODE BEGIN IPCC1_MspDeInit 1 */

  /* USER CODE END IPCC1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

