#ifndef __Caculate_Motors_Cmd_H__
#define __Caculate_Motors_Cmd_H__

#include "stm32h7xx_hal.h"
#include "stdbool.h"
#include "AFCGlobalDef.h"

void Motors_Output_Min(void);
void outMotorsPWM(AFCSTATUS sts,double fServoOut[3], double *fThrottleOut, int16_t iMotorsPWM[MOTORS_CHANEL_NUM]);

//void motorsOutByPwmVal(double iMotorsPWM[MOTORS_CHANEL_NUM]);
void motorsOutByEscVal(double fMotorsESCVal[MOTORS_CHANEL_NUM]);

void initMotorsAFCSts(void);
void getMotorsAFCSts(AFCSTATUS *sts);
void setMotorSlowStart(int16_t iThrottleOut);

#endif


