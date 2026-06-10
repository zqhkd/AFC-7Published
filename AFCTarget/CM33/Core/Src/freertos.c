/* USER CODE BEGIN Header */
/*******************************************************************************
 * (C) COPYRIGHT 2026 ACE Tech Co.
 * 作    者 ： 曾庆华
 * 文 件 名 ： freertos.c
 * 版    本 ： V7.01.260603
 * 描    述 ： FreeRTOS 多速率子任务容器，实现默认主根任务向基频控制环的无缝升级
 *******************************************************************************/
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
/* USER CODE BEGIN Variables */
/* 多速率业务任务句柄全局总线装订 */
osThreadId_t Task01Handle = NULL;
osThreadId_t Task02Handle = NULL;
osThreadId_t Task03Handle = NULL;
osThreadId_t defaultTaskHandle = NULL;

// osThreadId_t TaskAirSpeedHandle; 

/* 默认多率分频周期参数定义（毫秒单位） */
uint32_t g_Task01PeriodMs = 10;
uint32_t g_Task02PeriodMs = 20;
uint32_t g_Task03PeriodMs = 30;

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 4096,  // 4KB 充足栈空间防御
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
/* 弱符号多率业务控制逻辑实体（Simulink 顶层自动编译覆盖此 3 个弱符号） */
__weak void Task01Isr(void){}
__weak void Task02Isr(void){}
__weak void Task03Isr(void){}

/* ==========================================================================
 * Task01 ~ Task03 线程：标准 CMSIS-RTOS V2 多率调度
 * ========================================================================== */
void highThreadTask01(void *argument) {
    (void)argument;
    uint32_t tick = osKernelGetTickCount();
    for(;;) {
        tick += pdMS_TO_TICKS(g_Task01PeriodMs);
        osDelayUntil(tick);
        Task01Isr(); 
    }
}

void mediumThreadTask02(void *argument) {
    (void)argument;
    uint32_t tick = osKernelGetTickCount();
    for(;;) {
        tick += pdMS_TO_TICKS(g_Task02PeriodMs);
        osDelayUntil(tick);
        Task02Isr(); 
    }
}

void lowThreadTask03(void *argument) {
    (void)argument;
    uint32_t tick = osKernelGetTickCount();
    for(;;) {
        tick += pdMS_TO_TICKS(g_Task03PeriodMs);
        osDelayUntil(tick);
        Task03Isr(); 
    }
}

/* 响应顶层配置的多率动态子任务装订注册模块 */
void initTaskScheduler(uint8_t iTask01Period, uint8_t iTask02Period, uint8_t iTask03Period)
{
    g_Task01PeriodMs = iTask01Period; g_Task02PeriodMs = iTask02Period; g_Task03PeriodMs = iTask03Period;
    
    if (g_Task01PeriodMs > 0) {
        const osThreadAttr_t t1_attr = { .name = "Task01", .priority = osPriorityHigh, .stack_size = 1024 };
        Task01Handle = osThreadNew(highThreadTask01, NULL, &t1_attr);
    }
    if (g_Task02PeriodMs > 0) {
        const osThreadAttr_t t2_attr = { .name = "Task02", .priority = osPriorityAboveNormal, .stack_size = 1024 };
        Task02Handle = osThreadNew(mediumThreadTask02, NULL, &t2_attr);
    }
    if (g_Task03PeriodMs > 0) {
        const osThreadAttr_t t3_attr = { .name = "Task03", .priority = osPriorityNormal, .stack_size = 1024 };
        Task03Handle = osThreadNew(lowThreadTask03, NULL, &t3_attr);
    }
    
    #ifdef __AFC7_ONLYM33_DEBUG_MODE__
        const osThreadAttr_t v35_attr = { .name = "VirtA35", .priority = osPriorityBelowNormal, .stack_size = 1024 };
        osThreadNew(Thread_Virtual_A35_Driver, NULL, &v35_attr);
    #endif
}
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

  (void)argument;
    
    /* 1. 在这里根据加载完成的参数，动态装订派生 Task01~Task03 的线程拓扑 */
    initTaskScheduler(g_Task01PeriodMs, g_Task02PeriodMs, g_Task03PeriodMs);

    /* 2. 将自身提升至飞控最高硬实时抢占级别，杜绝被慢速网络业务挂起 */
    osThreadSetPriority(osThreadGetId(), osPriorityRealtime);

    /* 3. 🚨 闭环跳转：单向调用控制底座主循环外部死循环函数，控制权彻底移交 🚨 */
    BaseThreadTask00(argument);

  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

