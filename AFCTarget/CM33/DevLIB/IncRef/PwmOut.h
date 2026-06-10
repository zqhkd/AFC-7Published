#ifndef __PWMOUT_H__
#define __PWMOUT_H__

#include "stdint.h"
#include "stdbool.h"

#define MAX_SERV_CH_NUM 12

void updateTimerFreq(uint8_t iCh,uint16_t preScaleVal,double escFreq,bool bStartTime);

uint32_t iGetTimerArr(uint8_t iCh);
void Set_Motor_PWM(uint8_t num,uint16_t value);

// 设置电调的DSHOT指令帧
void Set_Motor_DShot(uint8_t iChanel, uint16_t value);
// DShot电调解锁
void DShot_Unlock(uint8_t iChanel) ;

void PWM_Out_Test(void);


#endif
