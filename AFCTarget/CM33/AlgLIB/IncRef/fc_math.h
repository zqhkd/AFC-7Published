#ifndef __FC_MATH_H
#define __FC_MATH_H

#include "stm32h7xx_hal.h"

#define PI 3.141592653589793f

#define DEG_TO_RAD 0.017453292519943295769236907684886f
#define RAD_TO_DEG 57.295779513082320876798154814105f
#define RAD_TO_DEG_X100 5729.5779513082320876798154814105f

#define ToRad(x) (x*DEG_TO_RAD)

#define GRAVITY_MSS 9.80665f       // acceleration due to gravity in m/s/s
#define GRAVITY_MSS_X100 980.665f
#define gravity_squared 96.17038f  // GRAVITY_MSS*GRAVITY_MSS

#define LATLON_TO_M  0.01113195f
#define LATLON_TO_CM 1.113195f

#define max(a,b) ((a)>(b)?(a):(b))
#define min(a,b) ((a)<(b)?(a):(b))

//extern uint8_t control_dt_scale;

float constrain_float(float amt, float low, float high);
float wrap_180_cd_float(float angle);
float wrap_360_cd_float(float angle);
int16_t constrain_int16(int16_t amt, int16_t low, int16_t high);
int32_t constrain_int32(int32_t amt, int32_t low, int32_t high);
int32_t int32_abs(int32_t value);
int16_t int16_abs(int16_t value);
float sq(float v);
float pythagorous2(float a, float b);
float safe_sqrt(float v);
float fast_atan(float v);
float fast_atan2(float y, float x);
float wrap_PI(float angle_in_radians);
float safe_asin(float v);

void rotation_xy(int16_t *x, int16_t *y, uint16_t angle);
void params_up_checksum(uint8_t *data, uint8_t len, uint8_t *ck);

#define _d_lpf_alpha  0.556864f

typedef struct
{
	float kp,ki,kd,imax;
	float integrator;                          
  float last_input;                     
  float last_derivative;  
	float d_lpf_alpha;
} PID;

float PID_get_p(PID *pid, float error);
float PID_get_i(PID *pid, float error, float dt);
float PID_get_d(PID *pid, float input, float dt);
void PID_set_d_lpf_alpha(PID *pid, int16_t cutoff_frequency, float time_step);

#endif


