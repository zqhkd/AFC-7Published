#include "some_work.h"
#include "Sensor.h"     // level_space_check函数在L75处，需要访问gyro_offset
#include "vector3f.h"
#include "ahrs.h"
#include "calibration.h"
#include "SbusRC.h"
#include "fc_math.h"
#include "caculateMotorsCmd.h"
#include "DPS310.h"
#include "AFCGlobalVar.h"
#include "flyMode.h"

bool System_Init_Status = false;

bool Land_Complete = true;  //是否着陆标志位
bool Auto_Armed_Enable = false; //自动上锁标志位
bool Armed_Enable = false;  //解锁上锁使能

bool dcm_fast_ground_gains = true;

bool bRCRollLT1000,bRCPitchLT1000;
bool bRCThrottleEQ0;
bool bRCh0CmdGT1600,bRCCh1CmdGT1600;
bool bRCCh2CmdLT1400,bRCCh3CmdLT1400;

uint16_t iGetRCInput(float val)
{
	 uint16_t iVal;
	 iVal =(uint16_t) (1500+(val-992)/1.6f);
	 return iVal;
}


// 遥控器原始参数channel_raw值范围为：中值0x400(1024)、最小0x160(352)、最大0x69F(1695)
// 函数get_real_rc_intput将原始值channel_raw转换到1000--2000范围的RC值。
uint16_t get_real_rc_input(uint16_t value)
{
//	  return (uint16_t)(1000+(value-352)*(1000/(1695-352)));
	  return (uint16_t)(737.9f+value*0.7446f);
}


#define INPUT_ANGLE_MAX 1500.0f
// value取值范围为1000--2000，则该返回值为-1500 -- +1500，对应-15°至15°
int16_t Get_RC_Output(uint16_t value)
{
	float temp_value;

//	if(g_UavFcsParam.UavPara.FlyMode != POS_NAVI_MODE){ 
//			if(value > 1550)
//				temp_value = (float)(value-1550)/450.0f * INPUT_ANGLE_MAX;       
//			else if(value < 1450)
//				temp_value = (float)(value-1450)/450.0f * INPUT_ANGLE_MAX;
//			else
//				temp_value = 0.0f;
//	}
//	else
		temp_value = (float)(value-1500)/500.0f * INPUT_ANGLE_MAX;
	
	return (int16_t)temp_value;
}
#define VELOCITY_Z_MAX 250
// 将油门指令值转换为爬升率，油门指令值为0 -- 1000, 而爬升率为 -250 -- +250(VELOCITY_Z_MAX)
int16_t Get_Climb_Rate(int16_t value)
{
	int16_t desired_rate = 0;

  if(value < 400) {
     desired_rate = (int32_t)VELOCITY_Z_MAX * (value-400) / 400;
  }else if (value > 600) {
     desired_rate = (int32_t)VELOCITY_Z_MAX * (value-600) / 400;
  }else{
     desired_rate = 0;
  }

	return desired_rate;
}

void setRCLimitFlag(uint16_t iRCInput[4],int16_t RCServCmd[3],int16_t RCThrottle)
{
	bRCRollLT1000  = (int32_abs(RCServCmd[0]) < 1000);
	bRCPitchLT1000 = (int32_abs(RCServCmd[1]) < 1000);
  bRCThrottleEQ0 = (RCThrottle == 0);
	
	bRCh0CmdGT1600  = (iRCInput[0] > 1600);
	bRCCh1CmdGT1600 = (iRCInput[1] > 1600);
  bRCCh2CmdLT1400 = (iRCInput[2] < 1400);
	bRCCh3CmdLT1400 = (iRCInput[3] < 1400);
}


void setRcLimitFlg(bool bRCRoll,bool bRCPitch,bool bRCThrottle,bool bRCCh3,bool bRCCh2,bool bRCCh1,bool bRCCh0)
{
	bRCRollLT1000  = bRCRoll;	bRCPitchLT1000 = bRCPitch;  bRCThrottleEQ0 = bRCThrottle;
	
	bRCh0CmdGT1600  = bRCCh0;	bRCCh1CmdGT1600 = bRCCh1;  bRCCh2CmdLT1400 = bRCCh2;	bRCCh3CmdLT1400 = bRCCh3;
}

