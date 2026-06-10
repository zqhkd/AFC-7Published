/******************** (C) COPYRIGHT 2019 ACE Tech Co.*************************
 * 作  者 ： 曾庆华
 * 文件名 ： AFCTask.h
 * 版  本 ： V7.01.20260531
 * 描  述 ： AFC-7 任务管理模块
 * 官  网 ： www.acecreator.com
 * 淘  宝 ： acecreator.taobao.com
 * 公众号 ： 无人飞行控制
*****************************************************************************/
#ifndef __AFC_TASK_H__
#define __AFC_TASK_H__
#include "stdint.h"
#include "stdbool.h"

#include "AFCGlobalDef.h" 
#include "AFCGlobalVar.h"

void AFCTaskInit(void);
void MainProc(void);

 // Task00 核心基频线程（最高优先级抢占，绝对时空控制流）
 void BaseThreadTask00(void *argument);
 
#endif

/************************ (C) COPYRIGHT ACG co. *****END OF FILE****/
