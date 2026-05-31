#include "caculateMotorsCmd.h"

#include "AFCGlobalVar.h"
#include "some_work.h"
#include "PwmOut.h"
#include "fc_math.h"
#include "AFCAPI.h"
#include "AFCBasicAPI.h"

//电机控制输出量
int16_t Roll_Servo_Out, Pitch_Servo_Out, Yaw_Servo_Out, Throttle_Out;
//int16_t Motor_PWM_1, Motor_PWM_2, Motor_PWM_3, Motor_PWM_4;
//int16_t Throttle_Hover = 350; //油门基准

extern int16_t Max_Throttle, iEscPWMWidthRange;  //最大油门, 电调PWM值调节范围

bool Motor_slow_start = false;  // 电机低速启动标志
bool Limit_Throttle_Lower; //达到最小油门标志位
bool Limit_Throttle_Upper; //达到最大油门标志位
bool Limit_Roll_Pitch;     //横滚及俯仰限制标志位
bool Limit_Yaw;            //航偏角限制标志位

void Motors_Output_Min(void)
{
	Limit_Throttle_Lower = true;
	Limit_Throttle_Upper = false;
	Limit_Roll_Pitch = true;
	Limit_Yaw = true;

	Roll_Servo_Out = 0;	Pitch_Servo_Out = 0;	Yaw_Servo_Out = 0;
}

void Motors_PWM_Arrange(int16_t rpy_out[MOTORS_CHANEL_NUM])
{
	int16_t output_max,i;

	Limit_Throttle_Lower = 0;
	Limit_Throttle_Upper = 0;
	Limit_Yaw = 0;

	if(Throttle_Out <= 0) {
		Throttle_Out = 0;
		Limit_Throttle_Lower = true;
	}
	if(Throttle_Out >= Max_Throttle) {
		Throttle_Out = Max_Throttle;
		Limit_Throttle_Upper = true;
	}

	rpy_out[0] = - Roll_Servo_Out + Pitch_Servo_Out;
	rpy_out[1] = Roll_Servo_Out - Pitch_Servo_Out;
	rpy_out[2] = Roll_Servo_Out + Pitch_Servo_Out;
	rpy_out[3] = - Roll_Servo_Out - Pitch_Servo_Out;

	//加偏航角控制
	if(Yaw_Servo_Out>260) {
	    rpy_out[0] += 260;
			rpy_out[1] += 260;
			rpy_out[2] -= 260;
			rpy_out[3] -= 260;
			Limit_Yaw = true;
	} else if(Yaw_Servo_Out<-260) {
	    rpy_out[0] -= 260;
			rpy_out[1] -= 260;
			rpy_out[2] += 260;
			rpy_out[3] += 260;
			Limit_Yaw = true;
	} else {
        rpy_out[0] += Yaw_Servo_Out;
        rpy_out[1] += Yaw_Servo_Out;
        rpy_out[2] -= Yaw_Servo_Out;
        rpy_out[3] -= Yaw_Servo_Out;
	}

  rpy_out[0] += Throttle_Out;
	rpy_out[1] += Throttle_Out;
	rpy_out[2] += Throttle_Out;
	rpy_out[3] += Throttle_Out;

	// 找出rpy_out[0]--[3]中的最大值
	output_max = rpy_out[0];
	for(i=1;i < MOTORS_CHANEL_NUM;i++) {
        if(rpy_out[i]>output_max) output_max = rpy_out[i];
	}

	// 最大输出值超范围时，将所有输出往下降
	if(output_max > iEscPWMWidthRange) {
			Limit_Throttle_Upper = 1;
			rpy_out[0] += (iEscPWMWidthRange - output_max);
			rpy_out[1] += (iEscPWMWidthRange - output_max);
			rpy_out[2] += (iEscPWMWidthRange - output_max);
			rpy_out[3] += (iEscPWMWidthRange - output_max);
	}
//	if(rpy_out[0]<100) rpy_out[0] = 100;
//	if(rpy_out[1]<100) rpy_out[1] = 100;
//	if(rpy_out[2]<100) rpy_out[2] = 100;
//	if(rpy_out[3]<100) rpy_out[3] = 100;
	
	// V5.05.240911: 更改固定值100为怠速油门值
	for(i=0;i < MOTORS_CHANEL_NUM;i++) {
		if(rpy_out[i] < g_iIdlingSpeedWidth) rpy_out[i] = g_iIdlingSpeedWidth;
	}
}

