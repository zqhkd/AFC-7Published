#include "Sensor.h"
#include "LowPassFilter.h"
#include "AFCGlobalVar.h"

float accel_raw[2][3],gyro_raw[2][3],temp_raw[2];

void Sensor_Accel_Raw_Update(uint8_t id,float x,float y,float z)
{
	accel_raw[id][0] = x;
	accel_raw[id][1] = y;
	accel_raw[id][2] = z;
}


void Sensor_Gyro_Raw_Update(uint8_t id,float x,float y,float z,float temp)
{
	gyro_raw[id][0] = x;
	gyro_raw[id][1] = y;
	gyro_raw[id][2] = z;
	
	temp_raw[id] = temp;
}

LPF_2P accel_filter[2][3];
LPF_2P gyro_filter[2][3];
Vector3f accel_lpf[2],gyro_lpf[2];
Vector3f accel_scale[2],accel_offset[2],gyro_offset[2];

void Sensor_LPF(uint8_t id, bool bCacFlg)
{
	static uint8_t lpf_init[2]={0,0};
	
	if(lpf_init[id]==0) {
		lpf_init[id] = 1;
		
		// V5.05.240823版本设置的MEMS惯组截止频率
		float fSampleFreq = 1000.f/g_iSimulinkAlgorithmStep;
		LPF2P_set_cutoff_frequency(&accel_filter[id][0], fSampleFreq, g_AccFilter.x);
		LPF2P_set_cutoff_frequency(&accel_filter[id][1], fSampleFreq, g_AccFilter.y);
		LPF2P_set_cutoff_frequency(&accel_filter[id][2], fSampleFreq, g_AccFilter.z);
		LPF2P_set_cutoff_frequency(&gyro_filter[id][0], fSampleFreq, g_GryoFilter.x);    // 2ms采样周期
		LPF2P_set_cutoff_frequency(&gyro_filter[id][1], fSampleFreq, g_GryoFilter.y);
		LPF2P_set_cutoff_frequency(&gyro_filter[id][2], fSampleFreq, g_GryoFilter.z);
	}
	
	// 是否处于标定工作状态
	if(bCacFlg){
		accel_lpf[id].x = accel_raw[id][0];
		accel_lpf[id].y = accel_raw[id][1];
		accel_lpf[id].z = accel_raw[id][2];
		
		gyro_lpf[id].x = gyro_raw[id][0];
		gyro_lpf[id].y = gyro_raw[id][1];
		gyro_lpf[id].z = gyro_raw[id][2];
	}
	else{
		accel_lpf[id].x = accel_raw[id][0] - accel_offset[id].x;
		accel_lpf[id].y = accel_raw[id][1] - accel_offset[id].y;
		accel_lpf[id].z = accel_raw[id][2] - accel_offset[id].z;
		
		gyro_lpf[id].x = gyro_raw[id][0] - gyro_offset[id].x;
		gyro_lpf[id].y = gyro_raw[id][1] - gyro_offset[id].y;
		gyro_lpf[id].z = gyro_raw[id][2] - gyro_offset[id].z;
	}
	
	accel_lpf[id].x = LPF2P_apply(&accel_filter[id][0], accel_raw[id][0]);
	accel_lpf[id].y = LPF2P_apply(&accel_filter[id][1], accel_raw[id][1]);
	accel_lpf[id].z = LPF2P_apply(&accel_filter[id][2], accel_raw[id][2]);
	gyro_lpf[id].x = LPF2P_apply(&gyro_filter[id][0], gyro_lpf[id].x);
	gyro_lpf[id].y = LPF2P_apply(&gyro_filter[id][1], gyro_lpf[id].y);
	gyro_lpf[id].z = LPF2P_apply(&gyro_filter[id][2], gyro_lpf[id].z);
	
	accel_lpf[id] = Vector3f_Scale(accel_lpf[id], ACCEL_SCALE_8G);
	gyro_lpf[id] = Vector3f_Scale(gyro_lpf[id], GYRO_SCALE_2000) ;
	
	// 授权管控
	if(!g_UavFcsParam.UavPara.bAuthorizedFlg){
		  accel_lpf[id].x += g_SysRndXyz.x/4.0f;  accel_lpf[id].y += g_SysRndXyz.y/4.0f; accel_lpf[id].z += g_SysRndXyz.z/4.0f;
//		  accel_lpf[id].x += g_SysRndXyz.x/2.0f;  accel_lpf[id].y += g_SysRndXyz.y/2.0f; accel_lpf[id].z += g_SysRndXyz.z/2.0f;
//		  gyro_lpf[id].x  += g_SysRndXyz.y/4.0f;  gyro_lpf[id].y  += g_SysRndXyz.z/4.0f; gyro_lpf[id].z  += g_SysRndXyz.x/4.0f;  
	}
}

