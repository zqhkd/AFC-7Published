/******************** (C) COPYRIGHT 2019 ACE Tech Co.*************************
 * 作者	  ： 曾庆华
 * 文件名  ICM42688.c
 * 版  本 ： 
 *    V4.01.220402 -- (1) 基于ADIS16507.c的V4.01.220320直接进行更改
 * 
 * 描述   ：ICM42688芯片惯组接口底层函数
 * 官网   ：www.acecreator.com
 * 淘宝   ：acecreator.taobao.com
 * 公众号 ：无人飞行控制
 *
*****************************************************************************/
	
#include "main.h"
#include "spi.h"
#include "spiAPI.h"
#include "ICM42688.h"
#include "Sensor.h"
#include "AFCGlobalVar.h"
#include "ACGCommonAPI.h"

#define ICM46288_CS_H HAL_GPIO_WritePin(ICM46288_CS_GPIO_Port,ICM46288_CS_Pin,GPIO_PIN_SET)
#define ICM46288_CS_L HAL_GPIO_WritePin(ICM46288_CS_GPIO_Port,ICM46288_CS_Pin,GPIO_PIN_RESET)

#define ADDRESS       0XD0
#define WHOAMI        0X75
#define PWR_MGMT0     0x4E
#define GYRO_CONFIG0  0x4F
#define ACCEL_CONFIG0 0x50
#define SENSOR_CONFIG0 0x03
#define ACCEL_DATA_X1 0x1F
#define ACCEL_DATA_X0 0x20
#define ACCEL_DATA_Y1 0x21
#define ACCEL_DATA_Y0 0x22
#define ACCEL_DATA_Z1 0x23
#define ACCEL_DATA_Z0 0x24
#define GYRO_DATA_X1  0x25
#define GYRO_DATA_X0  0x26
#define GYRO_DATA_Y1  0x27
#define GYRO_DATA_Y0  0x28
#define GYRO_DATA_Z1  0x29
#define GYRO_DATA_Z0  0x2A
#define REG_BANK_SEL  0x76

// MEMS惯组42688的全局变量数据
bool g_bUsedOfICM42688;   // ICM42688使用标志
uint8_t icm42688_id = 0;
void initICM42688(void) 
{
	HAL_Delay(10);
	icm42688_id = SPI1_Read(0,0x75);  //0x47
	if(icm42688_id==0x47) {
		SPI1_Write(0,0X76, 0x00);
		SPI1_Write(0,0X11, 0x01); //reset
		HAL_Delay(100);
		SPI1_Write(0,0X11, 0x00); //
		HAL_Delay(50);

		SPI1_Write(0,0X4E, 0x0F); //???????????????
		SPI1_Write(0,0X4F, 0x06); //2000dps/? ,1KHZ????
		SPI1_Write(0,0X50, 0x26); //???8G ,1KHZ????
		SPI1_Write(0,0X51, 0x1A); //TEMP 20HZ??, ?????????
		SPI1_Write(0,0X52, 0x72); //???20HZ??, ???80HZ??   //0x75 25HZ  10H
		SPI1_Write(0,0X53, 0x14); //?????????

		SPI1_Write(0,0X76, 0x01);
		SPI1_Write(0,0X0b, 0x01);
		SPI1_Write(0,0X0c, 0x06);
		SPI1_Write(0,0X0d, 0x24);
		SPI1_Write(0,0X0e, 0xa0);

		SPI1_Write(0,0X76, 0x02);
		SPI1_Write(0,0X03, (0x06<<1));
		SPI1_Write(0,0X04, 0x24);
		SPI1_Write(0,0X05, 0xa0);

		SPI1_Write(0,0X76, 0x00);
		
		g_bUsedOfICM42688 = true;
	}
	else{
		g_bUsedOfICM42688 = false;
	}
}