// simulink接口读取的sbuscmd值范围为滚转ch1、俯仰2、偏航4为-100--100，而ch3为油门通道，其值为0--100
// 该函数处理后，可将原始通道指令fChanelRaw值(0x160(352)--0x69F(1695))转换为iRCInput值(1000--2000)
//    最后将其转换为RC_Roll、RC_Pitch、RC_Yaw角度指令值和RCThrottle油门指令、RCClimbRate爬升率指令，其中：
//          滚转和俯仰指令范围为-1500 -- + 1500, 而偏航角指令-10500 -- 10500
//          油门指令范围为0 -- 1000, 而航点模式下的爬升率指令为-250 -- 250
void get_rccmd_simulink_value(double fChanelRaw[4],int16_t RCServCmd[3],int16_t *RCThrottle,int16_t *RCClimbRate)
{
	uint16_t iRCInput[4];
//	for(uint8_t i = 0; i < 4; i++) iRCInput[i] = iGetRCInput(fChanelRaw[i]);  // 将原始信号转换为1100~1940之间的数值
	for(uint8_t i = 0; i < 4; i++) iRCInput[i] = get_real_rc_input(fChanelRaw[i]);  // V5.04.240623P 将原始信号转换为1000~2000之间的数值
	
	RCServCmd[0] = Get_RC_Output(iRCInput[0]);   // 滚转通道，对应原来的RC_Roll，角度范围为-1500 -- + 1500，实际对应INPUT_ANGLE_MAX
	RCServCmd[1] = Get_RC_Output(iRCInput[1]);   // 俯仰通道，对应原来的RC_Pitch，角度范围为-1500 -- + 1500
	RCServCmd[2] = Get_RC_Output(iRCInput[3])*7;  // 偏航通道，对应原来的RC_Yaw，角度范围为-10500 -- + 10500
	
	if(iRCInput[2] < 1100) *RCThrottle = 0;           // 油门通道，对应原来的RC_Throttle
	else if(iRCInput[2] > 1900) *RCThrottle = 1000;
	else *RCThrottle = (iRCInput[2]-1100)*1.25;       // RCThrottle油门遥控值转换为0--1000之间的数值

	if((g_UavFcsParam.UavPara.FlyMode==ALTITUDE_HOLD_MODE) || (g_UavFcsParam.UavPara.FlyMode == POS_NAVI_MODE))
		*RCClimbRate = Get_Climb_Rate(*RCThrottle);   // 定高飞行模式，通过计算爬升率设置油门大小。对应-250--+250，实际对应VELOCITY_Z_MAX
	else
		*RCClimbRate = *RCThrottle;     // 普通稳定模式，直接用油门控制控制高度

//  设置相关标志	
	setRCLimitFlag(iRCInput,RCServCmd,*RCThrottle);
}

void level_space_check(Vector3f Wxyz, Vector3f Axyz)
{
	static uint8_t level_space_ready = 0;
	static uint8_t level_space_ready_count = 0;
	
	if(!System_Init_Status) {
		if(fabs(Axyz.x) < 3.0f && fabs(Axyz.y) < 3.0f && Axyz.z < -0.8f     // V5.05.240817: Axyz取消乘以-1操作后，水平放置时Az变为-g, 无人机翻转判据需更改
//		if(fabs(Axyz.x) < 3.0f && fabs(Axyz.y) < 3.0f && Axyz.z > 0.8f   // 按前右下体系处理, 水平状态下az恒大于0，V5.04.240623
			 && fabs(Wxyz.x) < 0.1f && fabs(Wxyz.y) < 0.1f && fabs(Wxyz.y) < 0.1f) {    
			level_space_ready = 1;
		} else {
			level_space_ready = 0;
			Vector3f_Zero(&gyro_offset[0]);       // 注意：将ICM42688偏置清0
			Vector3f_Zero(&gyro_offset[1]);       // 注意：将ICM20602偏置清0，AFC-5V5.03.240616添加
		}

		if(level_space_ready) {
			level_space_ready_count++;
			if(level_space_ready_count == 50) {
				Start_Level_Cal();  //放置水平，开始水平校准
				dcm_fast_ground_gains = true;  //姿态解算进入快速模式
			} else if(level_space_ready_count == 130) {
				System_Init_Status = true;  //上电初始化完毕
			}
		} else {
			level_space_ready_count = 0;
		}
	}
}

void Armed_Enable_Done(int32_t roll_sensor, int32_t pitch_sensor)
{
    //上电未初始化完成不解锁
    if(!System_Init_Status) return;
	
		//初始倾角不能超过20度
    if(int32_abs(roll_sensor) > 2000 || int32_abs(pitch_sensor) > 2000) return;

		resetDPS_TP = false;  //高度复位取消
    Armed_Enable = true; //解锁
	
	  Auto_Armed_Enable = false; // 自动上锁标志位,V5.05.241126添加

  	Motors_Output_Min();

    //姿态解算进入正常模式
    dcm_fast_ground_gains = false;
}

void Disarmed_Done(void)
{
    Land_Complete = true;
    Armed_Enable = false;
		resetDPS_TP = true;  //高度复位打开
	
	  Auto_Armed_Enable = true; // 自动上锁标志位,V5.05.241126添加

    //姿态解算进入快速模式
    dcm_fast_ground_gains = true;
}

