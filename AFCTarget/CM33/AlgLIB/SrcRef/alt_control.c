#include "alt_control.h"
//#include "DPS310.h"
#include "ahrs.h"
#include "some_work.h"
#include "caculateMotorsCmd.h"
#include "LowPassFilter.h"
#include "atti_control.h"
#include "AFCBasicAPI.h"
#include "AFCGlobalVar.h"

#define Z_DELAY_NUM 10

float alt_pos_kP;
PID alt_rate;
PID alt_accel;
LowPassFilter vel_error_filter;
LowPassFilter accel_error_filter;
LowPassFilter baro_filter;

void init_alt_LPF(void)
{
	PID_set_d_lpf_alpha(&alt_accel, 20, Control_Dt);
	
	LPF_set_cutoff_frequency(&vel_error_filter, Control_Dt, 4.0f);
	LPF_set_cutoff_frequency(&accel_error_filter, Control_Dt, 2.0f);
	LPF_set_cutoff_frequency(&baro_filter, Control_Dt, 50.0f);
}

void altitude_update(AFCSTATUS sts,double fHeight,double fDcmRoll, double fDcmPitch, int16_t iThrottleOut,
                     	double *position_z, double *velocity_z, double *Throttle_Hover)
{
	int32_t roll_sensor, pitch_sensor;
	roll_sensor = fDcmRoll*RAD_TO_DEG_X100;  pitch_sensor = fDcmPitch*RAD_TO_DEG_X100;
	
	// 以下变量原来为本文件使用的全局变量，V5.03.20240420将其更改为函数内静态变量
	float position_z_error;
  static float position_z_correction, postion_z_history[Z_DELAY_NUM];
  static float position_z_base, position_z_i_err,accel_correction_hbf_z;

	static uint8_t alt_update_delay  = 0;
	float k1,k2,k3;
	float velocity_increase_z;
	float accel_ef_z;
	float height_slt_filter = 0;
	uint8_t i;
	
	k1 = 0.6f;
	k2 = 0.12f;
	k3 = 0.008f;
	
	accel_ef_z = (fGetDcmAccelEf(2) + GRAVITY_MSS)*100.0f;
//	accel_ef_z = (dcm_accel_ef.z + GRAVITY_MSS)*100.0f;
	accel_ef_z = -accel_ef_z;
	
	if(!sts.bSystemInitSts) return;
	
	height_slt_filter = LPF_apply(&baro_filter, fHeight);
	
	if(alt_update_delay < 50) {
		alt_update_delay++;
	} 
	
	if(!sts.bArmedEnable) {
		position_z_base = height_slt_filter;
		*position_z = height_slt_filter;
		accel_correction_hbf_z = -accel_ef_z;
		*velocity_z = 0;
		position_z_i_err = 0;
	}
	
	position_z_error = height_slt_filter - (postion_z_history[0] + position_z_correction);
	accel_correction_hbf_z += position_z_error * k3 * Control_Dt;
	*velocity_z += position_z_error * k2  * Control_Dt;
	position_z_correction += position_z_error * k1  * Control_Dt;
	
	velocity_increase_z = (accel_ef_z + accel_correction_hbf_z) * Control_Dt;
	position_z_base += (*velocity_z + velocity_increase_z*0.5f)*Control_Dt;
	*position_z = position_z_base + position_z_correction + position_z_i_err*0.18f;
	position_z_i_err += (height_slt_filter - (*position_z))*Control_Dt;  //再次融合真实值,提高定高精度
	*velocity_z += velocity_increase_z;

	for(i=0;i<(Z_DELAY_NUM-1);i++) {
		postion_z_history[i] = postion_z_history[i+1];
	}
	postion_z_history[Z_DELAY_NUM-1] = position_z_base;
	
	//油门基准更新
	if(iThrottleOut > g_iIdlingSpeedWidth && fabs(*velocity_z) < 60 &&
			int32_abs(roll_sensor) < 500 && int32_abs(pitch_sensor) < 500) {
				*Throttle_Hover = (*Throttle_Hover)*0.99f + iThrottleOut*0.01f;
	}
}

double pos_target_z;
void init_takeoff(double position_z, int16_t iThrottleOut)
{	
	pos_target_z = position_z;
	alt_accel.integrator = iThrottleOut - 300;
	
	setMotorSlowStart(iThrottleOut);
}

void set_current_alt_to_target_alt(double position_z) { pos_target_z = position_z; }

//float accel_z_cms = 300.0f;          
//float leash_down_z = 100.0f;         
//float leash_up_z = 300.0f;  
float vel_last_z;

