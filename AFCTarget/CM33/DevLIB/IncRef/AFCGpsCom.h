#ifndef __AFC_Gps_Com_H__
#define __AFC_Gps_Com_H__

#include <stdint.h>
#include <stdbool.h>
#include <usart.h>

#define GpsCom_MAX_Rx_SIZE  210   // NANO-D GPS语句要接收3条NMEA语句，字节数应该小于210

typedef enum  {
    NO_GPS = 0,             ///< No GPS connected/detected
    NO_FIX = 1,             ///< Receiving valid GPS messages but no lock
    GPS_OK_FIX_2D = 2,      ///< Receiving valid messages and 2D lock
    GPS_OK_FIX_3D = 3,      ///< Receiving valid messages and 3D lock
    GPS_OK_FIX_3D_DGPS = 4, ///< Receiving valid messages and 3D lock with differential improvements
    GPS_OK_FIX_3D_RTK = 5,  ///< Receiving valid messages and 3D lock, with relative-positioning improvements
} GPS_Status;

typedef struct {
   // all the following fields must all be filled by the backend driver
	 uint32_t gps_time;
   uint8_t num_sats;                   ///< Number of visible satelites
   GPS_Status status;                  ///< driver fix status,
   uint32_t time_week_ms;              ///< GPS time (milliseconds from start of GPS week)
   uint16_t time_week;                 ///< GPS week number
   double latitude,longitude;                  ///< last fix location
	
   float ground_speed,alt;                 ///< ground speed in m/sec
   float ground_course_cd;           ///< ground course in 100ths of a degree
   float hdop;                      ///< horizontal dilution of precision in cm
	
	 uint8_t gpssta;					//GPS质量指示符:0,定位不可用或无效;1,单点定位;2,伪距差分;4, RTK固定解；5, RTK浮点解；6,GNSS/INS组合导航；7, 用户设定位置.		

 //  Vector3f velocity;                  ///< 3D velocitiy in m/s, in NED format
	 float vx;
	 float vy;
	 float vz;
   uint8_t have_vertical_velocity;      ///< does this GPS give vertical velocity?
   uint32_t last_gps_time_ms;          ///< the system time we got the last GPS timestamp, milliseconds
} GpsInf;

// 外部可使用全局变量
extern bool bInitGpsComIn, g_bGpsUpdateFlg,g_bGpsPpsSignal;
extern GpsInf g_sGpsInf;

// 初始化DMA模式的GpsCom(huart2)。最关键问题是打开空闲中断
//  iGpsDev表示选用TG18--ublox格式GPS，还是选用NanoD -- NMEA格式GPS
void initGps(uint8_t iGpsDev);
// 处理GpsCom的空闲中断DMA：接收GpsCom字符串
void ProGpsComRcvIRQ(void);

// 获取GPS空间位置导航信息
double getGpsData(uint8_t iChannel);
	
#endif

/************************ (C) COPYRIGHT ACG co. *****END OF FILE****/
