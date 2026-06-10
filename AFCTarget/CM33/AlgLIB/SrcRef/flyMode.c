#include "AFCGlobalVar.h"
#include "flyMode.h"
#include "atti_control.h"
#include "ahrs.h"
#include "alt_control.h"
#include "AFCBasicAPI.h"

#define FlyModeModuleNo  1

void flyModeRun(AFCSTATUS sts,double position_z,double velocity_z,double fDcmAng[3], double fDcmWxyz[3], double fThrottleHover, double fRCCmd[4],double *fThrottleOut, double fRateBfTarget[3])
{
//	int32_t roll_sensor, pitch_sensor,yaw_sensor;
	static double fAngleEfTargetZ;               // 通过顶层静态变量处理，确保simuink输入参数值改变后会影响其地址值（即传地址方式）
	
	int16_t RC_Roll, RC_Pitch, RC_Yaw, RC_Climb_Rate;
	// 从遥控器命令中提取滚转、俯仰、偏航和爬升率命令
	RC_Roll = (int16_t)fRCCmd[0];  RC_Pitch = (int16_t)fRCCmd[1];  RC_Yaw = (int16_t)fRCCmd[2];  RC_Climb_Rate = (int16_t) fRCCmd[3];
	
  // 如果无人机未解锁，则执行地面处理并返回	
	if(!sts.bArmedEnable /*|| RC_Throttle <= 0*/) {
    
    groundProc(position_z,fDcmAng,fDcmWxyz,&fAngleEfTargetZ,fRateBfTarget,fThrottleOut);
		
		g_iProcessRunNo = (FlyModeModuleNo<<8) + 1;
		
		return;
	}
	
	// 如果无人机处于定高模式或位置导航模式
  if((g_UavFcsParam.UavPara.FlyMode==ALTITUDE_HOLD_MODE) || (g_UavFcsParam.UavPara.FlyMode==POS_NAVI_MODE)){
		// 如果无人机已着陆完成且爬升率命令大于20，则重新初始化起飞
		if(sts.bLandComplete && RC_Climb_Rate > 20) {
			setAFCLandCompleteSts(false);
			init_takeoff(position_z,*fThrottleOut);
			g_iProcessRunNo = (FlyModeModuleNo<<8) + 2;
		}
		 // 如果无人机已着陆完成，则执行地面处理
		if(sts.bLandComplete) {
			groundProc(position_z,fDcmAng,fDcmWxyz,&fAngleEfTargetZ,fRateBfTarget,fThrottleOut);
			g_iProcessRunNo = (FlyModeModuleNo<<8) + 3;
		}
		 // 否则，执行姿态控制和油门输出更新
		else{
			//姿态控制
			attitude_control(RC_Roll*1.5, RC_Pitch*1.5, RC_Yaw, fDcmAng, fDcmWxyz[2],&fAngleEfTargetZ,fRateBfTarget);
			
			//油门输出
			*fThrottleOut = (double)(update_z_controller(sts,position_z,velocity_z,RC_Climb_Rate,fDcmAng[0], fDcmAng[1], (int16_t)fThrottleHover));

			g_iProcessRunNo = (FlyModeModuleNo<<8) + 4;
		}
  }
 // 如果不是定高模式或位置导航模式，则执行手动油门方式
	else{
			// 手动油门方式
		//	sts.bLandComplete = false;
			setAFCLandCompleteSts(false);
			
			//姿态控制
			attitude_control(RC_Roll*1.5, RC_Pitch*1.5, RC_Yaw, fDcmAng, fDcmWxyz[2],&fAngleEfTargetZ,fRateBfTarget);
			
			//油门输出
			*fThrottleOut = (double)set_throttle_out(RC_Climb_Rate, true, fDcmAng[0], fDcmAng[1]);
			
			g_iProcessRunNo = (FlyModeModuleNo<<8) + 5;
	}
}

void flyModeInit(void)
{
	
}

