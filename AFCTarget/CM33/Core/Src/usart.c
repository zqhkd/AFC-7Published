/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
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
#include "usart.h"

/* USER CODE BEGIN 0 */
// DMA_HandleTypeDef handle_HPDMA1_Channel1;   //  stm32cubeMX自动生成代码HPDMA1缺少通道1句柄，需手动添加。added on 20260510 by zqhkd
DMA_HandleTypeDef handle_HPDMA3_Channel1;      //  stm32cubeMX自动生成代码HPDMA3也缺少通道1句柄，需手动添加。added on 20260530 by zqhkd
/* USER CODE END 0 */

UART_HandleTypeDef huart4;
UART_HandleTypeDef huart5;
UART_HandleTypeDef huart7;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart3;
UART_HandleTypeDef huart6;
DMA_HandleTypeDef handle_HPDMA3_Channel7;
DMA_HandleTypeDef handle_HPDMA3_Channel6;
DMA_HandleTypeDef handle_HPDMA3_Channel9;
DMA_HandleTypeDef handle_HPDMA3_Channel8;
DMA_HandleTypeDef handle_HPDMA3_Channel13;
DMA_HandleTypeDef handle_HPDMA3_Channel12;
DMA_HandleTypeDef handle_HPDMA3_Channel0;
DMA_HandleTypeDef handle_HPDMA3_Channel5;
DMA_HandleTypeDef handle_HPDMA3_Channel4;
DMA_HandleTypeDef handle_HPDMA3_Channel11;
DMA_HandleTypeDef handle_HPDMA3_Channel10;

/* UART4 init function */
void MX_UART4_Init(void)
{

  /* USER CODE BEGIN UART4_Init 0 */

  /* USER CODE END UART4_Init 0 */

  /* USER CODE BEGIN UART4_Init 1 */

  /* USER CODE END UART4_Init 1 */
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 115200;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  huart4.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart4.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart4.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart4, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart4, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART4_Init 2 */

  /* USER CODE END UART4_Init 2 */

}
/* UART5 init function */
void MX_UART5_Init(void)
{

  /* USER CODE BEGIN UART5_Init 0 */

  /* USER CODE END UART5_Init 0 */

  /* USER CODE BEGIN UART5_Init 1 */

  /* USER CODE END UART5_Init 1 */
  huart5.Instance = UART5;
  huart5.Init.BaudRate = 115200;
  huart5.Init.WordLength = UART_WORDLENGTH_8B;
  huart5.Init.StopBits = UART_STOPBITS_1;
  huart5.Init.Parity = UART_PARITY_NONE;
  huart5.Init.Mode = UART_MODE_TX_RX;
  huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart5.Init.OverSampling = UART_OVERSAMPLING_16;
  huart5.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart5.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart5.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart5) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart5, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart5, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart5) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART5_Init 2 */

  /* USER CODE END UART5_Init 2 */

}
/* UART7 init function */
void MX_UART7_Init(void)
{

  /* USER CODE BEGIN UART7_Init 0 */

  /* USER CODE END UART7_Init 0 */

  /* USER CODE BEGIN UART7_Init 1 */

  /* USER CODE END UART7_Init 1 */
  huart7.Instance = UART7;
  huart7.Init.BaudRate = 115200;
  huart7.Init.WordLength = UART_WORDLENGTH_8B;
  huart7.Init.StopBits = UART_STOPBITS_1;
  huart7.Init.Parity = UART_PARITY_NONE;
  huart7.Init.Mode = UART_MODE_TX_RX;
  huart7.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart7.Init.OverSampling = UART_OVERSAMPLING_16;
  huart7.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart7.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart7.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart7) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart7, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart7, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart7) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART7_Init 2 */

  /* USER CODE END UART7_Init 2 */

}
/* USART1 init function */

void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 38400;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}
/* USART3 init function */

void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 100000;
  huart3.Init.WordLength = UART_WORDLENGTH_9B;
  huart3.Init.StopBits = UART_STOPBITS_2;
  huart3.Init.Parity = UART_PARITY_EVEN;
  huart3.Init.Mode = UART_MODE_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart3, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart3, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}
/* USART6 init function */

