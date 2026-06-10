#ifndef __FLY_MODE_MODULE_H__
#define __FLY_MODE_MODULE_H__

#include "stm32h7xx_hal.h"
#include "vector3f.h"
#include "some_work.h"

void flyModeInit(void);
void flyModeRun(AFCSTATUS sts,double position_z,double velocity_z,double fDcmAng[3], double fDcmWxyz[3], double fThrottleHover,double fRCCmd[4], double *Throttle_Out, double fRateBfTarget[3]);
#endif
