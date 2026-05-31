#ifndef __ALT_CONTROL_H
#define __ALT_CONTROL_H

#include "stm32h7xx_hal.h"
#include "fc_math.h"
#include "stdbool.h"
#include "AFCGlobalDef.h"

//extern float velocity_z,position_z;

extern float alt_pos_kP;
extern PID alt_rate;
extern PID alt_accel;

void init_alt_LPF(void);
void altitude_update(AFCSTATUS sts,double fBarHeight,double fDcmRoll, double fDcmPitch,int16_t iThrottleOut, double *position_z, double *velocity_z, double *Throttle_Hover);
void init_takeoff(double position_z,int16_t iThrottleOut);
void set_current_alt_to_target_alt(double position_z);
int16_t set_throttle_out(int16_t throttle_out, bool angle_boost,double dcmRoll, double dcmPitch);

int16_t update_z_controller(AFCSTATUS sts, double position_z, double velocity_z, int16_t climb_rate,double dcmRoll, double dcmPitch,int16_t iThrottleHover);
//int16_t updateZController(AFCSTATUS sts, double position_z, double velocity_z, int16_t climb_rate,double dcmRoll, double dcmPitch,float altAccelInt,double *posTarZ,int16_t iThrottleHover);

#endif


