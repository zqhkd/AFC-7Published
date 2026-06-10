#include "ICM20602.h"
#include "spi.h"
#include "spiAPI.h"
#include "Sensor.h"

#include "AFCGlobalVar.h"
#include "ACGCommonAPI.h"

bool g_bUsedOfICM20602;   // ICM42688使用标志
uint8_t icm20602_id = 0;
void initICM20602(void) 
{
	uint8_t data=0;
	
	SPI4_CS(SPI4CsICM20602);
	HAL_Delay(2);
	icm20602_id = SPI4_Read(SPI4CsICM20602,0x75);  //0x12
	if(icm20602_id==0x12) {
		HAL_Delay(50);
		SPI4_Write(SPI4CsICM20602,0x6B,0x80);
		HAL_Delay(50);
		//00000001 PWR_MGMT_1       [6]	SLEEP	When set to 1, the chip is set to sleep mode.                                              
		//[2:0]	CLKSEL[2:0] 1	Auto selects the best available clock source  PLL if ready, else use the Internal oscillator 
		SPI4_Write(SPI4CsICM20602,0x6B,0x01);
		HAL_Delay(3);
		
		//00001100 GYROSCOPE CONFIGURATION [4:3] UI Path Gyro Full Scale Select 011 = ∮2000dps
		SPI4_Write(SPI4CsICM20602,0x1B,0x18);
		//00000001 CONFIGURATION [2:0]	DLPF_CFG[2:0] Filter BW 184(Hz) Filter Delay 2.9 (ms)
		SPI4_Write(SPI4CsICM20602,0x1A,0x01);  //0x01:177Hz  0x02:108Hz
		//00000100 SMPLRT_DIV SMPLRT_DIV[7:0]   200Hz = Divides the internal sample rate (see register CONFIG) to generate 
		//the sample rate that controls sensor data output rate, FIFO sample rate.  NOTE: This register is only effective 
		//when FCHOICE_B register bits are 2ˇb00, and (0 < DLPF_CFG < 7). This is the update rate of the sensor 
		//register: SAMPLE_RATE = INTERNAL_SAMPLE_RATE / (1 + SMPLRT_DIV) Where INTERNAL_SAMPLE_RATE = 1kHz
		SPI4_Write(SPI4CsICM20602,0x19,0x04);
		//00000000 LP_MODE_CONFIG [7]	GYRO_CYCLE	When set to ˉ1ˇ low-power gyroscope mode is enabled.  Default setting is ˉ0ˇ
		SPI4_Write(SPI4CsICM20602,0x1E,0x00);
		
		//00001000 ACCEL_CONFIG [4:3]	AFS_SEL[1:0]	UI Path Accel Full Scale Select:10 = ∮8g
		SPI4_Write(SPI4CsICM20602,0x1C,0x10);
		//00000100 SMPLRT_DIV SMPLRT_DIV[7:0]   200Hz = Divides the internal sample rate (see register CONFIG) to generate 
		//the sample rate that controls sensor data output rate, FIFO sample rate.  NOTE: This register is only effective 
		//when FCHOICE_B register bits are 2ˇb00, and (0 < DLPF_CFG < 7). This is the update rate of the sensor 
		//register: SAMPLE_RATE = INTERNAL_SAMPLE_RATE / (1 + SMPLRT_DIV) Where INTERNAL_SAMPLE_RATE = 1kHz
		SPI4_Write(SPI4CsICM20602,0x19,0x04);
		//00100000 ACCEL_CONFIG2       [5:4]  DEC2_CFG[1:0] 0 
		SPI4_Write(SPI4CsICM20602,0x1D,0x02);  //0x02:121Hz  0x03:61Hz
		
		//SAMPLE_RATE = INTERNAL_SAMPLE_RATE / (1 + SMPLRT_DIV)
		//Where INTERNAL_SAMPLE_RATE = 1kHz
		SPI4_Write(SPI4CsICM20602,0x19,0x00);  //500Hz
		
		//set [5:3] as 0; 1  XYZ accelerometer is disabled. 0  XYZ accelerometer is on.
		data = SPI4_Read(SPI4CsICM20602,0x6C);
		data = (data|0x80)&0xC7; 
		SPI4_Write(SPI4CsICM20602,0x6C,data);
		
		//set [2:0] as 0; 1  XYZ gyroscope is disabled. 0  XYZ gyroscope is on.
		data = SPI4_Read(SPI4CsICM20602,0x6C);
		data = (data|0x80)&0xF8; 
		SPI4_Write(SPI4CsICM20602,0x6C,data);
		
		SPI4_NCS(SPI4CsICM20602);
		
		g_bUsedOfICM20602 = true;
	} else g_bUsedOfICM20602 = false;
}

