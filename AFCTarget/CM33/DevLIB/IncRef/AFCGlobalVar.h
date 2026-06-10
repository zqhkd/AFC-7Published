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

// 和实时飞行任务相关的时钟信号
//	_Extern_Global_Var_ uint8_t volatile g_iFlyMode;    // 实时时钟多任务周期计数器
	_Extern_Global_Var_ bool volatile g_bUsedOfAFCBasicAC;    // AFCBasicAC模块使用标志
	 
	_Extern_Global_Var_ TRealTimeCnt volatile g_sRealTimeCount;    // 实时时钟多任务周期计数器
	_Extern_Global_Var_ uint16_t g_iSimulinkAlgorithmStep;          // 用户定义的simulink算法程序的控制步长
	_Extern_Global_Var_ float Control_Dt;          // 用户定义的simulink算法程序的控制步长
   
	_Extern_Global_Var_	uint8_t  g_iCurRunMode;          // 当前工作模式
	_Extern_Global_Var_	uint16_t g_iProcessRunNo;       // 进程运行编号

	_Extern_Global_Var_	uint16_t g_iWriteSDFailedCnt;       // 写入SD卡失败次数
	_Extern_Global_Var_	uint8_t g_iUSBDiskWriteFreq;
	 
	_Extern_Global_Var_	int16_t roll_sensor, pitch_sensor, yaw_sensor;
	_Extern_Global_Var_	float gfPositionX,gfPositionY,gfPositionZ;
	_Extern_Global_Var_	float gfVelocityX,gfVelocityY,gfVelocityZ;
	 
	_Extern_Global_Var_	bool bInitDtComIn, bInitSimuComIn, bInitSBusComIn, bInitGpsComIn,bInitCdioComIn;   
	_Extern_Global_Var_	bool g_bDtComOutSucessful,g_bDtComOutBufIsUsing;
	_Extern_Global_Var_	bool g_bSimuComOutSucessful,g_bSimuComOutBufIsUsing;
	
	_Extern_Global_Var_ bool g_bSBusUsed[NumOfSBusChannel];  // 遥控器通道使用标志
	_Extern_Global_Var_ char g_sModelName[120],g_sModelVersion[8],g_sAFCToolBoxVersion[8];

  _Extern_Global_Var_ uint16_t g_iRCChMax,g_iRCChMin;   // 遥控器通道最大最小值

// 用户级配置参数
//	_Extern_Global_Var_ uint8_t g_iCurRemoter,g_iCurUAVFrame,g_iCurAFCBoard;  // 选择当前遥控器, 无人机机架, AFC控制器板
//	_Extern_Global_Var_ int8_t g_iAFCBoardXYZDir[3];  // AFC控制器板的安装方向
	
	_Extern_Global_Var_ uint8_t g_iM1CHANEL, g_iM2CHANEL,g_iM3CHANEL,g_iM4CHANEL;  // 选择无人机电机对应PWM的通道号
	
	// 磁力计校准用相关参数
	_Extern_Global_Var_ bool  g_bICM42688Calibing,g_bICM20602Calibing,g_bDPS310Calibing,g_bIst8310Calibing;  // 磁力计校准标志
//		_Extern_Global_Var_ float g_bMagBias[3];                    // 磁力计MagX/Y/Z的偏置
  _Extern_Global_Var_ bool g_bUsedOfWLan01,bWLanRecvReady01;
		
	// 获取FM25V01中存储的参数
	 _Extern_Global_Var_ SSaveParamFlash  g_UavFcsParam;
	 _Extern_Global_Var_	Vector3f g_GryoFilter,g_AccFilter;    // 后续可将该滤波器信息都置入无人机配置参数中

	 _Extern_Global_Var_	Vector3f g_SysRndXyz;    // 
	 
   _Extern_Global_Var_ bool  g_bTestCalibOpening[SENSOR_NUM];
	 
	 // 增加俯仰、滚转配平角
	 _Extern_Global_Var_	int16_t g_RollSensor0,g_PitchSensor0;
	 
	 _Extern_Global_Var_	float fThrottleHover,fShutOffThrottle; 
	 _Extern_Global_Var_	float accel_z_cms,leash_down_z, leash_up_z;
   _Extern_Global_Var_	int16_t g_iIdlingSpeedWidth, g_iSelfDetectPW;
	 
	 _Extern_Global_Var_	float AFC_SYSTEM_MAIN_VER,AFC_SYSTEM_DATE_VER;
	 _Extern_Global_Var_	uint32_t AFC_SYSTEM_SN_ID;
	 
#endif
	 
/************************ (C) COPYRIGHT ACE Co. about ANSGlobalVar *****END OF FILE****/
