/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "copro_sync.h"
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "adc.h"
#include "fdcan.h"
#include "hpdma.h"
#include "i2c.h"
#include "ipcc.h"
#include "openamp.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32mp2xx.h"
// #include "myir_board_defs.h"
// #include "openamp.h"

#define __GLOBAL_VAR_FIRST_USE__
#include "AFCGlobalVar.h"
#undef  __GLOBAL_VAR_FIRST_USE__

#include "AFCTask.h"

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

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */
// AFC-7飞控板的时钟合闸函数实现
void AFC7_Hardware_Clock_Gating_On(void);
/* 定义最可靠的串口发送函数，避免 printf 卡死 */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
#ifdef __AFC7_DEBUG_MODE__
    __asm volatile ("bkpt #0");  // 在Linux6.6.48中，必须使用该语句让M33核处于挂起状态，才能正常调试。
#endif
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  AFC7_Hardware_Clock_Gating_On();
  /* USER CODE END Init */

    /* Configure the system clock */
  if(IS_DEVELOPER_BOOT_MODE())
  {
    SystemClock_Config();
  }
  else
  {
   SystemCoreClockUpdate();
  }

  if(!IS_DEVELOPER_BOOT_MODE())
  {
    /* IPCC initialisation */
    MX_IPCC1_Init();
    /*Corpo Sync Initialization*/
    CoproSync_Init();
  }

  /* OpenAmp initialisation ---------------------------------*/
  // MX_OPENAMP_Init(RPMSG_REMOTE, NULL);

  /* USER CODE BEGIN SysInit */
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_HPDMA3_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_FDCAN1_Init();
  MX_FDCAN2_Init();
  MX_I2C1_Init();
  MX_I2C3_Init();
  MX_SPI1_Init();
  MX_SPI4_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM4_Init();
  MX_TIM5_Init();
  MX_TIM8_Init();
  MX_TIM12_Init();
  MX_TIM13_Init();
  MX_TIM14_Init();
  MX_UART4_Init();
  MX_UART5_Init();
  MX_USART1_UART_Init();
  MX_USART3_UART_Init();
  MX_USART6_UART_Init();
  MX_UART7_Init();
  /* USER CODE BEGIN 2 */

  // 🚨 执行算法自检状态机加载、离散控制模型初始状态初值加载
  AFCTaskInit();
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();  /* Call init function for freertos objects (in cmsis_os2.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)  // 永远不会执行到这里
  {
    /* USER CODE END WHILE */
    MainProc();
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
}

/* USER CODE BEGIN 4 */
// AFC-7飞控板的时钟合闸函数实现
/**
  * @brief  飞控板级支持包：硬件时钟总线强行常开配置
  * @note   针对 AFC-7 飞控板多核异构环境下的 M33 核心量身定制。
  * 由于 CubeMX 触发了异构保护机制未自动生成时钟门控代码，
  * 本函数必须在 main() 函数调用 HAL_Init() 以及任何外设 MX_XXX_Init() 之前执行！
  * @retval None
  */
