#ifndef __AFC_Tof_Com_H__
#define __AFC_Tof_Com_H__

#include "stdint.h"
#include "usart.h"
#include <stdbool.h>

extern bool g_bUsedOfTOF03;
// AFC-4V4.02.220318: 添加TOF-03激光雷达接口
void ProTofComRcvData(void);

void initTof(uint8_t iSlideNum);
double getTofData(uint8_t iChannel);
#endif

/************************ (C) COPYRIGHT ACG co. *****END OF FILE****/
