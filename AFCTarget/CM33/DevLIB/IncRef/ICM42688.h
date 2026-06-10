/******************** (C) COPYRIGHT 2019 ACE Tech Co.*************************
 * 作者	  ： 曾庆华
 * 文件名  ：ICM42688.c
 * 描述    ：ICM42688芯片惯组接口底层函数
 * 官网    ：www.acecreator.com
 * 淘宝    ：acecreator.taobao.com
 * 公众号  ：无人飞行控制
*****************************************************************************/
#ifndef __ICM42688_H__
#define __ICM42688_H__
#include "stdint.h"
#include "stdbool.h"

extern bool g_bUsedOfICM42688;   // ICM42688使用标志
extern float icm_42688_acc[3],icm_42688_gyr[3],icm_42688_temp;

// ? : 增加抗混叠滤波器和限波器参数设置
void initICM42688(void);   

// 读取ICM42688全部数据
void ICM42688_ReadData(void);

#endif

/************************ (C) COPYRIGHT ACG co. *****END OF FILE****/