int16_t update_z_controller(AFCSTATUS sts, double position_z, double velocity_z, int16_t climb_rate,double dcmRoll, double dcmPitch,int16_t iThrottleHover)
{
//	int32_t roll_sensor, pitch_sensor;
//	roll_sensor = dcmRoll*RAD_TO_DEG_X100;     pitch_sensor = dcmPitch * RAD_TO_DEG_X100;
	
	float accel_feedforward_z,accel_target_z;
	float pos_error_z,vel_error_z,accel_error_z;
	float linear_distance;
	float p,i,d;
	
	if(climb_rate<0) climb_rate *= 0.5;
	
	if((climb_rate<0 && !sts.bLimitThrottleLower) || (climb_rate>0 && !sts.bLimitThrottleUpper)) {
     pos_target_z += climb_rate * Control_Dt;
  }
	
	pos_error_z = pos_target_z - position_z;
	
	if (pos_error_z > leash_up_z) {
		pos_target_z = position_z + leash_up_z;
		pos_error_z = leash_up_z;
	}
	if (pos_error_z < -leash_down_z) {
		pos_target_z = position_z - leash_down_z;
		pos_error_z = -leash_down_z;
	}
	
	double vel_target_z;
	linear_distance = accel_z_cms/(2.0f*alt_pos_kP*alt_pos_kP);
	if (pos_error_z > 2*linear_distance ) {
		vel_target_z = sqrtf(2.0f*accel_z_cms*(pos_error_z-linear_distance));
  }else if (pos_error_z < -2.0f*linear_distance) {
		vel_target_z = -sqrtf(2.0f*accel_z_cms*(-pos_error_z-linear_distance));
	}else{
		vel_target_z = alt_pos_kP*pos_error_z;
	}
	vel_target_z = constrain_float(vel_target_z,-150.f,200.0f);
	
	accel_feedforward_z = (vel_target_z - vel_last_z)/Control_Dt;
	vel_last_z = vel_target_z;
	
	vel_error_z = LPF_apply(&vel_error_filter, vel_target_z - velocity_z);
	
	p = alt_rate.kp * vel_error_z;
	accel_target_z = constrain_int32(p+accel_feedforward_z,-30000,30000);
	
	float z_accel_meas = -(fGetDcmAccelEf(2) + GRAVITY_MSS) * 100.0f;  
//	float z_accel_meas = -(dcm_accel_ef.z + GRAVITY_MSS) * 100.0f;  
	accel_error_z = LPF_apply(&accel_error_filter, constrain_float(accel_target_z-z_accel_meas, -30000, 30000));
	
	p = alt_accel.kp * accel_error_z;
  i = alt_accel.integrator;
	if ((!sts.bLimitThrottleLower && !sts.bLimitThrottleUpper) || (i>0&&accel_error_z<0) || (i<0&&accel_error_z>0)) {
		i = PID_get_i(&alt_accel, accel_error_z, Control_Dt);
	}
	d = PID_get_d(&alt_accel, accel_error_z, Control_Dt);

	// 由于set_throttle_out中使用了roll_sensor、pitch_sensor，此处应改写为下句L184行，否则会出现油门及巡航油门值提升不上
//  int16_t iThrottleOut = set_throttle_out((int16_t)(p+i+d+iThrottleHover), true,roll_sensor, pitch_sensor); 
  int16_t iThrottleOut = set_throttle_out((int16_t)(p+i+d+iThrottleHover), true, dcmRoll, dcmPitch);
	return iThrottleOut;
}


