/******************** (C) COPYRIGHT 2019 ACE Tech Co.*************************
 * 作者	  ： 曾庆华
 * 文件名  ：IST8310.c
 * 描述    ：IST8310地磁传感器接口底层函数
 * 官网    ：www.acecreator.com
 * 淘宝    ：acecreator.taobao.com
 * 公众号  ：无人飞行控制
*****************************************************************************/
#ifndef __IST8310_H__
#define __IST8310_H__
#include "stdint.h"
#include "stdbool.h"

extern bool g_bUsedOfIST8310;   // IST8310使用标志
extern float g_fMagRaw[3];
extern float ist8310_angle;

// V4.02.220621: 增加抗混叠滤波器和限波器参数设置
void initIST8310(void);   

// 读取IST8310全部数据
void IST8310_ReadData(void);

#endif

/************************ (C) COPYRIGHT ACG co. *****END OF FILE****/
