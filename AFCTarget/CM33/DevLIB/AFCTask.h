/******************** (C) COPYRIGHT 2019 ACE Tech Co.*************************
 * 作  者 ： 曾庆华
 * 文件名 ： AFCTask.h
 * 版  本 ： V1.03.200114
 *           V
 * 描  述 ：ANS-1任务管理模块
 * 官  网 ：www.acecreator.com
 * 淘  宝 ：acecreator.taobao.com
 * 公众号 ：无人飞行控制
*****************************************************************************/
#ifndef __AFC_TASK_H__
#define __AFC_TASK_H__
#include "stdint.h"
#include "stdbool.h"
#include "AFCGlobalDef.h"
#include "AFCGlobalVar.h"

#include "SbusRC.h"

//  V5.04.0623: 用于风速仪校准计时变量 
extern uint16_t iCounter;

void AFCTaskInit(void);
void initAFCPara(void);
void TaskRealTimeCount(void);
void MainProc(void);

void 	Task00Isr(void);
void 	SysTask(void);

void runResult2PC(double dcmAng[3],double fPositionZ);

void CioDeviceSampleTask(void);
#endif

/************************ (C) COPYRIGHT ACG co. *****END OF FILE****/
