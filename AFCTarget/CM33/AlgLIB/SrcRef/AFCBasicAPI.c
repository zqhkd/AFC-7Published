/******************** (C) COPYRIGHT 2019 ACE Tech Co.*************************
 * 作  者 ：  曾 庆 华
 * 文件名 ： AFCBasicAPI.c
 * 版  本 ：    
 *           V5.02.231221 : 基于案例旋翼小无人机K80 Pro进行改造。
 * 
 * 描  述 ：AFC-5 姿态控制基础模块（Basic Attitude Control Module）
 *               已实现基本遥控飞行与定高飞行功能
 *
 * 官  网 ：www.acecreator.com
 * 淘  宝 ：acecreator.taobao.com
 * 公众号 ：无人飞行控制
 *
*****************************************************************************/
#include "AFCBasicAPI.h"

#include "AFCAPI.h"
#include "AFCGlobalVar.h"
#include "ahrs.h"
#include "atti_control.h"
#include "alt_control.h"
#include "some_work.h"
#include "caculateMotorsCmd.h"
#include "LowPassFilter.h"
#include "flyMode.h"
#include "FM25V01.h"

typedef union{
	AFCSTATUS sSts;
	uint32_t  iSts;
}StsConvert;

uint32_t getAFCUintSts(AFCSTATUS sSts)
{
	  StsConvert tmp;
    tmp.sSts = sSts;	
	  return tmp.iSts;
}

AFCSTATUS getAFCStsBit(uint32_t iSts)
{
	  StsConvert tmp;
    tmp.iSts = iSts;	
	  return tmp.sSts;
}

uint32_t getAFCStsInt(void)
{
	  StsConvert tmp;
    tmp.sSts = getAFCSts();	
	  return tmp.iSts;
}

void ahrsStep(double fWxyz[3],double fAxyz[3],double fHeadAng,bool bDcmFast,double fDcmAng[3], double fDcmWxyz[3])  //姿态解算
{
//	  ahrs_update(*(Vector3f *)fWxyz,*(Vector3f *)fAxyz,fHeadAng,bDcmFast,(Vector3f *)fDcmAng,(Vector3f *)fDcmWxyz);  //姿态解算
	
	  ahrs_update(fWxyz,fAxyz,fHeadAng,bDcmFast, fDcmAng,  fDcmWxyz);  //姿态解算
}

void ahrsNStep(double fWx,double fWy,double fWz,double fAx,double fAy,double fAz,double fHeadAng,bool bDcmFast,double fDcmAng[3], double fDcmWxyz[3])  //姿态解算
{
//	  ahrs_update(*(Vector3f *)fWxyz,*(Vector3f *)fAxyz,fHeadAng,bDcmFast,(Vector3f *)fDcmAng,(Vector3f *)fDcmWxyz);  //姿态解算
	  double fWxyz[3],fAxyz[3];
	  fWxyz[0] = fWx;   fWxyz[1] = fWy;  fWxyz[2] = fWz;
	  fAxyz[0] = fAx;   fAxyz[1] = fAy;  fAxyz[2] = fAz;
	  ahrs_update(fWxyz,fAxyz,fHeadAng,bDcmFast, fDcmAng,  fDcmWxyz);  //姿态解算
}

// 姿态角位置控制
void initAttitudePos(void)
{
		switch(g_UavFcsParam.UavPara.UavFrame){
			case K80A_UAV:
				angle_kP[0] = 8.0f;
				angle_kP[1] = 6.6f;
				angle_kP[2] = 8.0f;
				break;
			case K80B_UAV:
				angle_kP[0] = 5.0f;
				angle_kP[1] = 5.0f;
				angle_kP[2] = 5.5f;
				break;
			case F550_UAV:
			case USER_UAV:
				angle_kP[0] = 6.6f;
				angle_kP[1] = 6.6f;
				angle_kP[2] = 8.0f;
				break;
			case HS620_UAV:
//				angle_kP[0] = 3.0f;
//				angle_kP[1] = 4.0f;
			  angle_kP[0] = 6.2f;  
				angle_kP[1] = 6.2f;
			
				angle_kP[2] = 8.0f;
				break;
		}
}

