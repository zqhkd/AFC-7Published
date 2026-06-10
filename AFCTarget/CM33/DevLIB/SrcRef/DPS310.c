/******************** (C) COPYRIGHT 2019 ACE Tech Co.*************************
 * 作者	  ： 曾庆华
 * 文件名  DPS310.c
 * 版  本 ： 
 *    V4.01.220402 -- 
 * 
 * 描述   ：DPS310芯片大气参数传感器接口底层函数
 * 官网   ：www.acecreator.com
 * 淘宝   ：acecreator.taobao.com
 * 公众号 ：无人飞行控制
 *
*****************************************************************************/
	
#include "DPS310.h"
#include "main.h"
#include "spi.h"
#include "spiAPI.h"
#include "stdbool.h"

#define DPS310_REG_PRESS  0x00
#define DPS310_REG_TEMP   0x03
#define DPS310_REG_PCONF  0x06
#define DPS310_REG_TCONF  0x07
#define DPS310_REG_MCONF  0x08
#define DPS310_REG_CREG   0x09
#define DPS310_REG_ISTS   0x0A
#define DPS310_REG_FSTS   0x0B
#define DPS310_REG_RESET  0x0C
#define DPS310_REG_PID    0x0D
#define DPS310_REG_COEF   0x10
#define DPS310_REG_CSRC   0x28

typedef struct dps280_cal {
		int16_t C0;  // 12bit
		int16_t C1;  // 12bit
		int32_t C00; // 20bit
		int32_t C10; // 20bit
		int16_t C01; // 16bit
		int16_t C11; // 16bit
		int16_t C20; // 16bit
		int16_t C21; // 16bit
		int16_t C30; // 16bit
		uint8_t temp_source;
} calibration;
calibration dps310_cal;

void read_calibration(void);
void calculate_PT(int32_t UT, int32_t UP, float *pressure, float *temperature);
float get_altitude_difference(float press);

// 气压计DPS310的全局变量数据
bool g_bUsedOfDPS310;   // DPS310使用标志
uint8_t dps310_id = 0;
float pressure, temperature;
float base_pressure,base_temperature;
float baro_altitude = 0.0f;

void initDPS310(void) 
{
	SPI4_CS(SPI4CsDPS310);
	HAL_Delay(2);
	dps310_id = SPI4_Read(SPI4CsDPS310,0x0D);  //0x10
	if(dps310_id!=0x10) {
		g_bUsedOfDPS310 = false;return;
	}
	
	read_calibration();
	
	SPI4_Write(SPI4CsDPS310,DPS310_REG_CREG, 0x0C);  // shift for 16x oversampling
	SPI4_Write(SPI4CsDPS310,DPS310_REG_PCONF, 0x54);  // 32 Hz, 16x oversample
	SPI4_Write(SPI4CsDPS310,DPS310_REG_TCONF, 0x54 | dps310_cal.temp_source);  // 32 Hz, 16x oversample
	SPI4_Write(SPI4CsDPS310,DPS310_REG_MCONF, 0x07);  // continuous temp and pressure.
	
	// work around broken temperature handling on some sensors
	// using undocumented register writes
	// see https://github.com/infineon/DPS310-Pressure-Sensor/blob/dps310/src/DpsClass.cpp#L442
	SPI4_Write(SPI4CsDPS310,0x0E, 0xA5);
	SPI4_Write(SPI4CsDPS310,0x0F, 0x96);
	SPI4_Write(SPI4CsDPS310,0x62, 0x02);
	SPI4_Write(SPI4CsDPS310,0x0E, 0x00);
	SPI4_Write(SPI4CsDPS310,0x0F, 0x00);

	SPI4_NCS(SPI4CsDPS310);
	
	g_bUsedOfDPS310 = true;
}

//bool resetDPS_TP = true;
bool resetDPS_TP = false;   // 为确保正常使用DPS310模块时，不做复位压力值处理，当调用
void reset_temp_press_update(void)
{
	if(!resetDPS_TP) return;
	base_pressure = base_pressure*0.9f + pressure*0.1f;
	base_temperature = base_temperature*0.9f + temperature*0.1f;
}

