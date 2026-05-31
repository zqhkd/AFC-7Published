/******************** (C) COPYRIGHT 2019 ACE Tech Co.*************************
 * 作    者： 曾庆华
 * 文 件 名： IST8310.c
 * 版    本： 
 *    
 * 
 * 描述   ：IST8310地磁传感器芯片底层接口函数
 * 官网   ：www.acecreator.com
 * 淘宝   ：acecreator.taobao.com
 * 公众号 ：无人飞行控制
 *
*****************************************************************************/
	
#include "IST8310.h"

#include "main.h"
#include "i2c.h"
#include "i2cAPI.h"
#include "AFCGlobalVar.h"
#include "ACGCommonAPI.h"

bool g_bUsedOfIST8310;   // IST8310使用标志

#define IST8310_SLAVE_ADDRESS     0x0C
#define IST8310_REG_STB           0x0C	//Self-Test response
#define IST8310_REG_INFO          0x01	//More Info
#define IST8310_REG_WIA           0x00	//Who I am
#define IST8310_REG_DATAX         0x03	//Output Value x
#define IST8310_REG_DATAY         0x05	//Output Value y
#define IST8310_REG_DATAZ         0x07	//Output Value z
#define IST8310_REG_STAT1         0x02	//Status register
#define IST8310_REG_STAT2         0x09	//Status register
#define IST8310_REG_CNTRL1        0x0A	//Control setting register 1
#define IST8310_REG_CNTRL2        0x0B	//Control setting register 2
#define IST8310_REG_CNTRL3        0x0D	//Control setting register 3
#define IST8310_REG_OFFSET_START  0xDC	//Offset
#define IST8310_REG_SELECTION_REG 0x42   //Sensor Selection register
#define IST8310_REG_TEST_REG      0x40   //Chip Test register
#define IST8310_REG_TUNING_REG    0x47    //Bandgap Tuning register

uint8_t ist8310_id = 0;
void initIST8310(void) 
{
	HAL_Delay(10);
	ist8310_id = Single_Read2(IST8310_SLAVE_ADDRESS, IST8310_REG_WIA);
	if(ist8310_id==0x10) {
		Single_Write2(IST8310_SLAVE_ADDRESS, 0x41, 0x24);
		Single_Write2(IST8310_SLAVE_ADDRESS, IST8310_REG_SELECTION_REG, 0xC0);
		Single_Write2(IST8310_SLAVE_ADDRESS, IST8310_REG_CNTRL1, 0x01);
		g_bUsedOfIST8310 = true;
	} else g_bUsedOfIST8310 = false;
}

//  获取数字磁力计传感器IST8310三轴磁力数据
#define MAG_XY_SCALE  0.1953125
#define MAG_Z_SCALE   0.30517558125
float g_fMagRaw[3];
float ist8310_angle = 0.0f;
void IST8310_ReadData(void) 
{
	int16_t mag_raw[3];
	
	int16_t tmp;
	uint8_t buf[6];
	I2C_Read2(IST8310_SLAVE_ADDRESS,0x03,6,buf);
	mag_raw[0] = (int16_t)((((uint16_t)buf[1]) << 8) | buf[0]);
  mag_raw[1] = (int16_t)((((uint16_t)buf[3]) << 8) | buf[2]);
  mag_raw[2] = (int16_t)((((uint16_t)buf[5]) << 8) | buf[4]);  
	
	
	tmp = mag_raw[0];
		// 相对AFC-5控制盒而言，以前、左、上为正向
	if(g_UavFcsParam.UavPara.FcsBoard == AFC5A_BOARD){
		mag_raw[0] = mag_raw[1] * iXYZDir(g_UavFcsParam.UavPara.XSetup);      // V5.04.240710 装配方向系数
		mag_raw[1] = tmp * iXYZDir(g_UavFcsParam.UavPara.YSetup);
		mag_raw[2] = mag_raw[2] * iXYZDir(g_UavFcsParam.UavPara.ZSetup);     // V5.04.240710 装配方向系数
	}
	
	if(g_UavFcsParam.UavPara.FcsBoard == AFC5B_BOARD){
		mag_raw[0] = -mag_raw[1]* iXYZDir(g_UavFcsParam.UavPara.XSetup);      // V5.04.240710 装配方向系数
		mag_raw[1] = -tmp * iXYZDir(g_UavFcsParam.UavPara.YSetup);
		mag_raw[2] = -mag_raw[2]  * iXYZDir(g_UavFcsParam.UavPara.ZSetup);      // V5.04.240710 装配方向系数
	}

		// 相对AFC-6控制盒而言，U5(8310)坐标系为左+X，后+Y，下+Z
	if(g_UavFcsParam.UavPara.FcsBoard == AFC6_BOARD){
		mag_raw[0] = -mag_raw[1] * iXYZDir(g_UavFcsParam.UavPara.XSetup);      // V5.04.240710 装配方向系数
		mag_raw[1] = -tmp * iXYZDir(g_UavFcsParam.UavPara.YSetup);
		mag_raw[2] = mag_raw[2] * iXYZDir(g_UavFcsParam.UavPara.ZSetup);     // V5.04.240710 装配方向系数
	}

	
	g_fMagRaw[0] = mag_raw[0] * MAG_XY_SCALE; g_fMagRaw[1] = mag_raw[1] * MAG_XY_SCALE; g_fMagRaw[2] = mag_raw[2] * MAG_Z_SCALE;  // 转换为标准单位uT
	Single_Write2(IST8310_SLAVE_ADDRESS, IST8310_REG_CNTRL1,0x01);
	
	if((mag_raw[0]!=0) || (mag_raw[1]!=0))	ist8310_angle = -atan2f(mag_raw[1], mag_raw[0])*RAD_TO_DEG;
}
/************************ (C) COPYRIGHT ACE Co. about IST8310 by ZengQinghua *****END OF FILE****/

