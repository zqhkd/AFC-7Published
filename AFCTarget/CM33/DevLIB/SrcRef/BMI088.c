/******************** (C) COPYRIGHT 2019 ACE Tech Co.*************************
 * 作者	  ： 曾庆华
 * 文件名  BMI088.c
 * 版  本 ： 
 *    V4.01.220402 -- (1) 基于ADIS16507.c的V4.01.220320直接进行更改
 * 
 * 描述   ：BMI088芯片惯组接口底层函数
 * 官网   ：www.acecreator.com
 * 淘宝   ：acecreator.taobao.com
 * 公众号 ：无人飞行控制
 *
*****************************************************************************/
	
#include "main.h"
#include "spi.h"
#include "BMI088.h"

// MEMS惯组42688的全局变量数据
bool g_bUsedOfBMI088;   // BMI088使用标志

void initBMI088(uint8_t iGyroRangeSel,uint8_t iAcclRangeSel,uint8_t iSlideNum,uint8_t iAntiAliasFreq,uint8_t iNotchFreq) 
{
	
}

void BMI088_ReadData(void) 
{
}
/************************ (C) COPYRIGHT ACE Co. about ADIS164XX *****END OF FILE****/