void attitudePosStep(double roll_angle_ef, double pitch_angle_ef, double yaw_rate_ef,double fDcmAng[3],double fWz, double *fAngleEfTargetZ, double fRateBfTarget[3], double *fEfTargetZ)
{
	attitude_control(roll_angle_ef,pitch_angle_ef,yaw_rate_ef,fDcmAng,fWz, fAngleEfTargetZ, fRateBfTarget);
	*fEfTargetZ = *fAngleEfTargetZ;
}

// 姿态角角速率控制
void initAttitudeRate(void)
{
		switch(g_UavFcsParam.UavPara.UavFrame){
			case K80A_UAV:
			// 以下为K80 Pro 飞行正常参数
				pid_rate[0].kp = 0.30f;
				pid_rate[0].ki = 0.1f;
				pid_rate[0].kd = 3.5f;
				
				pid_rate[1].kp = pid_rate[0].kp;
				pid_rate[1].ki = pid_rate[0].ki;
				pid_rate[1].kd = pid_rate[0].kd;
				
				pid_rate[2].kp = 0.6f;
				pid_rate[2].ki = 0.03f;
				pid_rate[2].kd = 1.5f;
				
				pid_rate[0].imax = 500.0f;
				pid_rate[1].imax = 500.0f;
				pid_rate[2].imax = 500.0f;
				break;
			case K80B_UAV:
				pid_rate[0].kp = 0.02f;
				pid_rate[0].ki = 0.06f;
				pid_rate[0].kd = 0.02f;
				
				pid_rate[1].kp = pid_rate[0].kp;
				pid_rate[1].ki = pid_rate[0].ki;
				pid_rate[1].kd = pid_rate[0].kd;
				
				pid_rate[2].kp = 0.2f;
				pid_rate[2].ki = 0.2f;
				pid_rate[2].kd = 0.0f;
				
				pid_rate[0].imax = 500.0f;
				pid_rate[1].imax = 500.0f;
				pid_rate[2].imax = 500.0f;
				break;
			case F550_UAV:
			case USER_UAV:
			// 以下为F550姿态稳定飞行正常参数
//				pid_rate[0].kp = 0.07f;
//				pid_rate[0].ki = 0.025f;
//				pid_rate[0].kd = 0.0022f;
//				
//				pid_rate[1].kp = pid_rate[0].kp;
//				pid_rate[1].ki = pid_rate[0].ki;
//				pid_rate[1].kd = pid_rate[0].kd;

//				pid_rate[2].kp = 0.3f;
//				pid_rate[2].ki = 0.03f;
//				pid_rate[2].kd = 0.5f;
//				
//				pid_rate[0].imax = 500.0f;
//				pid_rate[1].imax = 500.0f;
//				pid_rate[2].imax = 500.0f;
				pid_rate[0].kp = 0.09f;
				pid_rate[0].ki = 0.0f;
				pid_rate[0].kd = 0.0f;
				
				pid_rate[1].kp = pid_rate[0].kp;
				pid_rate[1].ki = pid_rate[0].ki;
				pid_rate[1].kd = pid_rate[0].kd;
				
				pid_rate[2].kp = 0.09f;
				pid_rate[2].ki = 0.0f;
				pid_rate[2].kd = 0.0f;
				
				pid_rate[0].imax = 500.0f;
				pid_rate[1].imax = 500.0f;
				pid_rate[2].imax = 500.0f;
				break;
			case HS620_UAV:
			// 以下为HS620姿态稳定飞行正常参数
				pid_rate[0].kp = 0.1f;
				pid_rate[0].ki = 0.025f;
				pid_rate[0].kd = 0.0f;      // 该值设置不当，容易引起姿态角回中
			
				pid_rate[1].kp = 0.1f;
				pid_rate[1].ki = 0.025f;
				pid_rate[1].kd = 0.0f;

				pid_rate[2].kp = 0.3f;
				pid_rate[2].ki = 0.03f;
				pid_rate[2].kd = 0.0f;
				
				pid_rate[0].imax = 500.0f;
				pid_rate[1].imax = 500.0f;
				pid_rate[2].imax = 500.0f;
				break;
		}
}

