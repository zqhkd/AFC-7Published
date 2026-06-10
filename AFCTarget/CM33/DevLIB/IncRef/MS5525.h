/******************** (C) COPYRIGHT 2019 ACE Tech Co.*************************
 * 作者	  ： 曾庆华
 * 文件名  ：MS5525.c
 * 描述    ：MS5525风速计接口底层函数
 * 官网    ：www.acecreator.com
 * 淘宝    ：acecreator.taobao.com
 * 公众号  ：无人飞行控制
*****************************************************************************/
#ifndef __MS5525_H__
#define __MS5525_H__
#include "stdint.h"
#include "stdbool.h"

extern bool g_bUsedOfMs5525,bCalDiffPressPaZeroOff;   // 空速管Ms5525使用标志,差压信号校准标志
extern float diff_press_vel,ms5525_temperature,diff_press_pa_raw, gMS5525SampleTim;  // MS5525的采样时间

// 初始化Ms5525风速计
void initMS5525(float sampleTim);   
// 读取Ms5525全部数据
void MS5525_ReadData(void);
// 计算差压信号的零偏校准值。校准标志bCalDiffPressPaZeroOff为真表示已校准，零偏校准值存放在diff_press_init中
void calDiffPressZeroOff(uint32_t iCaliCounter,bool bCaliDoing);
#endif

/************************ (C) COPYRIGHT ACG co. *****END OF FILE****/
