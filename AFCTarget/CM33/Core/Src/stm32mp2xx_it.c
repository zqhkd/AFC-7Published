/* USER CODE BEGIN Header */
/*******************************************************************************
 * (C) COPYRIGHT 2026 ACE Tech Co.
 * 作    者 ： 曾庆华
 * 文 件 名 ： stm32mp2xx_it.c
 * 版    本 ： V7.06.260603
 * 描    述 ： 硬件中断服务向量表，实现 IPCC 中断入口处对外部时标的逆向整流与空间对标压栈
 *******************************************************************************/
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32mp2xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
extern UART_HandleTypeDef huart5;
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern IPCC_HandleTypeDef hipcc1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim12;
extern TIM_HandleTypeDef htim13;
extern TIM_HandleTypeDef htim14;
extern DMA_HandleTypeDef handle_HPDMA3_Channel7;
extern DMA_HandleTypeDef handle_HPDMA3_Channel6;
extern DMA_HandleTypeDef handle_HPDMA3_Channel9;
extern DMA_HandleTypeDef handle_HPDMA3_Channel8;
extern DMA_HandleTypeDef handle_HPDMA3_Channel13;
extern DMA_HandleTypeDef handle_HPDMA3_Channel12;
extern DMA_HandleTypeDef handle_HPDMA3_Channel1;
extern DMA_HandleTypeDef handle_HPDMA3_Channel0;
extern DMA_HandleTypeDef handle_HPDMA3_Channel5;
extern DMA_HandleTypeDef handle_HPDMA3_Channel4;
extern DMA_HandleTypeDef handle_HPDMA3_Channel11;
extern DMA_HandleTypeDef handle_HPDMA3_Channel10;
extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart5;
extern UART_HandleTypeDef huart7;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart6;
extern TIM_HandleTypeDef htim6;

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
   while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Secure Fault Interrupt.
  */
