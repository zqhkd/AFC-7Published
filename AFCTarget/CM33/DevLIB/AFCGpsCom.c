/******************** (C) COPYRIGHT 2021 AFC Tech Co.*************************
 * 作    者  ： 曾庆华
 * 文 件 名  ：AFCGpsCom.c
 * 版    本  ：
 *        AFC-5V5.02.231016: 基于DMA串口中断方式进行改写 
 *             
 * 描    述  ：GPS模块串口接口函数
 * 
 * 官    网  ：www.acecreator.com
 * 淘    宝  ：acecreator.taobao.com
 * 公 众 号  ：飞行控制与仿真
 *
*****************************************************************************/
#include "AFCGpsCom.h"
#include "FemtMesGps.h" 
#include "ACGCommonAPI.h"
#include "AFCGlobalVar.h"
#include "ACGRingBuffAPI.h"

#define GPS_DEV_M10   11
#define GPS_DEV_NANO  21
uint8_t g_iGpsDev;

uint8_t  g_sGpsComRxBuf[GpsCom_MAX_Rx_SIZE]; 		// 单帧串口中断接收缓冲,  最大SimuCom_MAX_Rx_SIZE字节
bool g_bGpsUpdateFlg,g_bGpsPpsSignal;
GpsInf g_sGpsInf;

TRingBuffer *lpGpsRingBuff;

__weak void GpsInf2ModelVar(void)
{
}

__weak void GpsPpsSignal2ModelVar(void)
{
}

void ExtGpsCom2ModelVar(void)
{
	GpsInf2ModelVar();
}

uint8_t gps_bytes[GpsCom_MAX_Rx_SIZE];

// 为保证FemtMesGps和ublox M10的兼容性，全面调整和更改了M10的scale和单位, 
//   即调整unPackUBloxInf()函数, 使得g_sGpsInf结构体内各个成员变量与FemtMesGps一致。
void unPackUBloxInf(void)
{
		//北京时间
		g_sGpsInf.gps_time = gps_bytes[10] + gps_bytes[9]*60 + gps_bytes[8]*3600;
		//定位类型
		if(gps_bytes[20] == 0x00)
			g_sGpsInf.status = (GPS_Status)0x01;
		else
			g_sGpsInf.status = (GPS_Status)gps_bytes[20];
		
		// 手册记录为：第21字节为diffSoln，1表示进入了差分校正（DGPS或RTK）,此时为RTK浮点解分米级，而2表示RTK固定解Fixed，厘米级
		// 20250704实测结果：第21字节，上电时为1，表示单点GPS；为0x43，表示进入浮点差分校正（DGPS或RTK）,精度分米级；为0x83, 表示RTK固定解Fixed，厘米级
		if(gps_bytes[21] == 0x83)  // zqinghua于20250704更改为0x83
			g_sGpsInf.status = (GPS_Status)0x04;
		if(gps_bytes[21] == 0x43)  // zqinghua于20250704更改为0x43
			g_sGpsInf.status = (GPS_Status)0x05;
		
		//卫星个数
		g_sGpsInf.num_sats = gps_bytes[23];
		
		//经度, 原始数据scale为1e-7,单位°
		uint32_t temp = (uint32_t)gps_bytes[24]|(uint32_t)(gps_bytes[25]<<8)|(uint32_t)(gps_bytes[26]<<16)|(uint32_t)(gps_bytes[27]<<24);
		g_sGpsInf.longitude = (double) temp*1e-7;    // 转换为度
		//纬度, 原始数据scale为1e-7，单位°
		temp = (uint32_t)gps_bytes[28]|(uint32_t)(gps_bytes[29]<<8)|(uint32_t)(gps_bytes[30]<<16)|(uint32_t)(gps_bytes[31]<<24);
		g_sGpsInf.latitude = (double) temp*1e-7;   // 转换为度
		// 海平面以上高度hMSL, 原始数据单位mm
		temp = (uint32_t)gps_bytes[36]|(uint32_t)(gps_bytes[37]<<8)|(uint32_t)(gps_bytes[38]<<16)|(uint32_t)(gps_bytes[39]<<24);
        g_sGpsInf.alt = temp*0.001f;    // 转换为m

    int32_t tempInt32;
		//正北速度  mm/s
		tempInt32= (int32_t)((uint32_t)gps_bytes[48]|(uint32_t)(gps_bytes[49]<<8)|(uint32_t)(gps_bytes[50]<<16)|(uint32_t)(gps_bytes[51]<<24));
		g_sGpsInf.vx = tempInt32*0.001f;  // 转换为m/s

		//正东速度  mm/s
		tempInt32 = (int32_t)((uint32_t)gps_bytes[52]|(uint32_t)(gps_bytes[53]<<8)|(uint32_t)(gps_bytes[54]<<16)|(uint32_t)(gps_bytes[55]<<24));
		g_sGpsInf.vy = tempInt32*0.001f;  // 转换为m/s

		//向下速度  mm/s  
		tempInt32 = (int32_t)((uint32_t)gps_bytes[56]|(uint32_t)(gps_bytes[57]<<8)|(uint32_t)(gps_bytes[58]<<16)|(uint32_t)(gps_bytes[59]<<24));
		g_sGpsInf.vz = tempInt32*0.001f;  // 转换为m/s
		
		//对地速度  mm/s
		temp = ((uint32_t)gps_bytes[60]|(uint32_t)(gps_bytes[61]<<8)|(uint32_t)(gps_bytes[62]<<16)|(uint32_t)(gps_bytes[63]<<24));
		g_sGpsInf.ground_speed = temp*0.001f;  // 转换为m/s
		
		//航向角 scale为1e-5, 单位deg
		g_sGpsInf.ground_course_cd = (uint32_t)gps_bytes[64]|(uint32_t)(gps_bytes[65]<<8)|(uint32_t)(gps_bytes[66]<<16)|(uint32_t)(gps_bytes[67]<<24);
		g_sGpsInf.ground_course_cd *= 1e-5f;

		//位置精度因子, scale为0.01
		g_sGpsInf.hdop = ((uint16_t)gps_bytes[76]|((uint16_t)gps_bytes[77]<<8))*0.01f;
}

