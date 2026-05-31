/******************** (C) COPYRIGHT 2019 ACE Tech Co.*************************
 * 作  者 ： 曾庆华
 * 文件名 ： FemtMesGps.c
 * 版  本 ： 基于ATKS1216F8更改为FemtMes GPS板程序
 *                              
 *      V4.01.220320: 主要是将以前的ATKS1216F8升级至飞纳经纬高精度GNSS板卡Femtomes而修改。
 *                    
 * 描  述 ：基于ATK S1216F8 GPS模块改造的GPS NMEA字符流处理函数库
 *            以“（库函数版本，适合阿波罗STM32F4开发板）扩展实验16 ATK-S1216F8 GPS模块实验”
 *              程序源码为基础进行修改
 * 官  网 ：www.acecreator.com
 * 淘  宝 ：acecreator.taobao.com
 * 公众号 ：无人飞行控制
*****************************************************************************/
#include "stdio.h"	 
#include "stdarg.h"	 
#include "string.h"	 
#include "math.h"

#include "usart.h"
#include "tim.h"
#include "ctype.h"

#include "AFCGlobalVar.h"
#include "AFCGpsCom.h"
#include "FemtMesGps.h" 

// 目前直接用AFCGpsCom.h中定义的GpsInf来代替TNmeaFetMes, 为保证FemtMesGps和ublox M10的兼容性，全面调整和更改了M10的scale和单位
////NMEA 0183 协议解析后数据存放结构体
//typedef struct  
//{
//	double latitude;				//纬度
//	double longitude;			    //经度
//	double hdop;					//水平精度因子 0~500,对应实际值0~50.0
//	uint8_t gpssta;					//GPS质量指示符:0,定位不可用或无效;1,单点定位;2,伪距差分;4, RTK固定解；5, RTK浮点解；6,GNSS/INS组合导航；7, 用户设定位置.				  

//	double alt;			 	     //天线高度,高于/低于平均海平面	 
//	double vx;					//ECEF坐标系下x轴速度,速度单位m/s
//	double vy;					//ECEF坐标系下y轴速度,速度单位m/s
//	double vz;					//ECEF坐标系下z轴速度,速度单位m/s
//}TNmeaFemtMes;

// 获取GPS秒脉冲信号
bool getGpsPpsSignal(void)
{
	 return g_bGpsPpsSignal;
}

// 获取GPS信号更新标志
bool getGpsUpdateFlg(void)
{
	 return g_bGpsUpdateFlg;
}

//从buf里面得到第cx个逗号所在的位置
//返回值:0~0XFE,代表逗号所在位置的偏移.
//       0XFF,代表不存在第cx个逗号							  
uint8_t NMEA_Comma_Pos(uint8_t *buf,uint8_t cx)
{	 		    
	uint8_t *p=buf;
	while(cx)
	{		 
		if(*buf=='*'||*buf<' '||*buf>'z')return 0XFF;//遇到'*'或者非法字符,则不存在第cx个逗号
		if(*buf==',')cx--;
		buf++;
	}
	return buf-p;	 
}
//m^n函数
//返回值:m^n次方.
uint32_t NMEA_Pow(uint8_t m,uint8_t n)
{
	uint32_t result=1;	 
	while(n--)result*=m;    
	return result;
}

