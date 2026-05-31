#ifndef __AFC_API_H__
#define __AFC_API_H__

#include "stdint.h"
#include "stdbool.h"

#include "ICM42688.h"
#include "ICM20602.h"

#include "DPS310.h"
#include "IST8310.h"
#include "MS5525.h"
#include "PwmOut.h"
#include "PwmIn.h"
#include "ADSample.h"
#include "SbusRC.h"

#include "FM25V01.h"

// 以下头文件为外部c文件来实现程序接口，接口函数不在AFCAPI.h中
#include "AFCDio.h"
#include "AFCGpuCom.h"
#include "AFCDtCom.h"
#include "AFCSimuCom.h"
#include "AFCGpsCom.h"

extern uint8_t giChNo[4];

extern bool g_bUsedOfADSample;
extern uint32_t g_iMinPW[MAX_SERV_CH_NUM],g_iMaxPW[MAX_SERV_CH_NUM];     // 电调脉宽最小开度对应高电平脉宽计数值、最大开度对应高电平脉宽计数值

// 获取AFC-5系统内部时钟
double getAFCTimerVal(void);

// 返回AFC-5飞控板的ID号
double getUavId(void);

void initADSample(void);
double getADSampleData(uint8_t iChannel);

// 获取MEMS惯组ICM42688的角速度、加速度信息。
double getICM42688Data(uint8_t iChannel);

// 获取MEMS惯组ICM20602的角速度、加速度信息
double getICM20602Data(uint8_t iChannel);

// 获取数字压力传感器DSP310信息。
double getDPS310Data(uint8_t iCh);

// 获取空速计MS5525信息。
double getMS5525Data(uint8_t iCh);
// 标校空速计MS5525零偏
void proDiffPressZeroOff(bool bReset, float fSampleTim);

//  获取数字磁力计传感器IST8310三轴磁力数据
double getIST8310Data(uint8_t iChannel);

// 遥控器参数读取
void initSBusCmd(uint8_t iChannel);
double readSBusCmd(uint8_t iChannel);

extern bool bEscMainSwitch;
// AFC-5V5.02.230918: 初始化PWM电调参数
void initEscPara(double escFreq, uint8_t iChanel, double minPW, double maxPW);
// AFC-5V5.05.250419: 初始化DShot电调参数
void initDShotPara(uint8_t iEscType, uint8_t iChanel);
// AFC-5V5.02.230918: 调节电调
void setEscVal(uint8_t iChanel, double fVal);
// 根据脉宽值获取电调开度值
double fGetEscVal(uint8_t idxCh,uint16_t iMotorsPWM);

// 初始化PWM采集通道
void initPwmSample(uint8_t chNum);
// 获取给定指定通道的角度值
double getPwmVal(uint8_t ch);

// 初始化舵系统采集通道
void initSrvoIn(uint8_t chNum);
// 获取给定指定通道的角度值
double getSrvoVal(uint8_t ch);

// 初始化舵系统输出通道
void initSrvoOut(uint8_t chNum);
// 获取给定指定通道的角度值
void setSrvoVal(uint8_t ch, double fVal);

// 设置各个中断任务的刷新周期
void initTaskScheduler(uint8_t iTask01Period,uint8_t iTask02Period,uint8_t iTask03Period);

void initTask0Period(void);
#endif
