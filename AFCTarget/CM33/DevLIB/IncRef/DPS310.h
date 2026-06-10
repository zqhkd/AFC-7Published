/******************** (C) COPYRIGHT 2019 ACE Tech Co.*************************
 * 作者	  ： 曾庆华
 * 文件名  ：DPS310.c
 * 描述    ：DPS310气压计芯片接口底层函数
 * 官网    ：www.acecreator.com
 * 淘宝    ：acecreator.taobao.com
 * 公众号  ：无人飞行控制
*****************************************************************************/
#ifndef __DPS310_H__
#define __DPS310_H__
#include "stdint.h"
#include "stdbool.h"

extern bool g_bUsedOfDPS310;   // ICM42688使用标志
extern bool resetDPS_TP;
extern float baro_altitude;
extern float pressure, temperature;

// V4.02.220621: 增加抗混叠滤波器和限波器参数设置
void initDPS310(void);   

// 读取DPS310全部数据
void DPS310_ReadData(void);

// 高度复位标志
void resetDPS310(bool bflg);
#endif

/************************ (C) COPYRIGHT ACG co. *****END OF FILE****/