// 从环形缓冲区中提取帧内容，解析GpsCom的传送数据
void vRcvUBloxInfTask(void)
{
	static uint8_t iState = 0;
	static uint8_t pBuff[GpsCom_MAX_Rx_SIZE];
	static bool bExitFlg = false;
	
	uint8_t gps_ck_a, gps_ck_b,data;
	uint8_t gps_payload_length, gps_payload_counter;

	while(!(bExitFlg || RingBufferEmpty(lpGpsRingBuff)))		// 判断环形缓冲区是否为空,或者是退出标志？
	{
	    // 非空，则开始处理数据
		RingBufferGet(lpGpsRingBuff,pBuff,1);
		data = pBuff[0];
		switch (iState)
    {
			// 开始判断帧头第1字节。UBlox协议（又称为PVT协议）
      case 0:
				if(data == 0xB5){
					iState++;
				}
      	break;
			// 开始判断帧头第2字节
      case 1:
				if(data == 0x62){
					iState++;
				}
				else iState = 0;   // 如果第2字节不是帧头，则返回至状态0，重新开始寻找帧头
      	break;
			case 2:
//			   gps_class = data;
			   gps_ck_b = gps_ck_a = data;
			   iState++;
			   break;
			case 3:
			// checksum byte
			   gps_ck_b += (gps_ck_a += data);
//			   gps_msg_id = data;
			 
				 if( data == 0x07){   // 检查结束符。第3字节为消息ID, 0x07表示PVT消息
					 iState++;
				 }
					else{
						iState = 0;
					}
					break;
			case 4:
				 iState++;
				 gps_ck_b += (gps_ck_a += data);                   // checksum byte
				 gps_payload_length = data;              // payload length low byte
			   break;
			case 5:
			   iState++;
			   gps_ck_b += (gps_ck_a += data);                   // checksum byte

				 gps_payload_length += (uint16_t)(data<<8);
				 if (gps_payload_length > 120) {
						gps_payload_length = 0;
						iState = 0;
				 }
				 gps_payload_counter = 0;                               // prepare to receive payload
				break;
					// Receive message data
			case 6:
				gps_ck_b += (gps_ck_a += data);                   // checksum byte
  			if (gps_payload_counter <  sizeof(gps_bytes)) {
					gps_bytes[gps_payload_counter] = data;
				}
				if (++gps_payload_counter == gps_payload_length)
					iState++;
				break;
				// Checksum and message processing
			case 7:
				iState++;
  			if (gps_ck_a != data) {
					iState = 0;
				}
				break;
			case 8:
				iState = 0;
			  if(gps_ck_b == data) {
					g_bGpsUpdateFlg = true;
					unPackUBloxInf();
					ExtGpsCom2ModelVar();
				}
				break;
      default:
     		break;
     }
	}
	bExitFlg = false;
}

void vRcvNanoGnssInfTask(void)
{
	GPS_Analysis(g_sGpsComRxBuf);
	ExtGpsCom2ModelVar();   // 将解析的数据传递给模型变量
}

