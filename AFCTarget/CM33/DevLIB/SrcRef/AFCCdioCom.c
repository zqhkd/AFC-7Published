/******************** (C) COPYRIGHT 2021 AFC Tech Co.*************************
 * 作    者  ： 曾庆华
 * 文 件 名  ：AFCCdioCom.c
 * 版    本  ：
 *        AFC-5V5.03.240128: 
 *             
 * 描    述  ：数传模块串口接口函数
 * 
 * 官    网  ：www.acecreator.com
 * 淘    宝  ：acecreator.taobao.com
 * 公 众 号  ：飞行控制与仿真
 *
*****************************************************************************/
#include "AFCCdioCom.h"
#include "ACGCommonAPI.h"
#include "AFCGlobalVar.h"

#define CdioComChkMode       SUM_CHECK

#define MaxCdioInFrameTypeNum   10
#define MaxCdioOutFrameTypeNum  20
#define MaxSignalNumOfCdioFrame 50         // 假设串口波特率为115200bps, 单帧1ms约传输11byte, 则10ms约传输110byte, 约27个浮点数，因此需要注意传输数据数量与波特率的匹配性

// 外部可使用全局变量
uint8_t  g_sCdioComTxBuf[CdioCom_MAX_Tx_SIZE]; 		// 单帧DMA发送缓冲,  最大CdioCom_MAX_Tx_SIZE字节
int8_t  g_curCdioComRcvLen;

uint8_t g_iMaxCdioOutFrameNum = 0;
uint8_t g_iCdioOutFrameType[MaxCdioOutFrameTypeNum];   // 输出帧的帧类型码或帧序列码
uint8_t g_idxCurCdioOutFrame;
int8_t g_iCdioOutSignalNum[MaxCdioOutFrameTypeNum];
float g_fCdioOutData[MaxCdioOutFrameTypeNum][MaxSignalNumOfCdioFrame];    // 通过CdioCom串口输出的参数

uint8_t g_idxCurCdioFrame=0;

void ClrCdioComSignalNum(void)
{
	uint8_t i;
	for(i = 0; i < MaxCdioOutFrameTypeNum; i++) g_iCdioOutSignalNum[i] = -1;
}

// 初始化模型输出串口
void initCdioComOut(uint8_t iSigNum,uint8_t iFrameType)
{
	  if(iSigNum > MaxSignalNumOfCdioFrame) iSigNum = MaxSignalNumOfCdioFrame;
	  g_iCdioOutSignalNum[g_iMaxCdioOutFrameNum] = iSigNum;
	  g_iCdioOutFrameType[g_iMaxCdioOutFrameNum] = iFrameType;

	  for(uint8_t i = 0; i < iSigNum; i++) g_fCdioOutData[g_iMaxCdioOutFrameNum][i] = 0.f;
	
	  g_iMaxCdioOutFrameNum++;   // 输出帧数量
}

int iFindCdioOutFrameIdx(uint8_t iFrameType)
{
	  return iFindIdBuffIdx(iFrameType,g_iCdioOutFrameType,g_iMaxCdioOutFrameNum);
}
	
// 通过模型端口发送信息，DMA方式
void DataDMA2CdioCom(uint8_t idxCurFrame)
{
	  uint16_t len;
		THeaderFrame pHeaderFrame;
		pHeaderFrame.iFrameTick = g_sRealTimeCount.fcsTime;
		pHeaderFrame.idSender = ID_Of_FCS;
		pHeaderFrame.idReceiver = ID_OF_GCS;
	
		pHeaderFrame.iFrameType = g_iCdioOutFrameType[idxCurFrame];  // 发送给CdioCom的帧类型码
			
// 原来为虚函数，需用它才能将外部虚函数实化。
		len = iPackData2Hex(pHeaderFrame,g_iCdioOutSignalNum[idxCurFrame],g_fCdioOutData[idxCurFrame],g_sCdioComTxBuf,SUM_CHECK); 
		
    //开始发送数据
    HAL_UART_Transmit_DMA(&CdioCom,g_sCdioComTxBuf,len);
}

// 传递指定参数给模型串口
void writeCdioCom(uint8_t iFrameType,uint8_t iChannel,double fVal)
{
	 static int iSigNum = 0;
	 if(g_idxCurCdioFrame == iFindCdioOutFrameIdx(iFrameType)){
		 if(g_iCdioOutSignalNum[g_idxCurCdioFrame] >=0){
			 if(iChannel < g_iCdioOutSignalNum[g_idxCurCdioFrame]){
					g_fCdioOutData[g_idxCurCdioFrame][iChannel] = fVal;
					iSigNum++;
					if(iSigNum >= g_iCdioOutSignalNum[g_idxCurCdioFrame]){
						// 当前帧的最后一个变量，则将当前帧数据打包输出，且将帧序号清0
//						g_idCurCdioOutFrame = iFrameType;
						DataDMA2CdioCom(g_idxCurCdioFrame);
						
						iSigNum = 0;
					}
			 }
		 }
	 }
}
/************************ (C) COPYRIGHT ACG co. *****END OF FILE****/