void attitudeRateStep(uint32_t iSts,double fRateBfTarget[3],double fDcmRate[3], double fChServoOut[3])
{
	  AFCSTATUS sts = getAFCStsBit(iSts);
	   
    attitude_output_controller(sts,fRateBfTarget,fDcmRate, fChServoOut);
}

// 高度控制初始化
void initAltitude(void)
{
		switch(g_UavFcsParam.UavPara.UavFrame){
			case K80A_UAV:
			// 以下为K80 Pro飞行正常参数
				alt_pos_kP = 1.8f;
				
				alt_rate.kp = 8.0f;
				alt_rate.ki = 0.0f;
				alt_rate.kd = 0.0f;
				alt_rate.imax = 0.0f;
				
				alt_accel.kp = 1.36f;
				alt_accel.ki = 0.1f;
				alt_accel.kd = 0.08f;
				alt_accel.imax = 800.0f;
				break;
			case K80B_UAV:
				alt_pos_kP = 6.0f;
				
				alt_rate.kp = 6.0f;
				alt_rate.ki = 10.0f;
				alt_rate.kd = 15.0f;
				alt_rate.imax = 0.0f;
				
				alt_accel.kp = 0.18f;
				alt_accel.ki = 0.0f;
				alt_accel.kd = 0.00f;
				alt_accel.imax = 800.0f;
				break;
			case F550_UAV:
			case USER_UAV:
			// 以下为F550定高飞行正常参数
				alt_pos_kP = 1.8f;
				
				alt_rate.kp = 5.0f;
				alt_rate.ki = 0.0f;
				alt_rate.kd = 0.0f;
				
				alt_rate.imax = 0.0f;
				
				alt_accel.kp = 0.5f;
				alt_accel.ki = 0.1f;
				alt_accel.kd = 0.02f;
				
				alt_accel.imax = 800.0f;
				break;
			case HS620_UAV:
			// 以下为HS620定高飞行正常参数
				alt_pos_kP = 1.1f;  // 高度位置环节 241109
				
			  alt_rate.kp = 7.8f;  // 高度速度环节  241109
				alt_rate.ki = 0.0f;
				alt_rate.kd = 0.0f;
				
				alt_rate.imax = 0.0f;
				
			// 高度加速度环节
			  alt_accel.kp = 0.57f;    
				alt_accel.ki = 0.11f;
				alt_accel.kd = 0.0f;
				
				alt_accel.imax = 800.0f;
				break;
		}
    init_alt_LPF();
	
	// 初始话高度通道油门相关参数
	  initAltitudeThrottle();
}

//高度状态更新
void altitudeStep(uint32_t iSts,double fBarHeight,double fDcmRoll, double fDcmPitch, double fThrottleOut,double *fPositionZ,double *fVelocityZ, double *fThrottleHover)
{
	  StsConvert tmp;
	  tmp.iSts = iSts;
		altitude_update(tmp.sSts,fBarHeight,fDcmRoll, fDcmPitch,(int16_t)fThrottleOut,fPositionZ,fVelocityZ, fThrottleHover);
}