//str转换为数字,以','或者'*'结束
//buf:数字存储区
//dx:小数点位数,返回给调用函数
//返回值:转换后的数值
int NMEA_Str2num(uint8_t *buf,uint8_t*dx)
{
	uint8_t *p=buf;
	uint32_t ires=0,fres=0;
	uint8_t ilen=0,flen=0,i;
	uint8_t mask=0;
	int res;
	while(1) //得到整数和小数的长度
	{
		if(*p=='-'){mask|=0X02;p++;}//是负数
		if(*p==','||(*p=='*'))break;//遇到结束了
		if(*p=='.'){mask|=0X01;p++;}//遇到小数点了
		else if(*p>'9'||(*p<'0'))	//有非法字符
		{	
			ilen=0;
			flen=0;
			break;
		}	
		if(mask&0X01)flen++;  // 从小数点处，flen开始计小数部分长度
		else ilen++;         //  ilen计整数部分长度
		p++;
	}
	
	if(mask&0X02)buf++;	//去掉负号
	for(i=0;i<ilen;i++)	//得到整数部分数据
	{  
		ires+=NMEA_Pow(10,ilen-1-i)*(buf[i]-'0');
	}
	
	if(flen>5)flen=5;	//最多取5位小数
	*dx=flen;	 		//小数点位数
	for(i=0;i<flen;i++)	//得到小数部分数据
	{  
		fres+=NMEA_Pow(10,flen-1-i)*(buf[ilen+1+i]-'0');
	} 
	res=ires*NMEA_Pow(10,flen)+fres;
	if(mask&0X02)res=-res;		   
	return res;
}

uint8_t iGetHexAsciiVal(char sChar)
{
	char HexAsciiArr[16] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
	uint8_t i;
	for(i = 0; i < 16; i++){
		if(tolower(sChar) == HexAsciiArr[i]){
			break;
		}
	}
	return(i);
}

uint8_t iGetCalVal(uint8_t charH,uint8_t charL)
{
	uint8_t iHexVal = 0, iByte;

	iByte = iGetHexAsciiVal((char)charH);
    if(iByte<16){
		iHexVal = iGetHexAsciiVal((char)charL);
		if(iHexVal < 16) iHexVal = iHexVal + (iByte<<4);
		else iHexVal = 0;
	}
	return iHexVal;
}
bool bChkNMEAFrameValid(uint8_t *pNmeaStr)
{
	bool bFlg = false;
	if(pNmeaStr != NULL) bFlg = true;
	return bFlg;
}

//分析GNGGA信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
void NMEA_GNGGA_Analysis(GpsInf *gpsx,uint8_t *buf)
{
	uint8_t *p1,dx;			 
	uint8_t posx;    
	p1=(uint8_t*)strstr((const char *)buf,"$GNGGA");
	
	if(bChkNMEAFrameValid(p1)){
	
	// 以下经纬度信息为AFC-4V4.01.220408版本添加
		uint32_t temp;	   
		uint8_t negFlg;
		double rs; 
		
    posx = NMEA_Comma_Pos(p1,1);   // 获取utc时间，对应hh/mm/s.ss
		if(posx!=0XFF){
			int itemp=NMEA_Str2num(p1+posx,&dx);
			if(itemp!=0){   // 丢星后，没收到信号时，连续两个逗号，返回数据为0则不进行处理，保留原来高度值。V1.13.200909
				gpsx->gps_time = itemp*pow(10,-dx);
			}						
		}
 	
		posx=NMEA_Comma_Pos(p1,3);								//南纬还是北纬 
		if(posx!=0XFF) negFlg=*(p1+posx);
		posx=NMEA_Comma_Pos(p1,2);								//得到纬度, 第2个逗号
		if(posx!=0XFF)
		{
			temp=NMEA_Str2num(p1+posx,&dx);
			if(temp!=0){   // 丢星后，没收到信号时，连续两个逗号，返回数据为0则不进行处理，保留原来纬度值。V1.13.200909
				gpsx->latitude=temp/NMEA_Pow(10,dx+2);	//得到°
				rs=temp%NMEA_Pow(10,dx+2);				//得到'
				gpsx->latitude = gpsx->latitude +(rs*pow(10,-dx))/60.;//转换为°
				if( toupper(negFlg) == 'S'){
					gpsx->latitude=-gpsx->latitude;
				}			
			}
		}
		
		posx=NMEA_Comma_Pos(p1,5);								//东经还是西经
		if(posx!=0XFF)negFlg=*(p1+posx);	
		posx=NMEA_Comma_Pos(p1,4);								//得到经度
		if(posx!=0XFF)
		{												  
			temp=NMEA_Str2num(p1+posx,&dx);		 	 
			if(temp!=0){   // 丢星后，没收到信号时，连续两个逗号，返回数据为0则不进行处理，保留原来经度值。V1.13.200909
				gpsx->longitude=temp/NMEA_Pow(10,dx+2);	//得到°
				rs=temp%NMEA_Pow(10,dx+2);				//得到'		 
				gpsx->longitude=gpsx->longitude+(rs*pow(10,-dx))/60.0;//转换为° 
				if( toupper(negFlg) == 'W'){
					gpsx->longitude = -gpsx->longitude;
				}
			}
		}
		
		posx=NMEA_Comma_Pos(p1,6);								//得到GPS质量指示符 GPS qual
		if(posx!=0XFF)gpsx->gpssta=NMEA_Str2num(p1+posx,&dx);
	
		posx=NMEA_Comma_Pos(p1,7);								//得到GPS卫星数量 sats
		if(posx!=0XFF)gpsx->num_sats=NMEA_Str2num(p1+posx,&dx);	

		
		int itemp;
		posx=NMEA_Comma_Pos(p1,8);								//得到水平精度因子hdop
		if(posx!=0XFF){
			itemp = NMEA_Str2num(p1+posx,&dx);
			if(itemp!=0){   // 丢星后，没收到信号时，连续两个逗号，返回数据为0则不进行处理，保留原来高度值。V1.13.200909
				gpsx->hdop = itemp*pow(10,-dx);
			}			
		}
		
		posx=NMEA_Comma_Pos(p1,9);								//  得到天线高度，高于/低于平均海平面，单位m
		if(posx!=0XFF){
			itemp = NMEA_Str2num(p1+posx,&dx);
			if(itemp!=0){   // 丢星后，没收到信号时，连续两个逗号，返回数据为0则不进行处理，保留原来高度值。V1.13.200909
				gpsx->alt = itemp*pow(10,-dx);             
			}			
		}
	}
}

