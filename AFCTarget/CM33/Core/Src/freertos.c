/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "FreeRTOS.h"
#include "cmsis_os2.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usart.h"
#include "gpio.h"
extern void My_UART_Send(char *str);
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
extern uint8_t aRxBuffer;           // 中断接收专用单字节缓冲区
extern uint8_t g_main_rx_buf[256];  // 主循环处理缓冲区
extern uint16_t g_main_rx_idx;  // 缓冲区当前长度
/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
/* 🚨 修复时钟不准：如果 5s 跑太快，说明 SystemCoreClock 变量偏小 🚨
   * 根据米尔板 Linux 启动后的实际频率，手动校准这个全局变量 */
/* 这里的 tick 就是基于 400MHz 修正后的精准节拍 */
  uint32_t tick_led = osKernelGetTickCount();
  uint32_t tick_hb = osKernelGetTickCount();
  uint32_t seq = 0;
  char hb_str[64];

  for(;;)
  {
    /* 1. 检测中断缓冲区是否有数据（异步回显） */
    if (g_main_rx_idx > 0)
    {
      /* 为防止打印混乱，简单加个短延时确保数据收全 */
      osDelay(10); 
      
      My_UART_Send("Res: ");
      HAL_UART_Transmit(&huart5, g_main_rx_buf, g_main_rx_idx, 100);
      My_UART_Send("\r\n");
      
      g_main_rx_idx = 0; // 清空主缓冲区索引
    }

    /* 2. 严格 2s 心跳 (基于修正后的 400MHz) */
    if ((osKernelGetTickCount() - tick_hb) >= 2000)
    {
      sprintf(hb_str, "[Heartbeat] Seq: %lu @ 400MHz\r\n", seq++);
      My_UART_Send(hb_str);
      tick_hb = osKernelGetTickCount();
    }

    /* 3. 严格 5s 闪灯 */
    if ((osKernelGetTickCount() - tick_led) >= 3000)
    {
      HAL_GPIO_TogglePin(GPIOZ, GPIO_PIN_5); 
      tick_led = osKernelGetTickCount();
    }
    
    osDelay(50); // 降低任务检查频率，减少 CPU 占用，接收靠中断，所以不会丢
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