float icm_20602_acc[3],icm_20602_gyr[3],icm_20602_temp;
void ICM20602_ReadData(void) 
{
	float tmp;
	uint8_t data[6];

	SPI4_CS(SPI4CsICM20602);
	
	SPI4_Read_Long(SPI4CsICM20602,0x3B,data,6);
  icm_20602_acc[0] = (int16_t) (data[0] << 8) | data[1];
  icm_20602_acc[1] = (int16_t) (data[2] << 8) | data[3];
  icm_20602_acc[2] = (int16_t) (data[4] << 8) | data[5];
	
	SPI4_Read_Long(SPI4CsICM20602,0x43,data,6);
	icm_20602_gyr[0] = (int16_t) (data[0] << 8) | data[1];
  icm_20602_gyr[1] = (int16_t) (data[2] << 8) | data[3];
  icm_20602_gyr[2] = (int16_t) (data[4] << 8) | data[5];
			
	// 读取温度值
	SPI4_Read_Long(SPI4CsICM20602,0x41,data,2);
	icm_20602_temp = (int16_t) (data[0] << 8) | data[1];
	icm_20602_temp = icm_20602_temp/326.8f + 25.0f;
	
	SPI4_NCS(SPI4CsICM20602);
	
	// AFC-5V5.05.240817: 机体系按前右下定义，注意Axyz测量的是除重力外无人机的力，那么未飞行时仅有重力时，无人机反作用力正向刚好为后、左、上为正向
	if(g_UavFcsParam.UavPara.FcsBoard == AFC5A_BOARD){
			tmp = icm_20602_acc[0];              // AFC-5V5.03.240616根据AFC-5正面向上安装要求
			icm_20602_acc[0] = -icm_20602_acc[1] * iXYZDir(g_UavFcsParam.UavPara.XSetup);
			icm_20602_acc[1] = tmp * iXYZDir(g_UavFcsParam.UavPara.YSetup);
			icm_20602_acc[2] = icm_20602_acc[2]* iXYZDir(g_UavFcsParam.UavPara.ZSetup);  // AFC-5V5.03.240616根据AFC-5正面向上安装要求，az取反
			
			tmp = icm_20602_gyr[0];              // AFC-5V5.03.240616根据AFC-5正面向上安装要求，wy取反
			icm_20602_gyr[0] = -icm_20602_gyr[1]* iXYZDir(g_UavFcsParam.UavPara.XSetup);
			icm_20602_gyr[1] = tmp* iXYZDir(g_UavFcsParam.UavPara.YSetup);
			icm_20602_gyr[2] = icm_20602_gyr[2]* iXYZDir(g_UavFcsParam.UavPara.ZSetup);  // AFC-5V5.03.240616根据AFC-5正面向上安装要求，wz取反
	}
		
	if(g_UavFcsParam.UavPara.FcsBoard == AFC5B_BOARD){
			tmp = icm_20602_acc[0];
			icm_20602_acc[0] = icm_20602_acc[1] * iXYZDir(g_UavFcsParam.UavPara.XSetup);
			icm_20602_acc[1] = -tmp * iXYZDir(g_UavFcsParam.UavPara.YSetup);   // AFC-5V5.03.240616根据AFC-5正面向上安装要求，ay取反
			icm_20602_acc[2] = icm_20602_acc[2]* iXYZDir(g_UavFcsParam.UavPara.ZSetup);  // AFC-5V5.03.240616根据AFC-5正面向上安装要求，az取反
			
			tmp = icm_20602_gyr[0];              // AFC-5V5.03.240616根据AFC-5正面向上安装要求，wx取反
			icm_20602_gyr[0] = icm_20602_gyr[1]* iXYZDir(g_UavFcsParam.UavPara.XSetup);
			icm_20602_gyr[1] = tmp* iXYZDir(g_UavFcsParam.UavPara.YSetup) * (-1);  // AFC-5V5.03.240616根据AFC-5正面向上安装要求，wy取反
			icm_20602_gyr[2] = icm_20602_gyr[2]* iXYZDir(g_UavFcsParam.UavPara.ZSetup);  
	}
		
	// AFC-5V5.05.240817: 机体系按前右下定义，注意Axyz测量的是除重力外无人机的力，那么未飞行时仅有重力时，无人机反作用力正向刚好为后、左、上为正向
	if(g_UavFcsParam.UavPara.FcsBoard == AFC6_BOARD){
			icm_20602_acc[0] = icm_20602_acc[0] * iXYZDir(g_UavFcsParam.UavPara.XSetup);
			icm_20602_acc[1] = icm_20602_acc[1] * iXYZDir(g_UavFcsParam.UavPara.YSetup);
			icm_20602_acc[2] = icm_20602_acc[2]* iXYZDir(g_UavFcsParam.UavPara.ZSetup);  // AFC-5V5.03.240616根据AFC-5正面向上安装要求，az取反
			
			icm_20602_gyr[0] = icm_20602_gyr[0]* iXYZDir(g_UavFcsParam.UavPara.XSetup);
			icm_20602_gyr[1] = icm_20602_gyr[1]* iXYZDir(g_UavFcsParam.UavPara.YSetup);
			icm_20602_gyr[2] = icm_20602_gyr[2]* iXYZDir(g_UavFcsParam.UavPara.ZSetup);  // AFC-5V5.03.240616根据AFC-5正面向上安装要求，wz取反
	}
		
	Sensor_Accel_Raw_Update(1,icm_20602_acc[0],icm_20602_acc[1],icm_20602_acc[2]);
	Sensor_Gyro_Raw_Update(1,icm_20602_gyr[0],icm_20602_gyr[1],icm_20602_gyr[2],icm_20602_temp);
	
	Sensor_LPF(1, g_bICM20602Calibing);    // 
}
/************************ (C) COPYRIGHT ACE Co. about ICM20602 by ZengQinghua *****END OF FILE****/
