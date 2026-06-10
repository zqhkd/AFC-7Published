/******************** (C) COPYRIGHT 2019 ACE Tech Co.*************************
 * 作者	  ： 曾庆华
 * 文件名  ：BMI088.c
 * 描述    ：BMI088芯片惯组接口底层函数
 * 官网    ：www.acecreator.com
 * 淘宝    ：acecreator.taobao.com
 * 公众号  ：无人飞行控制
*****************************************************************************/
#ifndef __BMI088_H__
#define __BMI088_H__
#include "stdint.h"
#include "stdbool.h"

extern bool g_bUsedOfBMI088;   // BMI088使用标志

// V4.02.220621: 增加抗混叠滤波器和限波器参数设置
void initBMI088(uint8_t iGyroRangeSel,uint8_t iAcclRangeSel,uint8_t iSlideNum,uint8_t iAntiAliasFreq,uint8_t iNotchFreq);   

// 读取ICM42688全部数据
void BMI088_ReadData(void);

#endif

/************************ (C) COPYRIGHT ACG co. *****END OF FILE****/