void SecureFault_Handler(void)
{
  /* USER CODE BEGIN SecureFault_IRQn 0 */

  /* USER CODE END SecureFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_SecureFault_IRQn 0 */
    /* USER CODE END W1_SecureFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/******************************************************************************/
/* STM32MP2xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32mp2xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles HPDMA3 Channel 0 global interrupt.
  */
void HPDMA3_Channel0_IRQHandler(void)
{
  /* USER CODE BEGIN HPDMA3_Channel0_IRQn 0 */

  /* USER CODE END HPDMA3_Channel0_IRQn 0 */
  HAL_DMA_IRQHandler(&handle_HPDMA3_Channel0);
  /* USER CODE BEGIN HPDMA3_Channel0_IRQn 1 */

  /* USER CODE END HPDMA3_Channel0_IRQn 1 */
}

/**
  * @brief This function handles HPDMA3 Channel 1 global interrupt.
  */
void HPDMA3_Channel1_IRQHandler(void)
{
  /* USER CODE BEGIN HPDMA3_Channel1_IRQn 0 */

  /* USER CODE END HPDMA3_Channel1_IRQn 0 */
  HAL_DMA_IRQHandler(&handle_HPDMA3_Channel1);
  /* USER CODE BEGIN HPDMA3_Channel1_IRQn 1 */

  /* USER CODE END HPDMA3_Channel1_IRQn 1 */
}

/**
  * @brief This function handles HPDMA3 Channel 4 global interrupt.
  */
void HPDMA3_Channel4_IRQHandler(void)
{
  /* USER CODE BEGIN HPDMA3_Channel4_IRQn 0 */

  /* USER CODE END HPDMA3_Channel4_IRQn 0 */
  HAL_DMA_IRQHandler(&handle_HPDMA3_Channel4);
  /* USER CODE BEGIN HPDMA3_Channel4_IRQn 1 */

  /* USER CODE END HPDMA3_Channel4_IRQn 1 */
}

/**
  * @brief This function handles HPDMA3 Channel 5 global interrupt.
  */
void HPDMA3_Channel5_IRQHandler(void)
{
  /* USER CODE BEGIN HPDMA3_Channel5_IRQn 0 */

  /* USER CODE END HPDMA3_Channel5_IRQn 0 */
  HAL_DMA_IRQHandler(&handle_HPDMA3_Channel5);
  /* USER CODE BEGIN HPDMA3_Channel5_IRQn 1 */

  /* USER CODE END HPDMA3_Channel5_IRQn 1 */
}

/**
  * @brief This function handles HPDMA3 Channel 6 global interrupt.
  */
void HPDMA3_Channel6_IRQHandler(void)
{
  /* USER CODE BEGIN HPDMA3_Channel6_IRQn 0 */

  /* USER CODE END HPDMA3_Channel6_IRQn 0 */
  HAL_DMA_IRQHandler(&handle_HPDMA3_Channel6);
  /* USER CODE BEGIN HPDMA3_Channel6_IRQn 1 */

  /* USER CODE END HPDMA3_Channel6_IRQn 1 */
}

/**
  * @brief This function handles HPDMA3 Channel 7 global interrupt.
  */
void HPDMA3_Channel7_IRQHandler(void)
{
  /* USER CODE BEGIN HPDMA3_Channel7_IRQn 0 */

  /* USER CODE END HPDMA3_Channel7_IRQn 0 */
  HAL_DMA_IRQHandler(&handle_HPDMA3_Channel7);
  /* USER CODE BEGIN HPDMA3_Channel7_IRQn 1 */

  /* USER CODE END HPDMA3_Channel7_IRQn 1 */
}

/**
  * @brief This function handles HPDMA3 Channel 8 global interrupt.
  */
void HPDMA3_Channel8_IRQHandler(void)
{
  /* USER CODE BEGIN HPDMA3_Channel8_IRQn 0 */

  /* USER CODE END HPDMA3_Channel8_IRQn 0 */
  HAL_DMA_IRQHandler(&handle_HPDMA3_Channel8);
  /* USER CODE BEGIN HPDMA3_Channel8_IRQn 1 */

  /* USER CODE END HPDMA3_Channel8_IRQn 1 */
}

/**
  * @brief This function handles HPDMA3 Channel 9 global interrupt.
  */
void HPDMA3_Channel9_IRQHandler(void)
{
  /* USER CODE BEGIN HPDMA3_Channel9_IRQn 0 */

  /* USER CODE END HPDMA3_Channel9_IRQn 0 */
  HAL_DMA_IRQHandler(&handle_HPDMA3_Channel9);
  /* USER CODE BEGIN HPDMA3_Channel9_IRQn 1 */

  /* USER CODE END HPDMA3_Channel9_IRQn 1 */
}

/**
  * @brief This function handles HPDMA3 Channel 10 global interrupt.
  */
void HPDMA3_Channel10_IRQHandler(void)
{
  /* USER CODE BEGIN HPDMA3_Channel10_IRQn 0 */

  /* USER CODE END HPDMA3_Channel10_IRQn 0 */
  HAL_DMA_IRQHandler(&handle_HPDMA3_Channel10);
  /* USER CODE BEGIN HPDMA3_Channel10_IRQn 1 */

  /* USER CODE END HPDMA3_Channel10_IRQn 1 */
}

/**
  * @brief This function handles HPDMA3 Channel 11 global interrupt.
  */
void HPDMA3_Channel11_IRQHandler(void)
{
  /* USER CODE BEGIN HPDMA3_Channel11_IRQn 0 */

  /* USER CODE END HPDMA3_Channel11_IRQn 0 */
  HAL_DMA_IRQHandler(&handle_HPDMA3_Channel11);
  /* USER CODE BEGIN HPDMA3_Channel11_IRQn 1 */

  /* USER CODE END HPDMA3_Channel11_IRQn 1 */
}

/**
  * @brief This function handles HPDMA3 Channel 12 global interrupt.
  */
void HPDMA3_Channel12_IRQHandler(void)
{
  /* USER CODE BEGIN HPDMA3_Channel12_IRQn 0 */

  /* USER CODE END HPDMA3_Channel12_IRQn 0 */
  HAL_DMA_IRQHandler(&handle_HPDMA3_Channel12);
  /* USER CODE BEGIN HPDMA3_Channel12_IRQn 1 */

  /* USER CODE END HPDMA3_Channel12_IRQn 1 */
}

/**
  * @brief This function handles HPDMA3 Channel 13 global interrupt.
  */
void HPDMA3_Channel13_IRQHandler(void)
{
  /* USER CODE BEGIN HPDMA3_Channel13_IRQn 0 */

  /* USER CODE END HPDMA3_Channel13_IRQn 0 */
  HAL_DMA_IRQHandler(&handle_HPDMA3_Channel13);
  /* USER CODE BEGIN HPDMA3_Channel13_IRQn 1 */

  /* USER CODE END HPDMA3_Channel13_IRQn 1 */
}

/**
  * @brief This function handles TIM2 global interrupt.
  */
void TIM2_IRQHandler(void)
{
  /* USER CODE BEGIN TIM2_IRQn 0 */

  /* USER CODE END TIM2_IRQn 0 */
  HAL_TIM_IRQHandler(&htim2);
  /* USER CODE BEGIN TIM2_IRQn 1 */

  /* USER CODE END TIM2_IRQn 1 */
}

/**
  * @brief This function handles TIM4 global interrupt.
  */
void TIM4_IRQHandler(void)
{
  /* USER CODE BEGIN TIM4_IRQn 0 */

  /* USER CODE END TIM4_IRQn 0 */
  HAL_TIM_IRQHandler(&htim4);
  /* USER CODE BEGIN TIM4_IRQn 1 */

  /* USER CODE END TIM4_IRQn 1 */
}

/**
  * @brief This function handles USART1 global interrupt.
  */
void USART1_IRQHandler(void)
{
  /* USER CODE BEGIN USART1_IRQn 0 */

  /* USER CODE END USART1_IRQn 0 */
  HAL_UART_IRQHandler(&huart1);
  /* USER CODE BEGIN USART1_IRQn 1 */

  /* USER CODE END USART1_IRQn 1 */
}

/**
  * @brief This function handles USART3 global interrupt.
  */
void USART3_IRQHandler(void)
{
  /* USER CODE BEGIN USART3_IRQn 0 */

  /* USER CODE END USART3_IRQn 0 */
  HAL_UART_IRQHandler(&huart3);
  /* USER CODE BEGIN USART3_IRQn 1 */

  /* USER CODE END USART3_IRQn 1 */
}

/**
  * @brief This function handles TIM5 global interrupt.
  */
void TIM5_IRQHandler(void)
{
  /* USER CODE BEGIN TIM5_IRQn 0 */

  /* USER CODE END TIM5_IRQn 0 */
  HAL_TIM_IRQHandler(&htim5);
  /* USER CODE BEGIN TIM5_IRQn 1 */

  /* USER CODE END TIM5_IRQn 1 */
}

/**
  * @brief This function handles UART4 global interrupt.
  */
void UART4_IRQHandler(void)
{
  /* USER CODE BEGIN UART4_IRQn 0 */

  /* USER CODE END UART4_IRQn 0 */
  HAL_UART_IRQHandler(&huart4);
  /* USER CODE BEGIN UART4_IRQn 1 */

  /* USER CODE END UART4_IRQn 1 */
}

/**
  * @brief This function handles UART5 global interrupt.
  */
void UART5_IRQHandler(void)
{
  /* USER CODE BEGIN UART5_IRQn 0 */
  /* 1. 高速通道：检查是否是接收中断 (RXNE) */
  // if (__HAL_UART_GET_FLAG(&huart5, UART_FLAG_RXNE) != RESET)
  // {
  //     /* 🚨 直接读取底层数据寄存器 (RDR)！读取动作会自动清除 RXNE 标志位 🚨 */
  //     /* 全程没有加锁、没有状态判断，极速完成！ */
  //     uint8_t rx_data = (uint8_t)(huart5.Instance->RDR & 0x00FF);
      
  //     if (g_main_rx_idx < 255) {
  //         g_main_rx_buf[g_main_rx_idx++] = rx_data;
  //     }
  //     return; /* 直接返回，不用再往下走臃肿的 HAL_UART_IRQHandler */
  // }

  // /* 2. 防护通道：如果是溢出错误 (ORE) 导致的中断，强力清除 */
  // if (__HAL_UART_GET_FLAG(&huart5, UART_FLAG_ORE) != RESET)
  // {
  //     __HAL_UART_CLEAR_FLAG(&huart5, UART_CLEAR_OREF);
  //     /* 虚拟读取一次数据，帮助清除错误状态 */
  //     volatile uint32_t tmpreg = huart5.Instance->RDR;
  //     (void)tmpreg;
  //     return;
  // }

  /* USER CODE END UART5_IRQn 0 */
  HAL_UART_IRQHandler(&huart5);
  /* USER CODE BEGIN UART5_IRQn 1 */

  /* USER CODE END UART5_IRQn 1 */
}

/**
  * @brief This function handles TIM6 global interrupt.
  */
void TIM6_IRQHandler(void)
{
  /* USER CODE BEGIN TIM6_IRQn 0 */

  /* USER CODE END TIM6_IRQn 0 */
  if (htim6.Instance != NULL) {
    HAL_TIM_IRQHandler(&htim6);
  }
  else
  {
  /* Disable theTIM6global Interrupt */
    HAL_NVIC_DisableIRQ(TIM6_IRQn);
  }
  /* USER CODE BEGIN TIM6_IRQn 1 */

  /* USER CODE END TIM6_IRQn 1 */
}

/**
  * @brief This function handles USART6 global interrupt.
  */
void USART6_IRQHandler(void)
{
  /* USER CODE BEGIN USART6_IRQn 0 */

  /* USER CODE END USART6_IRQn 0 */
  HAL_UART_IRQHandler(&huart6);
  /* USER CODE BEGIN USART6_IRQn 1 */

  /* USER CODE END USART6_IRQn 1 */
}

/**
  * @brief This function handles UART7 global interrupt.
  */
void UART7_IRQHandler(void)
{
  /* USER CODE BEGIN UART7_IRQn 0 */

  /* USER CODE END UART7_IRQn 0 */
  HAL_UART_IRQHandler(&huart7);
  /* USER CODE BEGIN UART7_IRQn 1 */

  /* USER CODE END UART7_IRQn 1 */
}

/**
  * @brief This function handles Mailbox 1 RX Occupied interrupt.
  */
void IPCC1_RX_IRQHandler(void)
{
  /* USER CODE BEGIN IPCC1_RX_IRQn 0 */

  /* USER CODE END IPCC1_RX_IRQn 0 */
  HAL_IPCC_RX_IRQHandler(&hipcc1);
  /* USER CODE BEGIN IPCC1_RX_IRQn 1 */

  /* USER CODE END IPCC1_RX_IRQn 1 */
}

/**
  * @brief This function handles Mailbox 1 TX Free interrupt.
  */
void IPCC1_TX_IRQHandler(void)
{
  /* USER CODE BEGIN IPCC1_TX_IRQn 0 */

  /* USER CODE END IPCC1_TX_IRQn 0 */
  HAL_IPCC_TX_IRQHandler(&hipcc1);
  /* USER CODE BEGIN IPCC1_TX_IRQn 1 */

  /* USER CODE END IPCC1_TX_IRQn 1 */
}

/**
  * @brief This function handles TIM12 global interrupt.
  */
void TIM12_IRQHandler(void)
{
  /* USER CODE BEGIN TIM12_IRQn 0 */

  /* USER CODE END TIM12_IRQn 0 */
  HAL_TIM_IRQHandler(&htim12);
  /* USER CODE BEGIN TIM12_IRQn 1 */

  /* USER CODE END TIM12_IRQn 1 */
}

/**
  * @brief This function handles TIM13 global interrupt.
  */
void TIM13_IRQHandler(void)
{
  /* USER CODE BEGIN TIM13_IRQn 0 */

  /* USER CODE END TIM13_IRQn 0 */
  HAL_TIM_IRQHandler(&htim13);
  /* USER CODE BEGIN TIM13_IRQn 1 */

  /* USER CODE END TIM13_IRQn 1 */
}

/**
  * @brief This function handles TIM14 global interrupt.
  */
void TIM14_IRQHandler(void)
{
  /* USER CODE BEGIN TIM14_IRQn 0 */

  /* USER CODE END TIM14_IRQn 0 */
  HAL_TIM_IRQHandler(&htim14);
  /* USER CODE BEGIN TIM14_IRQn 1 */

  /* USER CODE END TIM14_IRQn 1 */
}

/**
  * @brief This function handles RCC wake-up interrupt.
  */
void RCC_WAKEUP_IRQHandler(void)
{
  /* USER CODE BEGIN RCC_WAKEUP_IRQn 0 */

  /* USER CODE END RCC_WAKEUP_IRQn 0 */
  HAL_RCC_WAKEUP_IRQHandler();
  /* USER CODE BEGIN RCC_WAKEUP_IRQn 1 */

  /* USER CODE END RCC_WAKEUP_IRQn 1 */
}

/* USER CODE BEGIN 1 */
void HAL_IPCC_RxCallback(IPCC_HandleTypeDef *hipcc, uint32_t ChannelIndex, IPCC_CHANNELDirTypeDef ChannelDir)
{
    (void)hipcc;
    (void)ChannelDir;

    /* 物理通道 0：仿真及外部主控指令总线核心通道 */
    if (ChannelIndex == 0) 
    {
        uint64_t u64SimA35TimeUs = 0;
        TSimSens tTmpSens;
        
        /* A. 空间对标：原汁原味地从共享中继区提取外部仿真机下发的传感要素数据 */
        vReadSimDataPacket(&tTmpSens, &u64SimA35TimeUs);
        
        /* B. 🚨 时间对标（时轴逆向投影）：利用固定的 T0 快照线消除 ROS2 的搬运时间抖动 */
        if (p_T0Snapshot->bIsT0Latched == 1)
        {
            // 相对仿真机零点的净流逝微秒增量
            uint64_t u64DeltaRunUs = u64SimA35TimeUs - p_T0Snapshot->t0_A35_us;
            // 刚性投影至本地控制轴：u64AlignedM33TimeUs = t0_M33 + Delta_t
            tTmpSens.u64TimeStampUs = p_T0Snapshot->t0_M33_us + u64DeltaRunUs; 
        }
        else
        {
            tTmpSens.u64TimeStampUs = getFcsM33TimeUs(); // 时轴未合闸前的安全防御
        }

        /* C. 分流多率无锁压栈：若处于仿真模式，直接改写 M33 普通内存缓冲区实体指针，实现孪生重叠 */
        if (bIsHilSimulationMode())
        {
            TIMUFrame sIMUFrame;
            sIMUFrame.fGyroX = tTmpSens.gyro[0];
            sIMUFrame.fGyroY = tTmpSens.gyro[1];
            sIMUFrame.fGyroZ = tTmpSens.gyro[2];
            sIMUFrame.fAccX  = tTmpSens.acc[0];
            sIMUFrame.fAccY  = tTmpSens.acc[1];
            sIMUFrame.fAccZ  = tTmpSens.acc[2];
            sIMUFrame.u64TimeStampUs = tTmpSens.u64TimeStampUs; // 已经纯净化对标后的硬微秒戳
            
            /* 刚性推入 M33 本地普通内存区专属 IMU 环形缓冲区，满帧滚动循环覆盖 */
            g_sIMURingBuffer[g_u8IMUWriteIdx] = sIMUFrame;
            g_u8IMUWriteIdx = (g_u8IMUWriteIdx + 1) % 32; // 矩阵规划深度为 32 帧
        }
    }
}
/* USER CODE END 1 */
