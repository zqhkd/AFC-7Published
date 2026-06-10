/******************** (C) COPYRIGHT 2019 ACE Tech Co.*************************
 * 作者	  ： 曾庆华
 * 文件名  ：ADSample.c
 * 描述    ：ADSample电压电流采集接口函数
 * 官网    ：www.acecreator.com
 * 淘宝    ：acecreator.taobao.com
 * 公众号  ：无人飞行控制
*****************************************************************************/
#ifndef __ADSample_H__
#define __ADSample_H__
#include "stdint.h"
#include "stdbool.h"

extern uint16_t ad_vol[2];

// 读取ADSample全部数据
void ADSample_ReadData(void);

// 读取ADC1值
uint16_t readADC1Value(void);

// 读取ADC2值
uint16_t readADC2Value(void);

#endif

/************************ (C) COPYRIGHT ACG co. *****END OF FILE****/
