/* * 摘录自米尔官方 stm32mp257f_eval.h 
 * 仅保留阶段 5 核心任务所需的资源定义，避免命名冲突
 */
#ifndef __MYIR_BOARD_DEFS_H
#define __MYIR_BOARD_DEFS_H

#include "res_mgr.h"

/* ==================================================================
 * NakedAFC7 阶段1：裸机硬件资源权限定义
 * 严格提取自 STM32MP257F-EV1 官方 BSP 底层握手协议
 * ================================================================== */

/* --- UART5 (TX: PG9, RX: PG10) 权限定义 --- */
#define MYIR_UART5_RIF_RES_TYP      RESMGR_RESOURCE_RIFSC
#define MYIR_UART5_RIF_RES_ID       RESMGR_RIFSC_UART5_ID

#define MYIR_UART5_TX_PORT_TYP      RESMGR_RESOURCE_RIF_GPIOG
#define MYIR_UART5_TX_PIN_ID        RESMGR_GPIO_PIN(9)

#define MYIR_UART5_RX_PORT_TYP      RESMGR_RESOURCE_RIF_GPIOG
#define MYIR_UART5_RX_PIN_ID        RESMGR_GPIO_PIN(10)

#define MYIR_UART5_RCC_RES_TYP      RESMGR_RESOURCE_RIF_RCC
#define MYIR_UART5_RCC_RES_ID       RESMGR_RCC_RESOURCE(96) // 官方: PG9/PG10 对应的 RCC 权限

/* --- LED3 (Blue LED: GPIOZ Pin 5) 权限与物理定义 --- */
#define MYIR_LED3_RIF_RES_TYP       RESMGR_RESOURCE_RIF_GPIOZ
#define MYIR_LED3_RIF_RES_ID        RESMGR_GPIO_PIN(5)

#define MYIR_LED3_RCC_RES_TYP       RESMGR_RESOURCE_RIF_RCC
#define MYIR_LED3_RCC_RES_ID        RESMGR_RCC_RESOURCE(99) // 官方: GPIOZ 对应的 RCC 权限

#define MYIR_LED3_PORT              GPIOZ
#define MYIR_LED3_PIN               GPIO_PIN_5

#endif /* __MYIR_BOARD_DEFS_H */