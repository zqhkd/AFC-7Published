/******************** (C) COPYRIGHT 2026 AFC Tech Co.*************************
 * 作    者  ： 曾庆华
 * 文 件 名  ： A35CommonFun.h
 * 版    本  ： AFC-7.A35.V1.0.260329:  
 * 描    述  ：OpenSTLinux系统共用函数
 * 官    网  ：www.acecreator.com
 * 淘    宝  ：acecreator.taobao.com
 * 公 众 号  ：飞行控制与仿真
*****************************************************************************/
#ifndef __A35_COMMPN_FUN_H__
#define __A35_COMMPN_FUN_H__

// 按任意键继续
void WaitAnyKey(char *tmps);

//  创建并启动绑定到特定 CPU 的实时任务, cpuId 绑定的核心编号 
//     双核 A35 在 Linux 系统中逻辑编号通常为 CPU 0 和 CPU 1: 
//       核 0 (CPU 0)：系统默认核心。通常负责处理系统中断、内核任务以及非实时性应用。
//       核 1 (CPU 1)：实时任务的首选核心。结合您之前提到的 1ms 确定性调度需求，应通过内核启动参数 isolcpus=1 将此核从系统调度器中隔离，并在应用层将实时任务显式绑定到核心 1 。 
int CreateAndStartTask(void *(*taskFunc)(void *), const char *taskName, int stackSize, int priority, int schedpolicy, int cpuId, void *arg);

#endif
/************************ (C) COPYRIGHT ACG co. *****END OF FILE****/