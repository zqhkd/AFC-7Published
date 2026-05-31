/******************** (C) COPYRIGHT 2019 ACE Tech Co.*************************
 * 作者	  ： 曾庆华
 * 文件名  AFCScanner.h
 * 描述    ：压力扫描阀接口底层函数
 * 官网    ：www.acecreator.com
 * 淘宝    ：acecreator.taobao.com
 * 公众号  ：无人飞行控制
*****************************************************************************/
#ifndef __AFC_Scanner_H__
#define __AFC_Scanner_H__
#include "stdint.h"
#include "stdbool.h"

#define AFC_SCANNER_PERIOD     5     // 扫描阀采集周期为5ms

extern bool g_bUsedOfPressScanner;   // 压力扫描阀使用标志
extern bool g_bUsedOfTempScanner;   // 温度扫描阀使用标志

// 初始化压力扫描阀
void initPressScan(void);   
// 读取全部压力数据
void ReadAllPressData(void);
// 读取压力扫描阀指定通道数据
double getPressData(uint8_t iChannel);

// 初始化温度扫描阀
void initTempratureScan(void);
////关闭温度采样
//void disableTempScanner(void);
////关闭压力采样
//void disablePressScanner(void);
//读取温度扫描存储数组
void ReadAllTemperatureData(void);
//读取指定通道的转换数据
double getTempratureData(uint8_t iChannel);

#endif

/************************ (C) COPYRIGHT ACG co. *****END OF FILE****/
