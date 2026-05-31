/******************** (C) COPYRIGHT 2019 ACE Tech Co.*************************
 * 作者	  ： 曾庆华
 * 文件名  ：FemtMesGps.h
 * 描述    ：GPS NMEA字符流处理函数库声明
 * 官网    ：www.acecreator.com
 * 淘宝    ：acecreator.taobao.com
 * 公众号  ：无人飞行控制
*****************************************************************************/
#ifndef __FemtMesGps_H__
#define __FemtMesGps_H__	 
#include <stdint.h>
#include <stdbool.h>

// 校验和引导符'*'自后向前所在位置。规定格式为*hh(cr)(lf)。其中hh为检验和的ASCII字符，为2字节，cr,lf为回车、换行，也为2字节
#define AsteriskPos          5
//NMEA 0183 协议解析后数据存放结构体

// 获取GPS秒脉冲信号
bool getGpsPpsSignal(void);
// 获取GPS信号更新状态
bool getGpsUpdateFlg(void);

void GPS_Analysis(uint8_t *buf);
#endif

/************************ (C) COPYRIGHT ACG co. *****END OF FILE****/