void AFC7_Hardware_Clock_Gating_On(void)
{
/* ==================================================================
     * 1. GPIO 全组端口时钟使能 (必须最优先，防止 MX_GPIO_Init 引脚配置死机)
     * ================================================================== */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    // __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE(); // FDCAN / SPI / TIM 常用引脚组
    // __HAL_RCC_GPIOE_CLK_ENABLE(); // 高级定时器 TIM1/TIM8 常用引脚组
    // __HAL_RCC_GPIOF_CLK_ENABLE(); // SPI4 / I2C3 常用引脚组
    __HAL_RCC_GPIOG_CLK_ENABLE(); // UART5 / UART4 常用引脚组
    // __HAL_RCC_GPIOH_CLK_ENABLE(); // 备用 GPIO 组
    __HAL_RCC_GPIOI_CLK_ENABLE(); // 备用 GPIO 组
    __HAL_RCC_GPIOZ_CLK_ENABLE(); // 跨域/低功耗控制引脚组

    /* ==================================================================
     * 2. 基础系统时钟与核心核间通讯 (IPCC1 + HPDMA1)
     * ================================================================== */
    // __HAL_RCC_TIM6_CLK_ENABLE();   // 关键死穴：HAL 库自身的 Timebase Source (TIM6)
    // __HAL_RCC_IPCC1_CLK_ENABLE();  // 📢 核间通讯时钟：确保 M33 与 A35 通过 OpenAMP 交互时不卡死
    // __HAL_RCC_HPDMA1_CLK_ENABLE(); // 高性能 DMA1 时钟：用于传感器数据的无感搬运
    // __HAL_RCC_HPDMA2_CLK_ENABLE(); // 高性能 DMA2 时钟：用于传感器数据的无感搬运

    /* ==================================================================
     * 3. 强行开启片内共享内存依赖的全部 SRAM 时钟
     * ================================================================== */
    // __HAL_RCC_SYSRAM_CLK_ENABLE();   // 开启系统级 SRAM 总线时钟
    // __HAL_RCC_BKPSRAM_CLK_ENABLE();  // 开启备份 SRAM 时钟

    /* ==================================================================
     * 4. 模拟采样外设 (ADC1 & ADC2)
     * ================================================================== */
    __HAL_RCC_ADC12_CLK_ENABLE();  // 在 MP2 中，ADC1 和 ADC2 共用该总线时钟开关

    /* ==================================================================
     * 5. 飞控核心车载总线 (双 FDCAN)
     * ================================================================== */
    __HAL_RCC_FDCAN_CLK_ENABLE();  // 使能 FDCAN 外设控制总线时钟（覆盖 FDCAN1 和 FDCAN2）

    /* ==================================================================
     * 6. 板载传感器与通信总线 (I2C & SPI)
     * ================================================================== */
    __HAL_RCC_I2C1_CLK_ENABLE();
    __HAL_RCC_I2C3_CLK_ENABLE();

    __HAL_RCC_SPI1_CLK_ENABLE();
    __HAL_RCC_SPI4_CLK_ENABLE();

    /* ==================================================================
     * 7. 电调控制、PWM 输出及捕获定时器阵列 (TIM)
     * ================================================================== */
    // 高级定时器 (Motor Control)
    __HAL_RCC_TIM1_CLK_ENABLE();
    __HAL_RCC_TIM8_CLK_ENABLE();
    
    // 通用定时器
    __HAL_RCC_TIM2_CLK_ENABLE();
    __HAL_RCC_TIM4_CLK_ENABLE();
    __HAL_RCC_TIM5_CLK_ENABLE();
    __HAL_RCC_TIM12_CLK_ENABLE();
    __HAL_RCC_TIM13_CLK_ENABLE();
    __HAL_RCC_TIM14_CLK_ENABLE();

    /* ==================================================================
     * 8. 庞大的串行通讯总线阵列 (UART & USART)
     * ================================================================== */
    __HAL_RCC_UART4_CLK_ENABLE();
    __HAL_RCC_UART5_CLK_ENABLE();
    __HAL_RCC_UART7_CLK_ENABLE();
    __HAL_RCC_UART8_CLK_ENABLE();
    __HAL_RCC_UART9_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_USART3_CLK_ENABLE();
    __HAL_RCC_USART6_CLK_ENABLE();

    /* ==================================================================
     * 9. 硬件同步屏障 (确保所有总线供电和时钟完全稳定)
     * ================================================================== */
    __DSB(); // Data Synchronization Barrier (数据同步屏障：确保所有时钟寄存器写入已完成)
    __ISB(); // Instruction Synchronization Barrier (指令同步屏障：刷新 M33 内核流水线)
}

void AFC7_Trigger_System_Hot_Reset(void)
{
// 1. 🚨 精准检测：如果不是冷启动（PORRSTF位为0），说明是调试器热复位进来的
    if ((RCC->HWRSTSCLRR & RCC_HWRSTSCLRR_PORRSTF) == 0) 
    {
        /* ----------------------------------------------------------------- */
        /* 🤝 局部清洗：不触发系统复位，只对 M33 侧可见的外设进行软清洗 */
        /* ----------------------------------------------------------------- */
        
        // 清除所有的复位状态标志（向该寄存器写全 1 会自动清除对应标志）
        RCC->HWRSTSCLRR = 0xFFFFFFFF;
        
        // 2. 🚨 纯掩码封杀：直接关闭 M33 侧（CPU2）所有的 IPCC1 中断使能
        // 在 MP257 架构中，C2CR 的低 2 位（Bit 0 和 Bit 1）掌控着中断大门
        // 我们用 0xFFFFFFFC 直接把这两位清零，从物理电平上直接断开 A35 的信号源
        IPCC1->C2CR &= 0xFFFFFFFC;
        
        // 3. 强行向状态清除寄存器写全 1，强制洗掉硬件内部属于 M33 侧的所有通道状态 (Occupied/Free)
        IPCC1->C2SCR = 0xFFFFFFFF;
        
        // 4. 清除 M33 侧 NVIC 内部所有可能处于挂起状态的旧中断
        // 🚨 同样改用硬核中断号：在 M33 (Cortex-M33) 的 NVIC 体系中，
        // IPCC1_CPU2 对应的硬件接收和发送中断位置一般为固定的中断向量号，
        // 为了确保头文件兼容，我们可以直接在底层清除这两个潜在的隐患：
        #if defined(IPCC1_CPU2_RX_IRQn)
        HAL_NVIC_ClearPendingIRQ(IPCC1_CPU2_RX_IRQn);
        HAL_NVIC_ClearPendingIRQ(IPCC1_CPU2_TX_IRQn);
        #elif defined(IPCC1_RX_IRQn)
        HAL_NVIC_ClearPendingIRQ(IPCC1_RX_IRQn);
        HAL_NVIC_ClearPendingIRQ(IPCC1_TX_IRQn);
        #else
        // 如果宏定义全部失效，直接通过内核原生 NVIC 寄存器无脑清除（全平台通用）
        // 针对常驻硬件中断线进行 Pending 标志位刷洗
        NVIC->ICPR[0] = 0xFFFFFFFF;
        NVIC->ICPR[1] = 0xFFFFFFFF;
        #endif
        
        // 5. 让内部流水线同步，保证清零立刻生效
        __DSB();
        __ISB();
        
        // 💡 局部复位完成，平滑向下执行，无缝衔接随后的 MX_IPCC1_Init
        return;
    }
    
    // 如果是正常的冷启动上电，同样清洗标志放行
    RCC->HWRSTSCLRR = 0xFFFFFFFF;
}
/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