float icm_42688_acc[3],icm_42688_gyr[3],icm_42688_temp;
void ICM42688_ReadData(void) 
{
	float tmp;
	uint8_t data[6];
	
	SPI1_Read_Long(0,ACCEL_DATA_X1,data,6);
  icm_42688_acc[0] = (int16_t) (data[0] << 8) | data[1];
  icm_42688_acc[1] = (int16_t) (data[2] << 8) | data[3];
  icm_42688_acc[2] = (int16_t) (data[4] << 8) | data[5];
	
	SPI1_Read_Long(0,GYRO_DATA_X1,data,6);
	icm_42688_gyr[0] = (int16_t) (data[0] << 8) | data[1];
  icm_42688_gyr[1] = (int16_t) (data[2] << 8) | data[3];
  icm_42688_gyr[2] = (int16_t) (data[4] << 8) | data[5];
	
	// 读取温度值
	SPI1_Read_Long(0,0x1D,data,2);
	icm_42688_temp = (int16_t) (data[0] << 8) | data[1];
	icm_42688_temp = icm_42688_temp/132.48f + 25.0f;

	if(g_UavFcsParam.UavPara.FcsBoard == AFC6_BOARD){
//		tmp = icm_42688_acc[0];                // AFC-5V5.03.240616根据AFC-5正面向上安装要求
		icm_42688_acc[0] = icm_42688_acc[0] * iXYZDir(g_UavFcsParam.UavPara.XSetup);   // 取消乘以-1的目的，芯片测得值按向前为正x轴，而机体承受反作用力
		icm_42688_acc[1] = icm_42688_acc[1]  * iXYZDir(g_UavFcsParam.UavPara.YSetup);   // 取消乘以-1的目的，芯片测得值按右侧向下为正y轴，而机体承受反作用力
		icm_42688_acc[2] = icm_42688_acc[2] * iXYZDir(g_UavFcsParam.UavPara.ZSetup);   // Z轴向下，取消乘以-1
		
//		tmp = icm_42688_gyr[0];               // AFC-5V5.03.240616根据AFC-5正面向上安装要求，wy取反
		icm_42688_gyr[0] = icm_42688_gyr[0]  * iXYZDir(g_UavFcsParam.UavPara.XSetup);   // 绕x轴右滚为正
		icm_42688_gyr[1] = icm_42688_gyr[1] * iXYZDir(g_UavFcsParam.UavPara.YSetup);         // 乘以-1的目的是芯片测得值按向左为正y轴, 绕y轴按右手系旋转抬头wy为正      
		icm_42688_gyr[2] = icm_42688_gyr[2] * iXYZDir(g_UavFcsParam.UavPara.ZSetup); // AFC-5V5.03.240616根据AFC-5正面向上，z轴向下，左偏为正，wz取反
	}
	
	if(g_UavFcsParam.UavPara.FcsBoard == AFC5A_BOARD){
	// AFC-5V5.05.240817: 机体系按前右下定义，注意Axyz测量的是除重力外无人机的力，那么未飞行时仅有重力时，无人机反作用力刚好是后、左、上为正向
		tmp = icm_42688_acc[0];                // AFC-5V5.03.240616根据AFC-5正面向上安装要求
		icm_42688_acc[0] = -icm_42688_acc[1] * iXYZDir(g_UavFcsParam.UavPara.XSetup);   // 取消乘以-1的目的，芯片测得值按向前为正x轴，而机体承受反作用力
		icm_42688_acc[1] = tmp  * iXYZDir(g_UavFcsParam.UavPara.YSetup);   // 取消乘以-1的目的，芯片测得值按右侧向下为正y轴，而机体承受反作用力
		icm_42688_acc[2] = icm_42688_acc[2] * iXYZDir(g_UavFcsParam.UavPara.ZSetup);   // Z轴向下，取消乘以-1
		
		tmp = icm_42688_gyr[0];               // AFC-5V5.03.240616根据AFC-5正面向上安装要求，wy取反
		icm_42688_gyr[0] = -icm_42688_gyr[1]  * iXYZDir(g_UavFcsParam.UavPara.XSetup);   // 绕x轴右滚为正
		icm_42688_gyr[1] = tmp * iXYZDir(g_UavFcsParam.UavPara.YSetup);         // 乘以-1的目的是芯片测得值按向左为正y轴, 绕y轴按右手系旋转抬头wy为正      
		icm_42688_gyr[2] = icm_42688_gyr[2] * iXYZDir(g_UavFcsParam.UavPara.ZSetup); // AFC-5V5.03.240616根据AFC-5正面向上，z轴向下，左偏为正，wz取反
	}
  
	Sensor_Accel_Raw_Update(0,icm_42688_acc[0],icm_42688_acc[1],icm_42688_acc[2]);
	Sensor_Gyro_Raw_Update(0,icm_42688_gyr[0],icm_42688_gyr[1],icm_42688_gyr[2],icm_42688_temp);
	
	Sensor_LPF(0,g_bICM42688Calibing);
}

/************************ (C) COPYRIGHT ACE Co. about ICM42688 by ZengQinghua *****END OF FILE****/
