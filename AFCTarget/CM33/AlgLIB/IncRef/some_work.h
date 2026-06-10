#ifndef __SOME_WORK_H__
#define __SOME_WORK_H__

#include "stm32h7xx_hal.h"
#include "stdbool.h"
#include "vector3f.h"

//extern bool System_Init_Status;
//extern bool Land_Complete; 
//extern bool Armed_Enable; 
//extern bool dcm_fast_ground_gains; 
//void get_rc_channel_value(void);

void get_rccmd_simulink_value(double iRcRawVal[4],int16_t RCServCmd[3],int16_t *RCThrottle,int16_t *RCClimbRate);

void fifty_hz_loop(double fWxyz[3], double fAxyz[3],double fDcmRoll, double fDcmPitch, int16_t iThrottleOut);

// 为新建AFCAuxFun.c文件添加相关代码
void level_space_check(Vector3f Wxyz, Vector3f Axyz);

// 获取AFC当前状态
AFCSTATUS getAFCSts(void);

void initAFCSts(void);
void setAFCLandCompleteSts(bool bVal);
void setAFCSts(uint8_t iBit,bool bVal);

void setRcLimitFlg(bool bRCRoll,bool bRCPitch,bool bRCThrottle,bool bRCCh3,bool bRCCh2,bool bRCCh1,bool bRCCh0);

#endif

