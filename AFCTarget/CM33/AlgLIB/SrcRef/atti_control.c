#include "atti_control.h"

#include "AFCGlobalVar.h"
#include "AFCBasicAPI.h"
#include "ahrs.h"
#include "matrix3f.h"
//#include "Sensor.h"
#include "caculateMotorsCmd.h"

float angle_kP[3];
PID pid_rate[3];

// 姿态角位置控制
void attitude_control(double roll_angle_ef, double pitch_angle_ef, double yaw_rate_ef,double fDcmAng[3],double fWz,double *fAngEfTarZ, double fRateBfTarget[3])
{
	  static Vector3f angle_bf_error; 
		float cos_roll, cos_pitch;
		float sin_roll, sin_pitch;
//	  float sin_yaw, cos_yaw;
	
//	  Vector3f angle_ef_target, rate_bf_target;

	// 求方向余弦
	  Matrix3f dcm;    //旋转矩阵
	  Matrix3f_from_euler(&dcm, fDcmAng[0], fDcmAng[1], fDcmAng[2]);   // 根据欧拉角求取旋转矩阵dcm
	
		float yaw_vector_x, yaw_vector_y;
		float length;

		yaw_vector_x = dcm.a.x;
		yaw_vector_y = dcm.b.x;
		length = sqrt(yaw_vector_x*yaw_vector_x+yaw_vector_y*yaw_vector_y);

		yaw_vector_x /= length;
		yaw_vector_y /= length;

//		sin_yaw = constrain_float(yaw_vector_y, -1.0, 1.0);
//		cos_yaw = constrain_float(yaw_vector_x, -1.0, 1.0);

		cos_pitch = sqrt(1 - (dcm.c.x * dcm.c.x));
		cos_roll = dcm.c.z / cos_pitch;
		cos_pitch = constrain_float(cos_pitch, 0, 1.0);
		cos_roll = constrain_float(cos_roll, -1.0, 1.0);

		sin_pitch = -dcm.c.x;
		sin_roll = dcm.c.y / cos_pitch;

    int32_t roll_sensor, pitch_sensor, yaw_sensor;
	// 将dcmAng中各个参数转换为RAD_TO_DEG_X100
		roll_sensor = fDcmAng[0]*RAD_TO_DEG_X100;
	  pitch_sensor = fDcmAng[1]*RAD_TO_DEG_X100;
	  yaw_sensor = fDcmAng[2]*RAD_TO_DEG_X100;
	  if(yaw_sensor < 0)	 yaw_sensor += 36000;
	
		Vector3f angle_ef_error; 
		
		//期望角度
		angle_ef_error.x = wrap_180_cd_float(roll_angle_ef - roll_sensor + g_RollSensor0);
		angle_ef_error.y = wrap_180_cd_float(pitch_angle_ef - pitch_sensor + g_PitchSensor0);     // V5.05.240823: 取消配平角
//		angle_ef_error.y = wrap_180_cd_float(pitch_angle_ef - 400 - pitch_sensor);  // V5.02.230915下午在奥佳华商务楼试飞调整修改：
																																								// 由于即使把遥控器俯仰杆微调量打到最大，无操纵时无人机仍然朝遥控者飘过来，
																																								//   因此在此控制指令中强行补一个4.00°俯仰指令。
																																								//   分析原因：由于电池+遥控接收机重量使得无人机尾部重量超重引起
		angle_ef_error.x = constrain_float(angle_ef_error.x, -2000, 2000);
		angle_ef_error.y = constrain_float(angle_ef_error.y, -2000, 2000);
		//航向角速度控制
		angle_ef_error.z = wrap_180_cd_float(*fAngEfTarZ - yaw_sensor);  // fAngEfTarZ期望的航向角
		angle_ef_error.z  = constrain_float(angle_ef_error.z, -3000, 3000);
		*fAngEfTarZ = angle_ef_error.z + yaw_sensor;
		*fAngEfTarZ += yaw_rate_ef * Control_Dt;
		*fAngEfTarZ = wrap_360_cd_float(*fAngEfTarZ);
		//对地坐标变换到机体坐标
		angle_bf_error.x = angle_ef_error.x - sin_pitch * angle_ef_error.z;
		angle_bf_error.y = cos_roll * angle_ef_error.y + sin_roll * cos_pitch * angle_ef_error.z;
		angle_bf_error.z = -sin_roll * angle_ef_error.y + cos_pitch * cos_roll * angle_ef_error.z;
		//姿态角度控制
		fRateBfTarget[0] = angle_kP[0] * angle_bf_error.x;
		fRateBfTarget[0] = constrain_float(fRateBfTarget[0], -14000, 14000);
		fRateBfTarget[1] = angle_kP[1] * angle_bf_error.y;
		fRateBfTarget[1] = constrain_float(fRateBfTarget[1], -14000, 14000);
		fRateBfTarget[2] = angle_kP[2] * angle_bf_error.z;
		fRateBfTarget[2] = constrain_float(fRateBfTarget[2], -9000, 9000);
		fRateBfTarget[0] += angle_bf_error.y * fWz;
		fRateBfTarget[1] += -angle_bf_error.x * fWz;
		//航向速度补偿
		fRateBfTarget[0] += - sin_pitch * yaw_rate_ef;
		fRateBfTarget[1] += sin_roll * cos_pitch * yaw_rate_ef;
		fRateBfTarget[2] += cos_pitch * cos_roll * yaw_rate_ef;
}