//分析GNDHV信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
void NMEA_GNDHV_Analysis(GpsInf *gpsx,uint8_t *buf)
{
	uint8_t *p1,dx;			 
	uint8_t posx;    
	p1=(uint8_t*)strstr((const char *)buf,"$GNDHV");
	
	if(bChkNMEAFrameValid(p1)){
	
	// 以下经纬度信息为AFC-4V4.01.220408版本添加
		int temp;
//		float rs;  
		posx=NMEA_Comma_Pos(p1,3);								// 第3个逗号后的数据为ECEF坐标系下的X轴速度
		if(posx!=0XFF){
			temp = NMEA_Str2num(p1+posx,&dx);
			if(temp!=0){   // 丢星后，没收到信号时，连续两个逗号，返回数据为0则不进行处理，保留原来高度值。V1.13.200909
				gpsx->vx = temp*pow(10,-dx);
			}			
		}
		
		posx=NMEA_Comma_Pos(p1,4);								// 第4个逗号后的数据为ECEF坐标系下的y轴速度
		if(posx!=0XFF){
			temp = NMEA_Str2num(p1+posx,&dx);
			if(temp!=0){   // 丢星后，没收到信号时，连续两个逗号，返回数据为0则不进行处理，保留原来高度值。V1.13.200909
				gpsx->vy = temp*pow(10,-dx);
			}			
		}
		
		posx=NMEA_Comma_Pos(p1,5);								// 第5个逗号后的数据为ECEF坐标系下的Z轴速度, 第6个逗号
		if(posx!=0XFF){
			temp = NMEA_Str2num(p1+posx,&dx);
			if(temp!=0){   // 丢星后，没收到信号时，连续两个逗号，返回数据为0则不进行处理，保留原来高度值。V1.13.200909
				gpsx->vz = temp*pow(10,-dx);
			}			
		}
		
//	  posx=NMEA_Comma_Pos(p1,6);								//得到ECEF坐标系下的地表速度（水平速度）, 第7个逗号
//		if(posx!=0XFF){
//			temp = NMEA_Str2num(p1+posx,&dx);
//			if(temp!=0){   // 丢星后，没收到信号时，连续两个逗号，返回数据为0则不进行处理，保留原来高度值。V1.13.200909
//				gpsx->ground_speed = temp*pow(10,-dx);
//			}
//		}

	}
}