void Motors_Armed_Check(int32_t roll_sensor, int32_t pitch_sensor)
{
	static uint16_t arming_counter = 0;
	
  if((g_UavFcsParam.UavPara.FlyMode==ALTITUDE_HOLD_MODE) || (g_UavFcsParam.UavPara.FlyMode == POS_NAVI_MODE)){
		static uint16_t arming_counter2 = 0;
		static uint8_t armed_rc_status = 0;
		
		if(bRCh0CmdGT1600 && bRCCh1CmdGT1600 && bRCCh2CmdLT1400 && bRCCh3CmdLT1400) {
			if(arming_counter <= 5) {
				arming_counter++;
			} else {
				if(!armed_rc_status) {
					if(!Armed_Enable) {
							Armed_Enable_Done(roll_sensor,pitch_sensor);
					} else {
							if(Land_Complete) Disarmed_Done();
					}
					armed_rc_status = 1;
					arming_counter2 = 10;
				}
			}
		}else {
			arming_counter = 0;
			if(arming_counter2>0) arming_counter2--;
			else armed_rc_status = 0;
		}
	}
  else{
		 static bool bFlg = true;
		 if(((channel_raw[6] > 1400) && bFlg)||((channel_raw[6] < 800) && !bFlg)){
				if(arming_counter <= 10) {
					arming_counter++;
				}
				else{
					 if((!Armed_Enable) && bFlg) {
							Armed_Enable_Done(roll_sensor,pitch_sensor);
							bFlg = false;
					 }
					 else{
						 Disarmed_Done();
						 bFlg = true;
					 }
					 arming_counter = 0;
				}
		 }
	 }
}

void Auto_Armed_Check(int16_t iThrottleOut)
{
		static uint16_t throttle_low_counter = 0;
		static uint8_t angle_protect_count = 0;
    if(!Armed_Enable) return;

    //倾角保护(摇杆回中，倾斜角度超过70度超过一段时间)
//    if(int32_abs(RC_Roll) < 1000 && int32_abs(RC_Pitch) < 1000 &&
    if(fabs(accel_lpf[0].x) > 7.0f || fabs(accel_lpf[0].y) > 7.0f || accel_lpf[0].z > 0.0f) {   // 当前只用第一个芯片的加计信号
        if(angle_protect_count < 30) {
            angle_protect_count++;
        } else {
            Disarmed_Done();
        }
    } else {
        angle_protect_count = 0;
    }

//    if(/*inav_position.z<100&&*/iThrottleOut <= 250 && RC_Throttle == 0) {
		
		if(iThrottleOut <= fShutOffThrottle && bRCThrottleEQ0) {  // V5.05.240914: 当油门开度值bRCThrottleEQ0为真，且iThrottleOut小于关机油门值时，检测是否关机
        throttle_low_counter++;
        if(throttle_low_counter > 50) { // 100ms持续满足条件
            Disarmed_Done();
            throttle_low_counter = 0;
        }
    } else {
        throttle_low_counter = 0;
    }
}

void fifty_hz_loop(double fWxyz[3], double fAxyz[3], double fDcmRoll, double fDcmPitch, int16_t iThrottleOut)
{
	Vector3f Wxyz,Axyz;
	int32_t roll_sensor,pitch_sensor;
	
	Wxyz.x = fWxyz[0];		Wxyz.y = fWxyz[1];	Wxyz.z = fWxyz[2];
	Axyz.x = fAxyz[0];		Axyz.y = fAxyz[1];	Axyz.z = fAxyz[2];
	roll_sensor= (int32_t)(fDcmRoll*RAD_TO_DEG_X100);	  pitch_sensor = (int32_t)(fDcmPitch*RAD_TO_DEG_X100);
	
	level_space_check(Wxyz,Axyz);
	if(!Armed_Enable)	Level_Cal_Update();    //水平校准
	
	Motors_Armed_Check(roll_sensor,pitch_sensor);  //电机解锁检测
	Auto_Armed_Check(iThrottleOut);    //电机自动上锁检测
}

AFCSTATUS getAFCSts(void)
{
	AFCSTATUS sts;
	sts.bArmedEnable = Armed_Enable; sts.bDcmFastGroundGains = dcm_fast_ground_gains; 
	sts.bLandComplete = Land_Complete; sts.bSystemInitSts = System_Init_Status;
	
  getMotorsAFCSts(&sts);

	sts.iSdCardWriteErr = (uint8_t)g_iWriteSDFailedCnt;
	sts.iTaskOverTimeNo = (uint8_t) g_sRealTimeCount.err_flag;
	
	if(iSbusErrNum<0x0f)	sts.iSbusErrNum = iSbusErrNum;
	else sts.iSbusErrNum = 0xf;
	
	return sts;
}

void setAFCLandCompleteSts(bool bVal)
{
	Land_Complete = bVal;
}

void setAFCSts(uint8_t iBit,bool bVal)
{
	switch(iBit){
		case SYSTEM_INIT_STS_D0:
			System_Init_Status = bVal;
		  break;
		case LAND_COMPLETE_D1:
			Land_Complete = bVal;
			break;
		case ARMED_ENABLE_D2:
			Armed_Enable = bVal;
		  break;
		case DCM_FAST_GROUND_D3:
			dcm_fast_ground_gains = bVal;
		  break;
	}
}

void initAFCSts(void)
{
	System_Init_Status = false;
  Land_Complete = true;  //是否着陆标志位
	Auto_Armed_Enable = false; //自动上锁标志位
	Armed_Enable = false;  //解锁上锁使能
	dcm_fast_ground_gains = true;
	initMotorsAFCSts();
}

/************************ (C) COPYRIGHT ACG co. *****END OF FILE****/
