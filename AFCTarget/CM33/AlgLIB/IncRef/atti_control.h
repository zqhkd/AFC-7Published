#ifndef __ATTI_CONTROL_H
#define __ATTI_CONTROL_H

#include "stm32h7xx_hal.h"
#include "stdbool.h"
#include "vector3f.h"
#include "fc_math.h"

//extern Vector3f angle_ef_target; 
//extern Vector3f rate_bf_target;

extern float angle_kP[3];
extern PID pid_rate[3];

void attitude_control(double roll_angle_ef, double pitch_angle_ef, double yaw_rate_ef,double fDcmAng[3],double fWz,double *fAngleEfTargetZ, double fRateBfTarget[3]);
void attitude_output_controller(AFCSTATUS sts,double fRateBfTarget[3],double fDcmRate[3], double fServoOut[3]);
void setRateBfTarget(double fDcmWxyz[3],double dcmYaw,double *fAngleEfTargetZ, double fRate_bf_target[3]);

#endif
