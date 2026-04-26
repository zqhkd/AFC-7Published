/******************** (C) COPYRIGHT 2026 AFC Tech Co.*************************
 * 作    者  ： 曾庆华
 * 文 件 名  ： TLD257HwFun.h
 * 版    本  ： V1.0.260329:  
 * 描    述  ： 米尔开发板硬件功能测试应用程序接口（API）头文件
 * 官    网  ： www.acecreator.com
 * 淘    宝  ： acecreator.taobao.com
 * 公 众 号  ： 飞行控制与仿真
*****************************************************************************/
#ifndef __TLD257_HW_FUN_H__
#define __TLD257_HW_FUN_H__

#include "A35CommonFun.h"

// 模块1：LED 指示灯测试
int TLD257Led(void);

// 模块2：按键与 LED 联动测试
int TLD257KeyLed(void);

// 模块3：A35调试串口 (J15) 双向通讯测试
int TLD257Uart(void);

// 模块4：ENET2 普通千兆网 UDP 测试
int TLD257EthBasic(void);

// 模块5：ENET1 TSN 硬件时间戳测试
int TLD257EthTsn(void);

// 模块6：1ms 实时任务线程调度测试
int TLD257RtTask(void);

// 模块7: ENET1 TSN高实时双核架构顶层框架程序测试
int TLD257TsnCoreFrame(void);

#endif

/************************ (C) COPYRIGHT ACG co. *****END OF FILE****/
