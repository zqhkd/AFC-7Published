/******************** (C) COPYRIGHT 2017 ACE Tech Co.*************************
 * 作    者 ： 曾庆华、贺俊达
 * 文 件 名 ： AFCTofCom.c
 * 描    述 ： Tof串口协议解析函数
 * 版    本 ：
 *     V1.01.211116 
 *            
 * 官    网 ：www.acecreator.com
 * 淘    宝 ：acecreator.taobao.com
 * 公 众 号 ：无人飞行控制
*****************************************************************************/
#include "AFCTofCom.h"
#include "AFCGlobalVar.h"
#include "ACGCommonAPI.h"

// TofCom最大字符字节数
//#define MAX_TofCom_Rx_SIZE        20
#define MAX_TofCom_Rx_SIZE        90
uint8_t _g_sTofComRxBuf[MAX_TofCom_Rx_SIZE];
bool g_bUsedOfTOF03 = false;

typedef struct{
	uint16_t Heads;
	uint16_t Distance;
	uint16_t SignalStrength;
	uint16_t Reserved;
	uint8_t  Tail;
	bool     bValidFlg;
}TTofFrame;

TTofFrame _g_sTofFrame;
uint8_t g_iSlideNumOfTof03;
bool g_bTofComRxBufIsIdle = true;

void UpdateTof03Data(uint8_t *buf)
{
	for(uint8_t i = 0; i < MAX_TofCom_Rx_SIZE; i++){
		if((_g_sTofComRxBuf[i] == 0x59) && (_g_sTofComRxBuf[i+1] == 0x59) && (i < MAX_TofCom_Rx_SIZE - 8)){
			_g_sTofFrame.Heads          = _g_sTofComRxBuf[i] + (_g_sTofComRxBuf[i+1]<<8);
			_g_sTofFrame.Distance       = _g_sTofComRxBuf[i+2] + (_g_sTofComRxBuf[i+3]<<8);
			_g_sTofFrame.SignalStrength = _g_sTofComRxBuf[i+4] + (_g_sTofComRxBuf[i+5]<<8);
			_g_sTofFrame.Reserved       = _g_sTofComRxBuf[i+6] + (_g_sTofComRxBuf[i+7]<<8);
			_g_sTofFrame.Tail           = _g_sTofComRxBuf[i+8];
			
			_g_sTofFrame.bValidFlg      = true;   // 置位数据有效标志

			break;
		}
	}
}

void ProTofComRcvData()
{
	uint32_t isrflags   = READ_REG(TofCom.Instance->ISR);
    uint32_t cr1its     = READ_REG(TofCom.Instance->CR1);
    bool bClearOverFlag = true;
	
    if(((isrflags & USART_ISR_IDLE) != RESET) && ((cr1its & USART_CR1_IDLEIE) != RESET) && ((isrflags & USART_ISR_ORE) != USART_ISR_ORE)){
	    __HAL_UART_CLEAR_IDLEFLAG(&TofCom);
	    uint32_t _len_dmarev = MAX_TofCom_Rx_SIZE - __HAL_DMA_GET_COUNTER(TofCom.hdmarx);
	    if(_len_dmarev){
//	        HAL_UART_DMAStopRx(&TofCom);
					/* 这里以前使用HAL_UART_DMAStop(huart)，但这个函数会导致TX的DMA被关闭，小概率DMA发送会丢数据 */
					/* 2021.10.24 修改为HAL_UART_AbortReceive(huart)，对应DMA发送也需要做一些处理 */
					HAL_UART_AbortReceive(&TofCom);
			
				  if(g_bTofComRxBufIsIdle)	UpdateTof03Data(_g_sTofComRxBuf);
					
	        HAL_UART_Receive_DMA(&TofCom, _g_sTofComRxBuf, MAX_TofCom_Rx_SIZE); 
				
	        __HAL_UART_DISABLE_IT(&TofCom, UART_IT_ERR);
	        __HAL_UART_DISABLE_IT(&TofCom, UART_IT_PE);
			
        // 不用清除溢出标志，其它情况都需要清除			
		    bClearOverFlag = false;
	    }
    }
	if(bClearOverFlag){
	   READ_REG(TofCom.Instance->ISR);
	   READ_REG(TofCom.Instance->RDR);
	   __HAL_UART_CLEAR_OREFLAG(&TofCom);
	   __HAL_UART_CLEAR_IDLEFLAG(&TofCom);
	}
}

// AFC-4V4.02.220318: 添加TOF-03激光雷达接口
double getTofData(uint8_t iChannel)
{
	double fVal;
	
// V4.02.220519: 增加滑动滤波处理
	static double fMoveArr[MAX_SLIDE_WIN_LEN];
	static uint8_t iPos={0};
	static double fOut = {0};
	static bool bSlideWinFull = {false};
	g_bTofComRxBufIsIdle = false;
	switch(iChannel){
		case 0:
			  fVal = (double)g_sRealTimeCount.fcsTime;   // 传感器采集时的第一个数据为采集时刻的控制器本地时标值
		    break;
		case 1:
		// V4.02.220519: 增加滑动滤波处理
			if(g_iSlideNumOfTof03!=0){
				vSlideWinAverage(fMoveArr,g_iSlideNumOfTof03,&iPos,_g_sTofFrame.Distance,&fOut,&bSlideWinFull);
				_g_sTofFrame.Distance = fOut;
			}
			fVal = _g_sTofFrame.Distance * 0.01;      // _g_sTofFrame.bValidFlg为假时，信号值为前面刷新时的老数据单位为cm，20240425乘以0.01转化为m。
			break;
		case 2:
		// 信号强度，取值范围10~3500。信号强度低于40，TF03将输出超量程值。正常测距时，信号强度变化范围为40~1200；当测量高反被测物时，信号强度会超过1500。	
			fVal = _g_sTofFrame.SignalStrength;
			break;
	}
	g_bTofComRxBufIsIdle = true;
	return fVal;
}

void initTof(uint8_t iSlideNum)
{
	InitComDMA(&TofCom,_g_sTofComRxBuf,MAX_TofCom_Rx_SIZE);
	g_bUsedOfTOF03 = true;
	g_iSlideNumOfTof03 = iSlideNum;
}
/************************ (C) COPYRIGHT ACG co. *****END OF FILE****/