void MX_USART6_UART_Init(void)
{

  /* USER CODE BEGIN USART6_Init 0 */

  /* USER CODE END USART6_Init 0 */

  /* USER CODE BEGIN USART6_Init 1 */

  /* USER CODE END USART6_Init 1 */
  huart6.Instance = USART6;
  huart6.Init.BaudRate = 115200;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  huart6.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart6.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart6.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart6) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart6, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart6, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart6) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART6_Init 2 */

  /* USER CODE END USART6_Init 2 */

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==UART4)
  {
  /* USER CODE BEGIN UART4_MspInit 0 */

  /* USER CODE END UART4_MspInit 0 */

    /**UART4 GPIO Configuration
    PB7     ------> UART4_TX
    PB6     ------> UART4_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF3_UART4;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Alternate = GPIO_AF3_UART4;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* UART4 DMA Init */
    /* HPDMA_REQUEST_UART4_TX Init */
    handle_HPDMA3_Channel7.Instance = HPDMA3_Channel7;
    handle_HPDMA3_Channel7.Init.Request = HPDMA_REQUEST_UART4_TX;
    handle_HPDMA3_Channel7.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
    handle_HPDMA3_Channel7.Init.Direction = DMA_MEMORY_TO_PERIPH;
    handle_HPDMA3_Channel7.Init.SrcInc = DMA_SINC_INCREMENTED;
    handle_HPDMA3_Channel7.Init.DestInc = DMA_DINC_FIXED;
    handle_HPDMA3_Channel7.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_BYTE;
    handle_HPDMA3_Channel7.Init.DestDataWidth = DMA_DEST_DATAWIDTH_BYTE;
    handle_HPDMA3_Channel7.Init.Priority = DMA_LOW_PRIORITY_LOW_WEIGHT;
    handle_HPDMA3_Channel7.Init.SrcBurstLength = 1;
    handle_HPDMA3_Channel7.Init.DestBurstLength = 1;
    handle_HPDMA3_Channel7.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0|DMA_DEST_ALLOCATED_PORT1;
    handle_HPDMA3_Channel7.Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
    handle_HPDMA3_Channel7.Init.Mode = DMA_NORMAL;
    if (HAL_DMA_Init(&handle_HPDMA3_Channel7) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle, hdmatx, handle_HPDMA3_Channel7);

    /* HPDMA_REQUEST_UART4_RX Init */
    handle_HPDMA3_Channel6.Instance = HPDMA3_Channel6;
    handle_HPDMA3_Channel6.Init.Request = HPDMA_REQUEST_UART4_RX;
    handle_HPDMA3_Channel6.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
    handle_HPDMA3_Channel6.Init.Direction = DMA_PERIPH_TO_MEMORY;
    handle_HPDMA3_Channel6.Init.SrcInc = DMA_SINC_FIXED;
    handle_HPDMA3_Channel6.Init.DestInc = DMA_DINC_INCREMENTED;
    handle_HPDMA3_Channel6.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_BYTE;
    handle_HPDMA3_Channel6.Init.DestDataWidth = DMA_DEST_DATAWIDTH_BYTE;
    handle_HPDMA3_Channel6.Init.Priority = DMA_LOW_PRIORITY_LOW_WEIGHT;
    handle_HPDMA3_Channel6.Init.SrcBurstLength = 1;
    handle_HPDMA3_Channel6.Init.DestBurstLength = 1;
    handle_HPDMA3_Channel6.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0|DMA_DEST_ALLOCATED_PORT1;
    handle_HPDMA3_Channel6.Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
    handle_HPDMA3_Channel6.Init.Mode = DMA_NORMAL;
    if (HAL_DMA_Init(&handle_HPDMA3_Channel6) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle, hdmarx, handle_HPDMA3_Channel6);

    /* UART4 interrupt Init */
    HAL_NVIC_SetPriority(UART4_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(UART4_IRQn);
  /* USER CODE BEGIN UART4_MspInit 1 */

  /* USER CODE END UART4_MspInit 1 */
  }
  else if(uartHandle->Instance==UART5)
  {
  /* USER CODE BEGIN UART5_MspInit 0 */

  /* USER CODE END UART5_MspInit 0 */

    /**UART5 GPIO Configuration
    PG9     ------> UART5_TX
    PG10     ------> UART5_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF5_UART5;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Alternate = GPIO_AF5_UART5;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    /* UART5 DMA Init */
    /* HPDMA_REQUEST_UART5_TX Init */
    handle_HPDMA3_Channel9.Instance = HPDMA3_Channel9;
    handle_HPDMA3_Channel9.Init.Request = HPDMA_REQUEST_UART5_TX;
    handle_HPDMA3_Channel9.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
    handle_HPDMA3_Channel9.Init.Direction = DMA_MEMORY_TO_PERIPH;
    handle_HPDMA3_Channel9.Init.SrcInc = DMA_SINC_INCREMENTED;
    handle_HPDMA3_Channel9.Init.DestInc = DMA_DINC_FIXED;
    handle_HPDMA3_Channel9.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_BYTE;
    handle_HPDMA3_Channel9.Init.DestDataWidth = DMA_DEST_DATAWIDTH_BYTE;
    handle_HPDMA3_Channel9.Init.Priority = DMA_LOW_PRIORITY_LOW_WEIGHT;
    handle_HPDMA3_Channel9.Init.SrcBurstLength = 1;
    handle_HPDMA3_Channel9.Init.DestBurstLength = 1;
    handle_HPDMA3_Channel9.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0|DMA_DEST_ALLOCATED_PORT1;
    handle_HPDMA3_Channel9.Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
    handle_HPDMA3_Channel9.Init.Mode = DMA_NORMAL;
    if (HAL_DMA_Init(&handle_HPDMA3_Channel9) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle, hdmatx, handle_HPDMA3_Channel9);

    /* HPDMA_REQUEST_UART5_RX Init */
    handle_HPDMA3_Channel8.Instance = HPDMA3_Channel8;
    handle_HPDMA3_Channel8.Init.Request = HPDMA_REQUEST_UART5_RX;
    handle_HPDMA3_Channel8.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
    handle_HPDMA3_Channel8.Init.Direction = DMA_PERIPH_TO_MEMORY;
    handle_HPDMA3_Channel8.Init.SrcInc = DMA_SINC_FIXED;
    handle_HPDMA3_Channel8.Init.DestInc = DMA_DINC_INCREMENTED;
    handle_HPDMA3_Channel8.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_BYTE;
    handle_HPDMA3_Channel8.Init.DestDataWidth = DMA_DEST_DATAWIDTH_BYTE;
    handle_HPDMA3_Channel8.Init.Priority = DMA_LOW_PRIORITY_LOW_WEIGHT;
    handle_HPDMA3_Channel8.Init.SrcBurstLength = 1;
    handle_HPDMA3_Channel8.Init.DestBurstLength = 1;
    handle_HPDMA3_Channel8.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0|DMA_DEST_ALLOCATED_PORT1;
    handle_HPDMA3_Channel8.Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
    handle_HPDMA3_Channel8.Init.Mode = DMA_NORMAL;
    if (HAL_DMA_Init(&handle_HPDMA3_Channel8) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle, hdmarx, handle_HPDMA3_Channel8);

    /* UART5 interrupt Init */
    HAL_NVIC_SetPriority(UART5_IRQn, 4, 0);
    HAL_NVIC_EnableIRQ(UART5_IRQn);
  /* USER CODE BEGIN UART5_MspInit 1 */

  /* USER CODE END UART5_MspInit 1 */
  }
  else if(uartHandle->Instance==UART7)
  {
  /* USER CODE BEGIN UART7_MspInit 0 */

  /* USER CODE END UART7_MspInit 0 */

    /**UART7 GPIO Configuration
    PD0     ------> UART7_RX
    PD3     ------> UART7_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Alternate = GPIO_AF6_UART7;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF6_UART7;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* UART7 DMA Init */
    /* HPDMA_REQUEST_UART7_TX Init */
    handle_HPDMA3_Channel13.Instance = HPDMA3_Channel13;
    handle_HPDMA3_Channel13.Init.Request = HPDMA_REQUEST_UART7_TX;
    handle_HPDMA3_Channel13.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
    handle_HPDMA3_Channel13.Init.Direction = DMA_MEMORY_TO_PERIPH;
    handle_HPDMA3_Channel13.Init.SrcInc = DMA_SINC_INCREMENTED;
    handle_HPDMA3_Channel13.Init.DestInc = DMA_DINC_FIXED;
    handle_HPDMA3_Channel13.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_BYTE;
    handle_HPDMA3_Channel13.Init.DestDataWidth = DMA_DEST_DATAWIDTH_BYTE;
    handle_HPDMA3_Channel13.Init.Priority = DMA_LOW_PRIORITY_LOW_WEIGHT;
    handle_HPDMA3_Channel13.Init.SrcBurstLength = 1;
    handle_HPDMA3_Channel13.Init.DestBurstLength = 1;
    handle_HPDMA3_Channel13.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0|DMA_DEST_ALLOCATED_PORT1;
    handle_HPDMA3_Channel13.Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
    handle_HPDMA3_Channel13.Init.Mode = DMA_NORMAL;
    if (HAL_DMA_Init(&handle_HPDMA3_Channel13) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle, hdmatx, handle_HPDMA3_Channel13);

    /* HPDMA_REQUEST_UART7_RX Init */
    handle_HPDMA3_Channel12.Instance = HPDMA3_Channel12;
    handle_HPDMA3_Channel12.Init.Request = HPDMA_REQUEST_UART7_RX;
    handle_HPDMA3_Channel12.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
    handle_HPDMA3_Channel12.Init.Direction = DMA_PERIPH_TO_MEMORY;
    handle_HPDMA3_Channel12.Init.SrcInc = DMA_SINC_FIXED;
    handle_HPDMA3_Channel12.Init.DestInc = DMA_DINC_INCREMENTED;
    handle_HPDMA3_Channel12.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_BYTE;
    handle_HPDMA3_Channel12.Init.DestDataWidth = DMA_DEST_DATAWIDTH_BYTE;
    handle_HPDMA3_Channel12.Init.Priority = DMA_LOW_PRIORITY_LOW_WEIGHT;
    handle_HPDMA3_Channel12.Init.SrcBurstLength = 1;
    handle_HPDMA3_Channel12.Init.DestBurstLength = 1;
    handle_HPDMA3_Channel12.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT1|DMA_DEST_ALLOCATED_PORT1;
    handle_HPDMA3_Channel12.Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
    handle_HPDMA3_Channel12.Init.Mode = DMA_NORMAL;
    if (HAL_DMA_Init(&handle_HPDMA3_Channel12) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle, hdmarx, handle_HPDMA3_Channel12);

    /* UART7 interrupt Init */
    HAL_NVIC_SetPriority(UART7_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(UART7_IRQn);
  /* USER CODE BEGIN UART7_MspInit 1 */

  /* USER CODE END UART7_MspInit 1 */
  }
  else if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspInit 0 */

  /* USER CODE END USART1_MspInit 0 */

    /**USART1 GPIO Configuration
    PG15     ------> USART1_RX
    PG14     ------> USART1_TX
    */
    GPIO_InitStruct.Pin = SimuComRx_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Alternate = GPIO_AF6_USART1;
    HAL_GPIO_Init(SimuComRx_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = SimuComTx_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF6_USART1;
    HAL_GPIO_Init(SimuComTx_GPIO_Port, &GPIO_InitStruct);

    /* USART1 DMA Init */
    /* HPDMA_REQUEST_USART1_TX Init */
    handle_HPDMA3_Channel1.Instance = HPDMA3_Channel1;
    handle_HPDMA3_Channel1.Init.Request = HPDMA_REQUEST_USART1_TX;
    handle_HPDMA3_Channel1.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
    handle_HPDMA3_Channel1.Init.Direction = DMA_MEMORY_TO_PERIPH;
    handle_HPDMA3_Channel1.Init.SrcInc = DMA_SINC_INCREMENTED;
    handle_HPDMA3_Channel1.Init.DestInc = DMA_DINC_FIXED;
    handle_HPDMA3_Channel1.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_BYTE;
    handle_HPDMA3_Channel1.Init.DestDataWidth = DMA_DEST_DATAWIDTH_BYTE;
    handle_HPDMA3_Channel1.Init.Priority = DMA_LOW_PRIORITY_LOW_WEIGHT;
    handle_HPDMA3_Channel1.Init.SrcBurstLength = 1;
    handle_HPDMA3_Channel1.Init.DestBurstLength = 1;
    handle_HPDMA3_Channel1.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0|DMA_DEST_ALLOCATED_PORT1;
    handle_HPDMA3_Channel1.Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
    handle_HPDMA3_Channel1.Init.Mode = DMA_NORMAL;
    if (HAL_DMA_Init(&handle_HPDMA3_Channel1) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle, hdmatx, handle_HPDMA3_Channel1);

    /* HPDMA_REQUEST_USART1_RX Init */
    handle_HPDMA3_Channel0.Instance = HPDMA3_Channel0;
    handle_HPDMA3_Channel0.Init.Request = HPDMA_REQUEST_USART1_RX;
    handle_HPDMA3_Channel0.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
    handle_HPDMA3_Channel0.Init.Direction = DMA_PERIPH_TO_MEMORY;
    handle_HPDMA3_Channel0.Init.SrcInc = DMA_SINC_FIXED;
    handle_HPDMA3_Channel0.Init.DestInc = DMA_DINC_INCREMENTED;
    handle_HPDMA3_Channel0.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_BYTE;
    handle_HPDMA3_Channel0.Init.DestDataWidth = DMA_DEST_DATAWIDTH_BYTE;
    handle_HPDMA3_Channel0.Init.Priority = DMA_LOW_PRIORITY_LOW_WEIGHT;
    handle_HPDMA3_Channel0.Init.SrcBurstLength = 1;
    handle_HPDMA3_Channel0.Init.DestBurstLength = 1;
    handle_HPDMA3_Channel0.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT1|DMA_DEST_ALLOCATED_PORT1;
    handle_HPDMA3_Channel0.Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
    handle_HPDMA3_Channel0.Init.Mode = DMA_NORMAL;
    if (HAL_DMA_Init(&handle_HPDMA3_Channel0) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle, hdmarx, handle_HPDMA3_Channel0);

    /* USART1 interrupt Init */
    HAL_NVIC_SetPriority(USART1_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspInit 1 */

  /* USER CODE END USART1_MspInit 1 */
  }
  else if(uartHandle->Instance==USART3)
  {
  /* USER CODE BEGIN USART3_MspInit 0 */

  /* USER CODE END USART3_MspInit 0 */

    /**USART3 GPIO Configuration
    PI6     ------> USART3_TX
    PI7     ------> USART3_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF6_USART3;
    HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Alternate = GPIO_AF6_USART3;
    HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

    /* USART3 DMA Init */
    /* HPDMA_REQUEST_USART3_TX Init */
    handle_HPDMA3_Channel5.Instance = HPDMA3_Channel5;
    handle_HPDMA3_Channel5.Init.Request = HPDMA_REQUEST_USART3_TX;
    handle_HPDMA3_Channel5.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
    handle_HPDMA3_Channel5.Init.Direction = DMA_MEMORY_TO_PERIPH;
    handle_HPDMA3_Channel5.Init.SrcInc = DMA_SINC_INCREMENTED;
    handle_HPDMA3_Channel5.Init.DestInc = DMA_DINC_FIXED;
    handle_HPDMA3_Channel5.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_BYTE;
    handle_HPDMA3_Channel5.Init.DestDataWidth = DMA_DEST_DATAWIDTH_BYTE;
    handle_HPDMA3_Channel5.Init.Priority = DMA_LOW_PRIORITY_LOW_WEIGHT;
    handle_HPDMA3_Channel5.Init.SrcBurstLength = 1;
    handle_HPDMA3_Channel5.Init.DestBurstLength = 1;
    handle_HPDMA3_Channel5.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0|DMA_DEST_ALLOCATED_PORT1;
    handle_HPDMA3_Channel5.Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
    handle_HPDMA3_Channel5.Init.Mode = DMA_NORMAL;
    if (HAL_DMA_Init(&handle_HPDMA3_Channel5) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle, hdmatx, handle_HPDMA3_Channel5);

    /* HPDMA_REQUEST_USART3_RX Init */
    handle_HPDMA3_Channel4.Instance = HPDMA3_Channel4;
    handle_HPDMA3_Channel4.Init.Request = HPDMA_REQUEST_USART3_RX;
    handle_HPDMA3_Channel4.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
    handle_HPDMA3_Channel4.Init.Direction = DMA_PERIPH_TO_MEMORY;
    handle_HPDMA3_Channel4.Init.SrcInc = DMA_SINC_FIXED;
    handle_HPDMA3_Channel4.Init.DestInc = DMA_DINC_INCREMENTED;
    handle_HPDMA3_Channel4.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_BYTE;
    handle_HPDMA3_Channel4.Init.DestDataWidth = DMA_DEST_DATAWIDTH_BYTE;
    handle_HPDMA3_Channel4.Init.Priority = DMA_LOW_PRIORITY_LOW_WEIGHT;
    handle_HPDMA3_Channel4.Init.SrcBurstLength = 1;
    handle_HPDMA3_Channel4.Init.DestBurstLength = 1;
    handle_HPDMA3_Channel4.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0|DMA_DEST_ALLOCATED_PORT1;
    handle_HPDMA3_Channel4.Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
    handle_HPDMA3_Channel4.Init.Mode = DMA_NORMAL;
    if (HAL_DMA_Init(&handle_HPDMA3_Channel4) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle, hdmarx, handle_HPDMA3_Channel4);

    /* USART3 interrupt Init */
    HAL_NVIC_SetPriority(USART3_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);
  /* USER CODE BEGIN USART3_MspInit 1 */

  /* USER CODE END USART3_MspInit 1 */
  }
  else if(uartHandle->Instance==USART6)
  {
  /* USER CODE BEGIN USART6_MspInit 0 */

  /* USER CODE END USART6_MspInit 0 */

    /**USART6 GPIO Configuration
    PF13     ------> USART6_TX
    PF14     ------> USART6_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF3_USART6;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Alternate = GPIO_AF3_USART6;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    /* USART6 DMA Init */
    /* HPDMA_REQUEST_USART6_TX Init */
    handle_HPDMA3_Channel11.Instance = HPDMA3_Channel11;
    handle_HPDMA3_Channel11.Init.Request = HPDMA_REQUEST_USART6_TX;
    handle_HPDMA3_Channel11.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
    handle_HPDMA3_Channel11.Init.Direction = DMA_MEMORY_TO_PERIPH;
    handle_HPDMA3_Channel11.Init.SrcInc = DMA_SINC_INCREMENTED;
    handle_HPDMA3_Channel11.Init.DestInc = DMA_DINC_FIXED;
    handle_HPDMA3_Channel11.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_BYTE;
    handle_HPDMA3_Channel11.Init.DestDataWidth = DMA_DEST_DATAWIDTH_BYTE;
    handle_HPDMA3_Channel11.Init.Priority = DMA_LOW_PRIORITY_LOW_WEIGHT;
    handle_HPDMA3_Channel11.Init.SrcBurstLength = 1;
    handle_HPDMA3_Channel11.Init.DestBurstLength = 1;
    handle_HPDMA3_Channel11.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0|DMA_DEST_ALLOCATED_PORT1;
    handle_HPDMA3_Channel11.Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
    handle_HPDMA3_Channel11.Init.Mode = DMA_NORMAL;
    if (HAL_DMA_Init(&handle_HPDMA3_Channel11) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle, hdmatx, handle_HPDMA3_Channel11);

    /* HPDMA_REQUEST_USART6_RX Init */
    handle_HPDMA3_Channel10.Instance = HPDMA3_Channel10;
    handle_HPDMA3_Channel10.Init.Request = HPDMA_REQUEST_USART6_RX;
    handle_HPDMA3_Channel10.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
    handle_HPDMA3_Channel10.Init.Direction = DMA_PERIPH_TO_MEMORY;
    handle_HPDMA3_Channel10.Init.SrcInc = DMA_SINC_FIXED;
    handle_HPDMA3_Channel10.Init.DestInc = DMA_DINC_INCREMENTED;
    handle_HPDMA3_Channel10.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_BYTE;
    handle_HPDMA3_Channel10.Init.DestDataWidth = DMA_DEST_DATAWIDTH_BYTE;
    handle_HPDMA3_Channel10.Init.Priority = DMA_LOW_PRIORITY_LOW_WEIGHT;
    handle_HPDMA3_Channel10.Init.SrcBurstLength = 1;
    handle_HPDMA3_Channel10.Init.DestBurstLength = 1;
    handle_HPDMA3_Channel10.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0|DMA_DEST_ALLOCATED_PORT1;
    handle_HPDMA3_Channel10.Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
    handle_HPDMA3_Channel10.Init.Mode = DMA_NORMAL;
    if (HAL_DMA_Init(&handle_HPDMA3_Channel10) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle, hdmarx, handle_HPDMA3_Channel10);

    /* USART6 interrupt Init */
    HAL_NVIC_SetPriority(USART6_IRQn, 4, 0);
    HAL_NVIC_EnableIRQ(USART6_IRQn);
  /* USER CODE BEGIN USART6_MspInit 1 */

  /* USER CODE END USART6_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==UART4)
  {
  /* USER CODE BEGIN UART4_MspDeInit 0 */

  /* USER CODE END UART4_MspDeInit 0 */

    /**UART4 GPIO Configuration
    PB7     ------> UART4_TX
    PB6     ------> UART4_RX
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_7|GPIO_PIN_6);

    /* UART4 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmatx);
    HAL_DMA_DeInit(uartHandle->hdmarx);

    /* UART4 interrupt Deinit */
    HAL_NVIC_DisableIRQ(UART4_IRQn);
  /* USER CODE BEGIN UART4_MspDeInit 1 */

  /* USER CODE END UART4_MspDeInit 1 */
  }
  else if(uartHandle->Instance==UART5)
  {
  /* USER CODE BEGIN UART5_MspDeInit 0 */

  /* USER CODE END UART5_MspDeInit 0 */

    /**UART5 GPIO Configuration
    PG9     ------> UART5_TX
    PG10     ------> UART5_RX
    */
    HAL_GPIO_DeInit(GPIOG, GPIO_PIN_9|GPIO_PIN_10);

    /* UART5 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmatx);
    HAL_DMA_DeInit(uartHandle->hdmarx);

    /* UART5 interrupt Deinit */
    HAL_NVIC_DisableIRQ(UART5_IRQn);
  /* USER CODE BEGIN UART5_MspDeInit 1 */

  /* USER CODE END UART5_MspDeInit 1 */
  }
  else if(uartHandle->Instance==UART7)
  {
  /* USER CODE BEGIN UART7_MspDeInit 0 */

  /* USER CODE END UART7_MspDeInit 0 */

    /**UART7 GPIO Configuration
    PD0     ------> UART7_RX
    PD3     ------> UART7_TX
    */
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_0|GPIO_PIN_3);

    /* UART7 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmatx);
    HAL_DMA_DeInit(uartHandle->hdmarx);

    /* UART7 interrupt Deinit */
    HAL_NVIC_DisableIRQ(UART7_IRQn);
  /* USER CODE BEGIN UART7_MspDeInit 1 */

  /* USER CODE END UART7_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */

    /**USART1 GPIO Configuration
    PG15     ------> USART1_RX
    PG14     ------> USART1_TX
    */
    HAL_GPIO_DeInit(GPIOG, SimuComRx_Pin|SimuComTx_Pin);

    /* USART1 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmatx);
    HAL_DMA_DeInit(uartHandle->hdmarx);

    /* USART1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART3)
  {
  /* USER CODE BEGIN USART3_MspDeInit 0 */

  /* USER CODE END USART3_MspDeInit 0 */

    /**USART3 GPIO Configuration
    PI6     ------> USART3_TX
    PI7     ------> USART3_RX
    */
    HAL_GPIO_DeInit(GPIOI, GPIO_PIN_6|GPIO_PIN_7);

    /* USART3 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmatx);
    HAL_DMA_DeInit(uartHandle->hdmarx);

    /* USART3 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART3_IRQn);
  /* USER CODE BEGIN USART3_MspDeInit 1 */

  /* USER CODE END USART3_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART6)
  {
  /* USER CODE BEGIN USART6_MspDeInit 0 */

  /* USER CODE END USART6_MspDeInit 0 */

    /**USART6 GPIO Configuration
    PF13     ------> USART6_TX
    PF14     ------> USART6_RX
    */
    HAL_GPIO_DeInit(GPIOF, GPIO_PIN_13|GPIO_PIN_14);

    /* USART6 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmatx);
    HAL_DMA_DeInit(uartHandle->hdmarx);

    /* USART6 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART6_IRQn);
  /* USER CODE BEGIN USART6_MspDeInit 1 */

  /* USER CODE END USART6_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