//分析GNRMC信息: 输出卫星定位状态、GNSS航迹角和地速
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
void NMEA_GNRMC_Analysis(GpsInf *gpsx,uint8_t *buf)
{
	uint8_t *p1,dx;			 
	uint8_t posx;    
	p1=(uint8_t*)strstr((const char *)buf,"$GNRMC");
	
	if(bChkNMEAFrameValid(p1)){
	
	// 以下为GNSS航迹角与地速输出信息
		int temp;
		posx=NMEA_Comma_Pos(p1,8);								//得到地速, 单位节，第8个逗号
		if(posx!=0XFF){
			temp = NMEA_Str2num(p1+posx,&dx);
			if(temp!=0){   // 丢星后，没收到信号时，连续两个逗号，返回数据为0则不进行处理，保留原来值。
				gpsx->ground_speed = temp*pow(10,-dx)*0.5144;   // 转换为m/s
			}			
		}
		
		posx=NMEA_Comma_Pos(p1,9);								//得到真北航迹角，用°表示的, 第9个逗号
		if(posx!=0XFF){
			temp = NMEA_Str2num(p1+posx,&dx);
			if(temp!=0){   // 丢星后，没收到信号时，连续两个逗号，返回数据为0则不进行处理，保留原来高度值。V1.13.200909
				gpsx->ground_course_cd = temp*pow(10,-dx);
			}			
		}
	}
}


//分析GPVTG信息: 输出GNSS航迹角和地速
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
void NMEA_GPVTG_Analysis(GpsInf *gpsx,uint8_t *buf)
{
	uint8_t *p1,dx;			 
	uint8_t posx;    
	p1=(uint8_t*)strstr((const char *)buf,"$GNVTG");
	
	if(bChkNMEAFrameValid(p1)){
	
	// 以下为GNSS航迹角与地速输出信息
		int temp;
//		float rs;
		posx=NMEA_Comma_Pos(p1,1);								// 第1个逗号后为真北航迹角, 第2个逗号
		if(posx!=0XFF){
			temp = NMEA_Str2num(p1+posx,&dx);
			if(temp!=0){   // 丢星后，没收到信号时，连续两个逗号，返回数据为0则不进行处理，保留原来值。
				gpsx->ground_course_cd = temp*pow(10,-dx);
			}			
		}
		
		posx=NMEA_Comma_Pos(p1,7);								// 第6个逗号后得到地速，用km/hr表示的, 第8个逗号
		if(posx!=0XFF){
			temp = NMEA_Str2num(p1+posx,&dx);
			if(temp!=0){   // 丢星后，没收到信号时，连续两个逗号，返回数据为0则不进行处理，保留原来高度值。V1.13.200909
				gpsx->ground_speed = temp*pow(10,-dx);
			}			
		}
	}
}

//提取NMEA-0183信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
void GPS_Analysis(uint8_t *buf)
{
//	HAL_GPIO_WritePin(TState_GPIO_Port,TState_Pin,GPIO_PIN_SET);     // LED_Pin状态位复位时，示波器观察的信号为高电平
	NMEA_GNGGA_Analysis(&g_sGpsInf,buf);
	NMEA_GNDHV_Analysis(&g_sGpsInf,buf);
//	NMEA_GNRMC_Analysis(&g_sGpsInf,buf);
	NMEA_GPVTG_Analysis(&g_sGpsInf,buf);

	g_sGpsInf.status = GPS_OK_FIX_3D_RTK;

//	HAL_GPIO_WritePin(TState_GPIO_Port,TState_Pin,GPIO_PIN_RESET);     // LED_Pin状态位复位时，示波器观察的信号为高电平	
}
/************************ (C) COPYRIGHT ACG Co. about ATKS1216F8 *****END OF FILE****/
