#include "fc_math.h"

//uint8_t control_dt_scale = 1;

float constrain_float(float amt, float low, float high) 
{
	return ((amt)<(low)?(low):((amt)>(high)?(high):(amt)));
}

float wrap_180_cd_float(float angle)
{
	if(angle > 54000.0f || angle < -54000.0f) {
     // for large numbers use modulus
     angle = fmod(angle,36000.0f);
  }
  if (angle > 18000.0f) { angle -= 36000.0f; }
  if (angle < -18000.0f) { angle += 36000.0f; }
  return angle;
}

float wrap_360_cd_float(float angle)
{
    if (angle >= 72000.0f || angle < -36000.0f) {
        // for larger number use fmodulus
        angle = fmod(angle, 36000.0f);
    }
    if (angle >= 36000.0f) angle -= 36000.0f;
    if (angle < 0.0f) angle += 36000.0f;
    return angle;
}

int16_t constrain_int16(int16_t amt, int16_t low, int16_t high) 
{
	return ((amt)<(low)?(low):((amt)>(high)?(high):(amt)));
}

int32_t constrain_int32(int32_t amt, int32_t low, int32_t high) {
	return ((amt)<(low)?(low):((amt)>(high)?(high):(amt)));
}

int32_t int32_abs(int32_t value)
{
	if(value < 0) value = -value;
	return value;
}
int16_t int16_abs(int16_t value)
{
	if(value < 0) value = -value;
	return value;
}

float sq(float v) {
	return v*v;
}

float pythagorous2(float a, float b) {
	return sqrtf(sq(a)+sq(b));
}

float safe_sqrt(float v)
{
    float ret = sqrtf(v);
    if (isnan(ret)) {
        return 0;
    }
    return ret;
}

float fast_atan(float v)
{
    float v2 = v*v;
    return (v*(1.6867629106f + v2*0.4378497304f)/(1.6867633134f + v2));
}

#define FAST_ATAN2_PIBY2_FLOAT  1.5707963f
float fast_atan2(float y, float x)
{
   if (x == 0.0f) {
       if (y > 0.0f) {
           return FAST_ATAN2_PIBY2_FLOAT;
       }
       if (y == 0.0f) {
           return 0.0f;
       }
       return -FAST_ATAN2_PIBY2_FLOAT;
   }
   float atan;
   float z = y/x;
   if (fabs( z ) < 1.0f) {
       atan = z / (1.0f + 0.28f * z * z);
       if (x < 0.0f) {
           if (y < 0.0f) {
               return atan - PI;
           }
           return atan + PI;
       }
   } else {
       atan = FAST_ATAN2_PIBY2_FLOAT - (z / (z * z + 0.28f));
       if (y < 0.0f) {
           return atan - PI;
       }
   }
   return atan;
}

float wrap_PI(float angle_in_radians)
{
    if (angle_in_radians > 10*PI || angle_in_radians < -10*PI) {
        // for very large numbers use modulus
        angle_in_radians = fmodf(angle_in_radians, 2*PI);
    }
    while (angle_in_radians > PI) angle_in_radians -= 2*PI;
    while (angle_in_radians < -PI) angle_in_radians += 2*PI;
    return angle_in_radians;
}

float safe_asin(float v)
{
	if (isnan(v)) {
		return 0.0;
	}
	if (v >= 1.0f) {
		return PI/2;
	}
	if (v <= -1.0f) {
		return -PI/2;
	}
	return asinf(v);
}

void rotation_xy(int16_t *x, int16_t *y, uint16_t angle)
{
	float tmp;
	
	switch(angle)
	{
		case 90:
			tmp = *x; *x = -*y; *y = tmp;
			break;
		case 180:
			*x = -*x; *y = -*y;
			break;
		case 270:
			tmp = *x; *x = *y; *y = -tmp;
			break;
		case 1180:
			*y = -*y;
	}
}

void params_up_checksum(uint8_t *data, uint8_t len, uint8_t *ck) 
{
    while (len--) {
        *ck ^= *data;
        data++;
    }
}

float PID_get_p(PID *pid, float error)
{
	return error * pid->kp;
}

float PID_get_i(PID *pid, float error, float dt)
{
	if((pid->ki != 0) && (dt != 0)) {
     pid->integrator += ((float)error * pid->ki) * dt;
		 //积分限幅
     if(pid->integrator < -pid->imax) {
        pid->integrator = -pid->imax;
     } else if (pid->integrator > pid->imax) {
        pid->integrator = pid->imax;
     }
     return pid->integrator;
  }
  return 0;
}

float PID_get_d(PID *pid, float input, float dt)
{
	float derivative;
	
	if((pid->kd != 0) && (dt != 0)) {
		derivative = (input - pid->last_input) / dt;

		//一阶低通滤波
    derivative = pid->last_derivative + pid->d_lpf_alpha * (derivative - pid->last_derivative);

    pid->last_input             = input;
    pid->last_derivative    = derivative;

    return pid->kd * derivative;
  }
  return 0;
}

void PID_set_d_lpf_alpha(PID *pid, int16_t cutoff_frequency, float time_step)
{    
    // calculate alpha
    float rc = 1/(2*PI*cutoff_frequency);
    pid->d_lpf_alpha = time_step / (time_step + rc);
}