// 由于目前代码中未使用AFCToolbox的altThrottleAPI模块(调用函数altThrottleStep)，因此暂时将其注释
//int16_t updateZController(AFCSTATUS sts, double position_z, double velocity_z, int16_t climb_rate,double dcmRoll, double dcmPitch,float altAccelInt,double *posTarZ,int16_t iThrottleHover)
//{
////	int32_t roll_sensor, pitch_sensor;
////	roll_sensor = dcmRoll*RAD_TO_DEG_X100;     pitch_sensor = dcmPitch * RAD_TO_DEG_X100;
//	
//	float accel_feedforward_z,accel_target_z;
//	float pos_error_z,vel_error_z,accel_error_z;
//	float linear_distance;
//	float p,i,d;
//	
//	if(climb_rate<0) climb_rate *= 0.5;
//	
//	if((climb_rate<0 && !sts.bLimitThrottleLower) || (climb_rate>0 && !sts.bLimitThrottleUpper)) {
//     *posTarZ += climb_rate * Control_Dt;
//  }
//	
//	pos_error_z = *posTarZ - position_z;
//	
//	if (pos_error_z > leash_up_z) {
//		*posTarZ = position_z + leash_up_z;
//		pos_error_z = leash_up_z;
//	}
//	if (pos_error_z < -leash_down_z) {
//		*posTarZ = position_z - leash_down_z;
//		pos_error_z = -leash_down_z;
//	}
//	
//	double vel_target_z;
//	linear_distance = accel_z_cms/(2.0f*alt_pos_kP*alt_pos_kP);
//	if (pos_error_z > 2*linear_distance ) {
//		vel_target_z = sqrtf(2.0f*accel_z_cms*(pos_error_z-linear_distance));
//  }else if (pos_error_z < -2.0f*linear_distance) {
//		vel_target_z = -sqrtf(2.0f*accel_z_cms*(-pos_error_z-linear_distance));
//	}else{
//		vel_target_z = alt_pos_kP*pos_error_z;
//	}
//	vel_target_z = constrain_float(vel_target_z,-150.f,200.0f);
//	
//	accel_feedforward_z = (vel_target_z - vel_last_z)/Control_Dt;
//	vel_last_z = vel_target_z;
//	
//	vel_error_z = LPF_apply(&vel_error_filter, vel_target_z - velocity_z);
//	
//	#if 1
//	p = alt_rate.kp * vel_error_z;
//	accel_target_z = constrain_int32(p+accel_feedforward_z,-30000,30000);
//	
//	float z_accel_meas = -(fGetDcmAccelEf(2) + GRAVITY_MSS) * 100.0f;  
////	float z_accel_meas = -(dcm_accel_ef.z + GRAVITY_MSS) * 100.0f;  
//	accel_error_z = LPF_apply(&accel_error_filter, constrain_float(accel_target_z-z_accel_meas, -30000, 30000));
//	
//	p = alt_accel.kp * accel_error_z;
////  i = alt_accel.integrator;
//  i = altAccelInt;
//	if ((!sts.bLimitThrottleLower && !sts.bLimitThrottleUpper) || (i>0&&accel_error_z<0) || (i<0&&accel_error_z>0)) {
//		i = PID_get_i(&alt_accel, accel_error_z, Control_Dt);
//	}
//	d = PID_get_d(&alt_accel, accel_error_z, Control_Dt);
//	#else
//	p = alt_rate.kp * vel_error_z;
//  i = alt_rate.integrator;
//	if ((!Limit_Throttle_Lower && !Limit_Throttle_Upper) || (i>0&&vel_error_z<0) || (i<0&&vel_error_z>0)) {
//		i = PID_get_i(&alt_rate, vel_error_z, Control_Dt);
//	}
//	d = alt_rate.kd*(dcm_accel_ef.z + GRAVITY_MSS) * 100.0f;
//	#endif

//	// 由于set_throttle_out中使用了roll_sensor、pitch_sensor，此处应改写为下句L184行，否则会出现油门及巡航油门值提升不上
////  int16_t iThrottleOut = set_throttle_out((int16_t)(p+i+d+iThrottleHover), true,roll_sensor, pitch_sensor); 
//  int16_t iThrottleOut = set_throttle_out((int16_t)(p+i+d+iThrottleHover), true, dcmRoll, dcmPitch);
//	return iThrottleOut;
//}

int16_t set_throttle_out(int16_t throttle_out, bool angle_boost,double dcmRoll, double dcmPitch)
{
	int16_t iThrottleOut;
	if(angle_boost) {
		float temp,cos_pitch,cos_roll;
		cos_roll = cos(dcmRoll);                cos_pitch = cos(dcmPitch);   
		
		int32_t roll_sensor, pitch_sensor;
		roll_sensor = dcmRoll*RAD_TO_DEG_X100;  pitch_sensor = dcmPitch*RAD_TO_DEG_X100;
		
		temp = constrain_float(cos_pitch * cos_roll, 0.5f, 1.0f);
		temp = constrain_float(9000-max(int32_abs(roll_sensor),int32_abs(pitch_sensor)), 0, 3000) / (3000 * temp);
		
		iThrottleOut = constrain_float((float)(throttle_out- g_iIdlingSpeedWidth) * temp + g_iIdlingSpeedWidth, g_iIdlingSpeedWidth, 1000);
  }else{
		iThrottleOut = throttle_out;
  }
	return iThrottleOut;
}