void initAltitudeThrottle(void)
{
	  switch(g_UavFcsParam.UavPara.UavFrame){
			case K80A_UAV:
				// 以下为K80 Pro 参数
				// 悬停油门
				 fThrottleHover = 350;

			  g_iSelfDetectPW = 0;  // 自检时最小脉宽调整值
			// 新增怠速脉宽，该脉宽小无人机为130，它表示怠速运行时脉宽为minEscPWMWdith + 130
				 g_iIdlingSpeedWidth	= 130;
			// 着陆时关机油门值
				 fShutOffThrottle = 150;  // 后面曾更改为250
			
				 accel_z_cms = 300.0f;          
				 leash_down_z = 100.0f;         
				 leash_up_z = 300.0f;  
				break;
			case K80B_UAV:
			// 以下为K80B定高飞行正常参数
				// 悬停油门
				fThrottleHover = 500;
			
			// 新增怠速脉宽，该脉宽小无人机为130，它表示怠速运行时脉宽为minEscPWMWdith + 130
   			g_iIdlingSpeedWidth	= 150;
			// 着陆时关机油门值
				 fShutOffThrottle = 247; 
				
				accel_z_cms = 200.0f;          
				leash_down_z = 100.0f;         
				leash_up_z = 100.0f;  
				break;
			case F550_UAV:
			case USER_UAV:
			// 以下为F550定高飞行正常参数
			  g_iSelfDetectPW = 10;   // 电调自检时最小脉宽调整值
			
				// 悬停油门
				fThrottleHover = 200;
			// 新增怠速脉宽，该脉宽小无人机为130，它表示怠速运行时脉宽为minEscPWMWdith + 100
				g_iIdlingSpeedWidth	= 120;
			// 着陆时关机油门值
				 fShutOffThrottle = 247; 
				
				accel_z_cms = 200.0f;          
				leash_down_z = 100.0f;         
				leash_up_z = 100.0f;  
				break;
			case HS620_UAV:
			// 以下为HS620定高飞行正常参数
				  g_iSelfDetectPW = 30;   // 电调自检时最小脉宽调整值
			// 悬停油门
				fThrottleHover = 500;
			// 新增怠速脉宽，该脉宽小无人机为130，它表示怠速运行时脉宽为minEscPWMWdith + 150
				g_iIdlingSpeedWidth	= 190;    // V5.05.240825由170更改为200
			
			// 着陆时关机油门值
				fShutOffThrottle = 300;
			
				accel_z_cms = 200.0f;          
				leash_down_z = 100.0f;         
				leash_up_z   = 100.0f;  
			
			  g_RollSensor0 = 0;     g_PitchSensor0 = 0;
		
				break;
		}
}

// 起飞初始化
void initTakeOff(double position_z, double fThrottleOut)
{
	  init_takeoff(position_z,(int16_t)fThrottleOut);
}

// 水平校准，电机解锁等
void  takeOffLandRun(double fWxyz[3],double fAxyz[3],double fDcmRoll, double fDcmPitch, double fThrottleOut)
{
	  static uint8_t iTim20ms = 0;
	
		if(iTim20ms++ >= 20/g_iSimulinkAlgorithmStep){
			iTim20ms = 0;
			fifty_hz_loop(fWxyz, fAxyz,fDcmRoll, fDcmPitch, (int16_t)fThrottleOut);
		}
}

void  takeOffLand(double fWx,double fWy,double fWz,double fAx,double fAy,double fAz,double fDcmRoll, double fDcmPitch, double fThrottleOut)
{
	  static uint8_t iTim20ms = 0;

	  double fWxyz[3],fAxyz[3];
	  fWxyz[0] = fWx;   fWxyz[1] = fWy;  fWxyz[2] = fWz;
	  fAxyz[0] = fAx;   fAxyz[1] = fAy;  fAxyz[2] = fAz;
	
		if(iTim20ms++ >= 20/g_iSimulinkAlgorithmStep){
			iTim20ms = 0;
			fifty_hz_loop(fWxyz, fAxyz,fDcmRoll, fDcmPitch, (int16_t)fThrottleOut);
		}
}

