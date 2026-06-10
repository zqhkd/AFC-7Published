/******************** (C) COPYRIGHT 2019 ACE Tech Co.*************************
 * 作  者 ： 曾庆华
 * 文件名 ： AFCBasicAPI.h
 * 版  本 ： V5.02.231221
 * 描  述 ： 基本飞行功能接口模块（Basic Fly Control API Module）
 * 官  网 ： www.acecreator.com
 * 淘  宝 ： acecreator.taobao.com
 * 公众号 ： 无人飞行控制
*****************************************************************************/
#ifndef __AFC_BASIC_API_H__
#define __AFC_BASIC_API_H__
#include "stdint.h"
#include "stdbool.h"
#include "AFCGlobalDef.h"

//extern float accel_z_cms,leash_down_z, leash_up_z;

void ahrsStep(double fWxyz[3],double fAxyz[3],double fHeadAng,bool bDcmFast,double fDcmAng[3], double fDcmWxyz[3]);  //  姿态解算, 由于simulink中存在地址引用问题，生存代码特别容易出错

void ahrsNStep(double fWx,double fWy,double fWz,double fAx,double fAy,double fAz,double fHeadAng,bool bDcmFast,double fDcmAng[3], double fDcmWxyz[3]);  //  姿态解算

// 姿态位置控制
void initAttitudePos(void);
void attitudePosStep(double roll_angle_ef, double pitch_angle_ef, double yaw_rate_ef,double fDcmAng[3],double fWz, double *fAngleEfTargetZ, double fRateBfTarget[3], double *fEfTargetZ);
// 姿态速度控制
void initAttitudeRate(void);
void attitudeRateStep(uint32_t iSts,double fRateBfTarget[3],double fDcmRate[3], double fChServoOut[3]);

// 高度控制初始化
// 高度控制初始化
void initAltitude(void);
		//高度状态更新
void 	altitudeStep(uint32_t sts,double fBarHeight,double fRollDegX100, double fPitchDegX100, double fThrottleOut,double *fPositionZ,double *fVelocityZ, double *fThrottleHover);


// 根据高度通道的位置、速度、爬升率等计算油门大小
void initAltitudeThrottle(void);

//// 根据高度通道的位置、速度、爬升率等计算油门大小
//double altitudeThrottleStep(uint32_t iSts, double position_z, double velocity_z, double fClimbRate,double dcmRoll, double dcmPitch,double fThrottleHover);

//// V5.02.240524新增: 和altitudeThrottleStep相比，主要是增加两个变量：高度回路加速度积分控制参数altAccelInt和目标高度值posTarZ
//double altThrottleStep(uint32_t iSts, double position_z, double velocity_z, double fClimbRate,double dcmRoll, double dcmPitch,double altAccelInt,double *posTarZ,double fThrottleHover);

// 初始化起飞程序
void initTakeOff(double position_z, double fThrottleOut);

// 水平校准，电机解锁等
//void  takeOffLandRun(double fWxyz[3],double fAxyz[3],double fDcmRoll, double fDcmPitch, double fThrottleOut,double fChServoOut[3]);
void  takeOffLandRun(double fWxyz[3],double fAxyz[3],double fDcmRoll, double fDcmPitch, double fThrottleOut);
void  takeOffLand(double fWx,double fWy,double fWz,double fAx,double fAy,double fAz,double fDcmRoll, double fDcmPitch, double fThrottleOut);

// 起飞前处理，或者着陆处理。根据地面情况获取油门控制指令
void groundProc(double position_z,double fDcmAng[3], double fDcmWxyz[3],double *fAngleEfTargetZ,double fRateBfTarget[3],double *fThrottle);

// 输出电机电调指令值（0--100）
void outMotorsValStep(uint32_t iSts,double fServoOut[3], double *fThrottle, double fMotorsEscVal[4]);

void initFlyMode(void);
// 飞行模式（即在flyMode.c中的顶层调度函数flyModeRun）
void 	flyModeStep(uint32_t iSts,double fPositionZ,double fVelocityZ,double fDcmAng[3],double fDcmWxyz[3],double fThrottleHover,double fRCCmd[4],double *fThrottleOut, double fRateBfTarget[3]);

// 获取RC各个通道控制指令
void getCtrlChCmd(double fRCInput[4],double fCtrlChCmd[4]);

// AFC-5飞控状态字获取
uint32_t getAFCUintSts(AFCSTATUS sSts);
AFCSTATUS getAFCStsBit(uint32_t iSts);

uint32_t getAFCStsInt(void);

// 目前和遥控器相关的这个函数仍然处于some_work.c文件中，后续需处理
//void getCurRCVal(int16_t *RC_Roll, int16_t *RC_Pitch, int16_t *RC_Yaw, uint16_t *RC_Throttle,int16_t *RC_Climb_Rate);
void setRcLimit(double bRCRoll,double bRCPitch,double bRCThrottle,double bRCCh3,double bRCCh2,double bRCCh1,double bRCCh0);
#endif

/************************ (C) COPYRIGHT ACG co. *****END OF FILE****/