void Set_Motors_Output(bool bLandComplete,int16_t iMotorsPWM[MOTORS_CHANEL_NUM])
{
	uint8_t i;
	if(bLandComplete) {
		for(i=0;i < MOTORS_CHANEL_NUM;i++) iMotorsPWM[i] = Throttle_Out;
	} 
	else {
		Motors_PWM_Arrange(iMotorsPWM);
	}
	
	uint8_t iChNo;
  for(i=0;i<MOTORS_CHANEL_NUM;i++){
		iChNo = giChNo[i];
		iMotorsPWM[i] = constrain_int16(iMotorsPWM[i] + g_iMinPW[iChNo], g_iMinPW[iChNo] + g_iIdlingSpeedWidth, g_iMaxPW[iChNo]);
	}
}

void Motors_Slow(AFCSTATUS sts,int16_t iThrottleOut)
{
	if(!sts.bMotorSlowStart) return;

	Max_Throttle += 3;
	if(Max_Throttle >= iThrottleOut) {
		Max_Throttle = iEscPWMWidthRange;
		Motor_slow_start = false;
	}
}

void outMotorsPWM(AFCSTATUS sts,double fServoOut[3], double *fThrottleOut, int16_t iMotorsPWM[MOTORS_CHANEL_NUM])
{
	  Roll_Servo_Out = (int16_t)fServoOut[0];	  Pitch_Servo_Out = (int16_t)fServoOut[1];    Yaw_Servo_Out  =(int16_t) fServoOut[2];   Throttle_Out = (int16_t)(*fThrottleOut);
	  
	  Motors_Slow(sts,Throttle_Out);

	  if(sts.bArmedEnable)
		     Set_Motors_Output(sts.bLandComplete,iMotorsPWM);   // ThrottleOut值可能会改变，而fServoOut不会变
	  else{
			   for(uint8_t i=0;i<MOTORS_CHANEL_NUM;i++) iMotorsPWM[i] = g_iMinPW[giChNo[i]];
		     Motors_Output_Min();   // RollServoOut、PitchServoOut、YawServoOut会改变至0，ThrottleOut不变
		}
		
	  fServoOut[0] = (double)Roll_Servo_Out;	  fServoOut[1] = (double)Pitch_Servo_Out;    fServoOut[2] = (double)Yaw_Servo_Out;		*fThrottleOut = (double)Throttle_Out;
}

void motorsOutByEscVal(double fMotorsESCVal[MOTORS_CHANEL_NUM])
{
	  setEscVal(g_iM1CHANEL,fMotorsESCVal[0]);
	  setEscVal(g_iM2CHANEL,fMotorsESCVal[1]);
	  setEscVal(g_iM3CHANEL,fMotorsESCVal[2]);
	  setEscVal(g_iM4CHANEL,fMotorsESCVal[3]);
}

void getMotorsAFCSts(AFCSTATUS *sts)
{
		sts->bLimitRollPitch = Limit_Roll_Pitch;
		sts->bLimitYaw = Limit_Yaw;
		sts->bLimitThrottleLower =	Limit_Throttle_Lower; 
		sts->bLimitThrottleUpper = Limit_Throttle_Upper; 
		sts->bMotorSlowStart = Motor_slow_start;
}

void initMotorsAFCSts(void)
{
	Limit_Roll_Pitch = false;		    Limit_Yaw = false;
	Limit_Throttle_Lower = false;		Limit_Throttle_Upper=false; 
	Motor_slow_start=false;
}

void setMotorSlowStart(int16_t iThrottleOut)
{
	Motor_slow_start = true; //缓慢起飞
  Max_Throttle = iThrottleOut;    // 将当前油门设置为最大油门
}

/************************ (C) COPYRIGHT ACG co. ********END OF FILE****/