void DPS310_ReadData(void) 
{
	static uint8_t baro_set_delay = 0;
	uint8_t buf[6];
	
	SPI4_CS(SPI4CsDPS310);
	SPI4_Read_Long(SPI4CsDPS310,DPS310_REG_PRESS,buf,3);
	SPI4_Read_Long(SPI4CsDPS310,DPS310_REG_TEMP,&buf[3],3);
	
	int32_t temp,press;
	press = ((int32_t)buf[2]) + ((int32_t)buf[1]<<8) + ((int32_t)buf[0]<<16);
	press = (press&0x800000) ? (0xFF000000|press) : press;
	temp  = ((int32_t)buf[5]) + ((int32_t)buf[4]<<8) + ((int32_t)buf[3]<<16);
	temp = (temp&0x800000) ? (0xFF000000|temp) : temp;
	SPI4_NCS(SPI4CsDPS310);
	
	calculate_PT(temp, press, &pressure, &temperature);
	
	if(baro_set_delay<50) {
		baro_set_delay++;
		if(baro_set_delay==50) {
			base_pressure = pressure;
			base_temperature = temperature;
		}
	}
	else{
		reset_temp_press_update();
		baro_altitude = get_altitude_difference(pressure);   // ret*100后返回气压高度计单位为cm
	}
}

float get_altitude_difference(float press)
{
    float ret;

//	  if(fabs(press)<0.01f) return 0.0f;  // V5.05.240921：原press==0，程序存在错误。实测气压值可为负值
	
    float scaling = press / base_pressure;
    float temp    = base_temperature + 273.15f;

	  ret = 153.8462f * temp * (1.0f - expf(0.190259f * logf(scaling)));   // ret返回值单位为m

    return ret;
}

void read_calibration(void)
{
	uint8_t buf[18];
	
	SPI4_Read_Long(SPI4CsDPS310,DPS310_REG_COEF,buf,18);
	dps310_cal.C0  = ((int16_t)buf[0] << 4) + ((buf[1] >>4) & 0x0F);
  dps310_cal.C0 = (dps310_cal.C0&0x0800)?(0xF000|dps310_cal.C0):dps310_cal.C0;
	dps310_cal.C1  = (buf[2] + (((int16_t)buf[1] & 0x0F)<<8));
	dps310_cal.C1 = (dps310_cal.C1&0x0800)?(0xF000|dps310_cal.C1):dps310_cal.C1;
	dps310_cal.C00 = (((int32_t)buf[4]<<4) + ((int32_t)buf[3]<<12)) + (((int32_t)buf[5]>>4) & 0x0F);
	dps310_cal.C00 = (dps310_cal.C00&0x080000)?(0xFFF00000|dps310_cal.C00):dps310_cal.C00;
	dps310_cal.C10 = (((int32_t)buf[5] & 0x0F)<<16) + buf[7] + ((int32_t)buf[6]<<8);
	dps310_cal.C10 = (dps310_cal.C10&0x080000)?(0xFFF00000|dps310_cal.C10):dps310_cal.C10;
	dps310_cal.C01 = (buf[9] + ((int16_t)buf[8]<<8));
	dps310_cal.C11 = (buf[11] + ((int16_t)buf[10]<<8));
	dps310_cal.C20 = (buf[13] + ((int16_t)buf[12]<<8));
	dps310_cal.C21 = (buf[15] + ((int16_t)buf[14]<<8));
	dps310_cal.C30 = (buf[17] + ((int16_t)buf[16]<<8));
	
	dps310_cal.temp_source = SPI4_Read(SPI4CsDPS310,DPS310_REG_CSRC);
	dps310_cal.temp_source &= 0x80;
}

void calculate_PT(int32_t UT, int32_t UP, float *pressure, float *temperature)
{
	// scaling for 16x oversampling
  const float scaling_16 = 1.0f/253952;
	
	float temp_scaled;
	float press_scaled;

	temp_scaled = (float)UT * scaling_16;
	*temperature = dps310_cal.C0 * 0.5f + dps310_cal.C1 * temp_scaled;

	 press_scaled = (float)UP * scaling_16;

	*pressure = dps310_cal.C00;
	*pressure += press_scaled * (dps310_cal.C10 + press_scaled * (dps310_cal.C20 + press_scaled * dps310_cal.C30));
	*pressure += temp_scaled * dps310_cal.C01;
	*pressure += temp_scaled * press_scaled * (dps310_cal.C11 + press_scaled * dps310_cal.C21);
}

void resetDPS310(bool bflg)
{
	 resetDPS_TP = bflg;
}
/************************ (C) COPYRIGHT ACE Co. about ADIS164XX *****END OF FILE****/
