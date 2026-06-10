#ifndef __ICM20602_H__
#define __ICM20602_H__

#include "stdint.h"
#include "stdbool.h"

extern bool g_bUsedOfICM20602;   // ICM42688使用标志
extern float icm_20602_acc[3],icm_20602_gyr[3],icm_20602_temp;

void initICM20602(void);
void ICM20602_ReadData(void) ;

#endif

