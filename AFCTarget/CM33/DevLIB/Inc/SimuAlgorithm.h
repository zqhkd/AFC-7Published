/******************** (C) COPYRIGHT 2020 ACG Tech Co.*************************
 * 作    者 ： 曾庆华
 * 文 件 名 ：SimuAlgorithm.h
 * 版    本 ：V1.05.200128
 * 描    述 ：机载飞控程序入口头文件
 * 官    网 ：www.acecreator.com
 * 淘    宝 ：acecreator.taobao.com
 * 公 众 号 ：无人飞行控制
*****************************************************************************/
#ifndef __ACG_SimuAlgorithm_H__
#define __ACG_SimuAlgorithm_H__

#include "stdint.h"

#define DTCOM_SAMPLE_PERIOD  2     // 表示DtCom输出刷新率为2ms

void InitializeModel(void);
void StepAlgorithmModel(void);

#endif

/************************ (C) COPYRIGHT ACG co. *****END OF FILE****/