#define RP_MOVE_COUNT 3
void attitude_output_controller(AFCSTATUS sts,double fRateBfTarget[3],double fDcmRate[3], double fServoOut[3])
{
	Vector3f rate_bf_target,attiRateDegX100;
	
	static float store_last_roll_rate, store_last_pitch_rate;
	static float last_yaw_rate[RP_MOVE_COUNT];
	float roll_gyro_rate, pitch_gyro_rate, yaw_gyro_rate;

	rate_bf_target.x = fRateBfTarget[0];  rate_bf_target.y = fRateBfTarget[1];   rate_bf_target.z = fRateBfTarget[2];

	attiRateDegX100 = getAttiRateRad2DegX100(fDcmRate);
	roll_gyro_rate = attiRateDegX100.x;  pitch_gyro_rate = attiRateDegX100.y;  yaw_gyro_rate = attiRateDegX100.z;
//	int16_t *Roll_Servo_Out,int16_t *Pitch_Servo_Out,int16_t *Yaw_Servo_Out

	float rate_error;
	float p,i,d;  
	uint8_t k;
	
	rate_error = rate_bf_target.x - roll_gyro_rate;
	p = PID_get_p(&pid_rate[0], rate_error);
	i = pid_rate[0].integrator;
	if(!sts.bLimitRollPitch || ((i>0&&rate_error<0)||(i<0&&rate_error>0))) {
		i = PID_get_i(&pid_rate[0], rate_error, Control_Dt);
	}
	d = -pid_rate[0].kd*(roll_gyro_rate-store_last_roll_rate);
	store_last_roll_rate = roll_gyro_rate;

	//横滚输出
	fServoOut[0] = constrain_float((p+i+d), -5000.0f, 5000.0f)*0.1f;
	
	rate_error = rate_bf_target.y - pitch_gyro_rate;
	p = PID_get_p(&pid_rate[1], rate_error);
	i = pid_rate[1].integrator;
	if(!sts.bLimitRollPitch || ((i>0&&rate_error<0)||(i<0&&rate_error>0))) {
		i = PID_get_i(&pid_rate[1], rate_error, Control_Dt);
	}
	d = -pid_rate[1].kd*(pitch_gyro_rate-store_last_pitch_rate);
	store_last_pitch_rate = pitch_gyro_rate;
	//俯仰输出
	fServoOut[1] = constrain_float((p+i+d), -5000.0f, 5000.0f)*0.1f;
	
	rate_error = rate_bf_target.z - yaw_gyro_rate;
	p = PID_get_p(&pid_rate[2], rate_error);
	i = pid_rate[2].integrator;
	if(!sts.bLimitYaw || ((i>0&&rate_error<0)||(i<0&&rate_error>0))) {
		i = PID_get_i(&pid_rate[2], rate_error, Control_Dt);
	}
	for(k=0;k<(RP_MOVE_COUNT-1);k++) {
		last_yaw_rate[k] = last_yaw_rate[k+1];
	}
	last_yaw_rate[RP_MOVE_COUNT-1] = yaw_gyro_rate;
	d = -pid_rate[2].kd*(yaw_gyro_rate-last_yaw_rate[0]);
	//航向输出
	fServoOut[2] = constrain_float((p+i+d), -4000.0f, 4000.0f)*0.1f;
}

void setRateBfTarget(double fDcmWxyz[3],double dcmYaw,double *fAngEfTarZ, double fRateBfTarget[3])
{
	// fRateBfTarget设置机体坐标系下目标角速度
	fRateBfTarget[0] = fDcmWxyz[0]*RAD_TO_DEG_X100;
	fRateBfTarget[1] = fDcmWxyz[1]*RAD_TO_DEG_X100;
	fRateBfTarget[2] = fDcmWxyz[2]*RAD_TO_DEG_X100;
	
	double yaw_sensor = dcmYaw*RAD_TO_DEG_X100;
	if(yaw_sensor < 0) yaw_sensor += 36000;
	
	// fAngEfTarZ惯性坐标系下目标航向角
	*fAngEfTarZ = yaw_sensor;
}

/************************ (C) COPYRIGHT ACG co. *****END OF FILE****/