// 起飞前处理，或者着陆处理。根据地面情况获取油门控制指令
void groundProc(double position_z,double fDcmAng[3], double fDcmWxyz[3],double *fAngleEfTargetZ,double fRateBfTarget[3],double *fThrottle)
{
	  setRateBfTarget(fDcmWxyz,fDcmAng[2],fAngleEfTargetZ,fRateBfTarget);	    // Bf表示机体坐标系，而Ef表示导航坐标系

		*fThrottle = set_throttle_out(0, false,fDcmAng[0],fDcmAng[1]);
	
		set_current_alt_to_target_alt(position_z);    // 0520：地面处理暂不用设置目标点，起飞时设置即可
}

// 输出电机电调指令值（0--100）
void outMotorsValStep(uint32_t iSts,double fServoOut[3], double *fThrottle, double fMotorsEscVal[MOTORS_CHANEL_NUM])
{
		int16_t iMotorsPWM[MOTORS_CHANEL_NUM];
	  outMotorsPWM(*((AFCSTATUS *)&iSts),fServoOut, fThrottle, iMotorsPWM);
	
	  for(uint8_t i=0;i < MOTORS_CHANEL_NUM;i++){
			fMotorsEscVal[i] = fGetEscVal(i,iMotorsPWM[i]);           // 暂时按照PWM值minPW--maxPW，电调调节值0--100来计算 
		}
//	  for(uint8_t i=0;i < MOTORS_CHANEL_NUM;i++) fMotorsEscVal[i] = (float) iMotorsPWM[i];           // 按照PWM原始值输出
}

void initFlyMode(void)
{
	// 包括三种飞行模式，可在此定义：
  //      1 -- ATTITDE_STAB_MODE:  对应姿态稳定模式
  //	    2 -- ALTITUDE_HOLD_MODE: 对应定高飞行模式
	//      3 -- POS_NAVI_MODE:      对应位置导航模式
  g_bUsedOfAFCBasicAC = true;  // 表示使用BasicAC模块库作为姿态稳定模式、定高飞行模式库
	
  resetDPS_TP = true;   // V5.03.240422: 进入正常飞行模式程序，刚启动无人机未解锁，处于地面位置，需对气压高度计进行复位处理，减少0位漂移
	
	initAttitudePos();
	Load_Parameters();
	initAFCSts();
}

//飞行模式
void 	flyModeStep(uint32_t iSts,double fPositionZ,double fVelocityZ,double fDcmAng[3],double fDcmWxyz[3],double fThrottleHover,double fRCCmd[4], double *fThrottleOut, double fRateBfTarget[3])
{
	 AFCSTATUS sts = getAFCStsBit(iSts);
	 flyModeRun(sts,fPositionZ,fVelocityZ,fDcmAng,fDcmWxyz,fThrottleHover,fRCCmd,fThrottleOut,fRateBfTarget);
	
   // 为connectPC程序准备姿态角解算数据
	  Vector3i attiAngDegX100 = getAttiAngRad2DegX100(fDcmAng);
		roll_sensor = attiAngDegX100.x;  pitch_sensor = attiAngDegX100.y; yaw_sensor = attiAngDegX100.z;
	//  为connectPC程序准备高度数据
		gfPositionZ = fPositionZ;
}

void getCtrlChCmd(double fRCRaw[4],double fCtrlChCmd[4])
{
	int16_t RCServCmd[3];
	int16_t RCThrottle, RCClimbRate;
	
	get_rccmd_simulink_value(fRCRaw,RCServCmd,&RCThrottle,&RCClimbRate);
	
	for(uint8_t i = 0; i < 3; i++) fCtrlChCmd[i] = (double) RCServCmd[i];
	fCtrlChCmd[3] = (double)RCClimbRate;
}

void setRcLimit(double bRCRoll,double bRCPitch,double bRCThrottle,double bRCCh3,double bRCCh2,double bRCCh1,double bRCCh0)
{
	 setRcLimitFlg((bool)bRCRoll,(bool)bRCPitch,(bool) bRCThrottle,(bool)bRCCh3,(bool)bRCCh2,(bool)bRCCh1,(bool)bRCCh0);
}

/************************ (C) COPYRIGHT ACG co. ********END OF FILE****/