// 处理GpsCom的空闲中断DMA：接收GpsCom字符串
void ProGpsComRcvIRQ(void)  
{  
	  uint32_t isrflags   = READ_REG(GpsCom.Instance->ISR);
    uint32_t cr1its     = READ_REG(GpsCom.Instance->CR1);
    bool bClearOverFlag = true;
	
    if(((isrflags & USART_ISR_IDLE) != RESET) && ((cr1its & USART_CR1_IDLEIE) != RESET) && ((isrflags & USART_ISR_ORE) != USART_ISR_ORE)){
	    __HAL_UART_CLEAR_IDLEFLAG(&GpsCom);
	    uint32_t _len_dmarev = GpsCom_MAX_Rx_SIZE - __HAL_DMA_GET_COUNTER(GpsCom.hdmarx);
	    if(_len_dmarev){
//            HAL_UART_DMAStopRx(&GpsCom);
			/* 这里以前使用HAL_UART_DMAStop(huart)，但这个函数会导致TX的DMA被关闭，小概率DMA发送会丢数据 */
			/* 2021.10.24 修改为HAL_UART_AbortReceive(huart)，对应DMA发送也需要做一些处理 */
			HAL_UART_AbortReceive(&GpsCom);
//			HAL_UART_Transmit_DMA(&ModelCom,g_sGpsComRxBuf,_len_dmarev);
			
			if(bInitGpsComIn){
				  if(g_iGpsDev == GPS_DEV_M10){
						// 向环形缓冲区写数据
						RingBufferPut(lpGpsRingBuff,g_sGpsComRxBuf,_len_dmarev);
						vRcvUBloxInfTask();
					}
					else{
						vRcvNanoGnssInfTask();
					}
		
	        HAL_UART_Receive_DMA(&GpsCom, g_sGpsComRxBuf, GpsCom_MAX_Rx_SIZE); 
			}
				
			__HAL_UART_DISABLE_IT(&GpsCom, UART_IT_ERR);
			__HAL_UART_DISABLE_IT(&GpsCom, UART_IT_PE);
			
        // 不用清除溢出标志，其它情况都需要清除			
		    bClearOverFlag = false;
	    }
    }
	if(bClearOverFlag){
	   READ_REG(GpsCom.Instance->ISR);
	   READ_REG(GpsCom.Instance->RDR);
		
		// 外部串口设备反复启动时，会导致溢出中断。如果用以下语句清空闲中断，会导致溢出中断反复被置位。
//	   __HAL_UART_CLEAR_IDLEFLAG(&GpsCom);    // 为防止溢出标志重置位，必须将该语句__HAL_UART_CLEAR_IDLEFLAG置于__HAL_UART_CLEAR_OREFLAG之前，否则，一旦退出该中断服务程序就会被重新置位
	   __HAL_UART_CLEAR_OREFLAG(&GpsCom);
	}
}

// 初始化DMA模式的GpsCom(huart2)。最关键问题是打开空闲中断
void initGps(uint8_t iGpsDev)
{
	g_bGpsUpdateFlg = false;
	bInitGpsComIn = true;
	if(iGpsDev==0){
		// 初始化数传串口接收DMA
		g_iGpsDev = GPS_DEV_M10;     // 表示为Blox M10 GNSS设备
		if(lpGpsRingBuff==NULL){
				lpGpsRingBuff = InitRingBuffer(lpGpsRingBuff,GpsCom_MAX_Rx_SIZE*2);
		}
	}
	else
		g_iGpsDev = GPS_DEV_NANO;   // 表示为NANO GNSS设备
	
	InitComDMA(&GpsCom,g_sGpsComRxBuf,GpsCom_MAX_Rx_SIZE);
}

// 获取GPS空间位置导航信息
double getGpsData(uint8_t iChannel)
{
	double fVal;
	switch(iChannel){
		case 0:
			fVal = g_sGpsInf.gps_time;
		  break;
		case 1:
			fVal = g_sGpsInf.longitude;
		  break;
		case 2:
			fVal = g_sGpsInf.latitude;
		  break;
		case 3:
			fVal = g_sGpsInf.alt;
		  break;
		case 4:
			fVal = g_sGpsInf.vx;
		  break;
		case 5:
			fVal = g_sGpsInf.vy;
		  break;
		case 6:
			fVal = g_sGpsInf.vz;
		  break;
		case 7:
			fVal = g_sGpsInf.ground_speed;
		  break;
		case 8:
			fVal = g_sGpsInf.ground_course_cd;
		  break;
		case 9:
			if(g_iGpsDev == GPS_DEV_M10)		fVal = g_sGpsInf.status;
		  else fVal = g_sGpsInf.gpssta;       // 北天GPS的NMEA0183协议xxGGA中的第6段quality
		  break;
		case 10:
			fVal = g_sGpsInf.num_sats;
		  break;
		case 11:
			fVal = g_sGpsInf.hdop;
		  break;
		case 12:
			fVal = g_sRealTimeCount.fcsTime;
		  break;
		default:
			fVal = 0;
		  break;
	}
	return fVal;
}

/************************ (C) COPYRIGHT ACG co. *****END OF FILE****/
