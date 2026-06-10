/******************** (C) COPYRIGHT 2019 ACE Tech Co.*************************
 * 作者	  ： 曾庆华
 * 文件名  ：AFCGlobalVar.h
 * 描述    ：AFC-2快速原型组件程序全局变量定义文件
 * 官网    ：www.acecreator.com
 * 淘宝    ：acecreator.taobao.com
 * 公众号  ：无人飞行控制
*****************************************************************************/
#ifndef __AFC_GlobalVar_H__
#define __AFC_GlobalVar_H__

#include "AFCGlobalDef.h"

#ifndef __GLOBAL_VAR_FIRST_USE__
   #define _Extern_Global_Var_ extern
#else
	 #define _Extern_Global_Var_ 
#endif
	// _Extern_Global_Var_ uint8_t volatile g_iFlyMode;    // 实时时钟多任务周期计数器
	_Extern_Global_Var_ uint16_t g_iSimulinkAlgorithmStep;          // 用户定义的simulink算法程序的控制步长
	_Extern_Global_Var_ TRealTimeCnt g_sRealTimeCount;    // 飞行控制实时任务调度相关的时钟计数器
	// _Extern_Global_Var_ bool volatile g_bUsedOfAFCBasicAC;    // AFCBasicAC模块使用标志

	// 获取无人机的配置参数
	_Extern_Global_Var_ SSaveParamFlash  g_UavFcsParam;

	_Extern_Global_Var_ char g_sModelName[120],g_sModelVersion[8],g_sAFCToolBoxVersion[8];
#endif
	 
/************************ (C) COPYRIGHT ACE Co. about ANSGlobalVar *****END OF FILE****/
