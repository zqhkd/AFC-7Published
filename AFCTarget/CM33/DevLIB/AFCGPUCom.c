/******************** (C) COPYRIGHT 2017 ACE Tech Co.*************************
 * 
 * 作    者  ： 曾庆华
 * 文 件 名  ：AFCGpuCom.c
 * 版    本  ：
 *             AFC-3V1.02.2100807
 * 描    述  ：英伟达AGX模块GpuCom串口接口函数
 * 官    网  ：www.acecreator.com
 * 淘    宝  ：acecreator.taobao.com
 * 公 众 号  ：无人飞行控制
 *
*****************************************************************************/
#include "usart.h"
#include "stm32h7xx.h"
#include <stdio.h>
#include <string.h>

#include "AFCGlobalVar.h"
#include "ACGCommonAPI.h"
#include "AFCGpuCom.h"

// GpuCom串口常数
#define GpuCom_MAX_Tx_SIZE  20              // V3.06.220127: 由于GPU数据处理时，内存空间出现泄漏，由120更改为20
#define GpuCom_MAX_Rx_SIZE  20              // V3.06.220127: 由于GPU数据处理时，内存空间出现泄漏，由120更改为20
#define MAX_SIGNAL_NUM_OF_GpuCom 30

// AFC-3V1.02.210807 增加GpuCom串口缓冲区   
uint8_t  g_sGpuComTxBuf[GpuCom_MAX_Tx_SIZE]; 		// DMA接收缓冲,  最大GpuCom_MAX_Tx_SIZE字节
uint8_t  g_sGpuComRxBuf[GpuCom_MAX_Rx_SIZE]; 		// DMA接收缓冲,  最大GpuCom_MAX_Rx_SIZE字节

uint8_t g_iGpuOutSignalNum;      // GPUCom端口输出信号数量
uint8_t g_iGpuInSignalNum;       // GPUCom端口输入信号数量
uint8_t g_iGPUOutFrameType,g_iGPUInFrameType; // AFC-2V1.01.210407：增加发送帧的帧类型码
float g_fOutGpuData[MAX_SIGNAL_NUM_OF_GpuCom];          // 通过GpuCom串口输出的参数
bool g_bGpuComOutFiniedFlg;     // 测试串口数据发送已完成标志
bool g_bGpuComDataReady;        // 控制系统已更新ModelCom串口监测数据的标志
float g_fInGpuData[MAX_SIGNAL_NUM_OF_GpuCom];     // 模型串口输入变量
bool g_bGpuComInputed = false, g_bUsedOfGpuCom = false;

uint16_t iPackGpuData2Hex(void)
{
	// 计算HEX传输时所需字符串长度，该字符串为：$TC,f1,f2,...,fn,*v  
	uint16_t len = sizeof(THeaderFrame) + g_iGpuOutSignalNum * sizeof(float) + 1;    // 累加和校验和占1个字节，CRC占2字节

	THeaderFrame pHeaderFrame;
	pHeaderFrame.iFrameID = FRAME_HEADER_ID;
	pHeaderFrame.iFrameLen = len;
	pHeaderFrame.iFrameTick = g_sRealTimeCount.fcsTime;
	pHeaderFrame.idSender = getCurUAVId();             // 
//	pHeaderFrame.idReceiver = ID_OF_GCS;
	pHeaderFrame.iFrameType = g_iGPUOutFrameType;

	uint16_t i,iHeader;
	for(i = 0; i < sizeof(THeaderFrame); i++)
	{
		 g_sGpuComTxBuf[i] = ((uint8_t *)(&pHeaderFrame))[i];
	}
	iHeader = i;

	for(i = 0; i < g_iGpuOutSignalNum;i++){
		g_sGpuComTxBuf[iHeader++] = ((uint8_t *)(&g_fOutGpuData[i]))[0];   // 将GPU端口数据转成在输出缓冲区中
		g_sGpuComTxBuf[iHeader++] = ((uint8_t *)(&g_fOutGpuData[i]))[1];
		g_sGpuComTxBuf[iHeader++] = ((uint8_t *)(&g_fOutGpuData[i]))[2];
		g_sGpuComTxBuf[iHeader++] = ((uint8_t *)(&g_fOutGpuData[i]))[3];
	}
	
	g_sGpuComTxBuf[iHeader++] = 	CalChkSum(g_sGpuComTxBuf,len-1);

	return len;
}

//#define __TEST_GPU_DMA_TRANSMIT_WIDTH__
void DMAWriteStr2GpuCom(uint16_t len)
{
	g_bGpuComOutFiniedFlg = false;   // 该语句仅用于传输完成标志检测运行方式使用，确保GpuCom发送前置假。其它仅赋值无实际意义
	HAL_UART_Transmit_DMA(&GpuCom,g_sGpuComTxBuf,len);
	g_bGpuComDataReady = false;        // 模型串口数据一经输出，则清模型串口输出数据刷新标志
}

// 通过模型端口发送信息，DMA方式
void DMAWriteData2GpuCom(void)
{
   DMAWriteStr2GpuCom(iPackGpuData2Hex());
}

// 采用DMA模式接收GpuCom的信息任务
void vRcvGpuComInfTask(uint32_t len)
{
	uint8_t i,j,iVarNum;
	uint16_t iFrameLen;
    TChar2FloatStruct pData;
	
	if(len > FRAME_HEADER_LENGTH){
		iFrameLen = g_sGpuComRxBuf[FrameLengthPos] + (g_sGpuComRxBuf[FrameLengthPos + 1] << 8);
		if(bChkFrameValid(g_sGpuComRxBuf,SUM_CHECK,iFrameLen)){
		 // GPU串口监测数据，最大g_iGpuInSignalNum字节
			 // 用户实验模型的串口监测。通过Simulink工具箱中的ReadMC读取GpuCom数据
		   iVarNum = (uint8_t)((iFrameLen - (sizeof(THeaderFrame) + 1) ) / 4);    // 获取传输变量个数 m = (N - 12)/4
		   for(i = 0; i < iVarNum; i++){
			   for(j = 0; j < 4; j++){
				  pData.sChar[j] =  g_sGpuComRxBuf[FrameTypePos + 1 + i*4 + j];
			   }
			   g_fInGpuData[i] = pData.fVal;
		   }
		   g_bGpuComInputed = true;
	   }
	}
}

// 处理GpuCom的空闲中断DMA：接收GpuCom字符串
void ProcGpuComRcvIRQ(void)  
{  
	uint32_t isrflags   = READ_REG(GpuCom.Instance->ISR);
    uint32_t cr1its     = READ_REG(GpuCom.Instance->CR1);
    bool bClearOverFlag = true;
	
    if(((isrflags & USART_ISR_IDLE) != RESET) && ((cr1its & USART_CR1_IDLEIE) != RESET) && ((isrflags & USART_ISR_ORE) != USART_ISR_ORE)){
	    __HAL_UART_CLEAR_IDLEFLAG(&GpuCom);
	    uint32_t _len_dmarev = GpuCom_MAX_Rx_SIZE - __HAL_DMA_GET_COUNTER(GpuCom.hdmarx);
	    if(_len_dmarev){
          HAL_UART_DMAStopRx(&GpuCom);
			
			    vRcvGpuComInfTask(_len_dmarev);
				
	        HAL_UART_Receive_DMA(&GpuCom, g_sGpuComRxBuf, GpuCom_MAX_Rx_SIZE); 
			
	       __HAL_UART_DISABLE_IT(&GpuCom, UART_IT_ERR);
	       __HAL_UART_DISABLE_IT(&GpuCom, UART_IT_PE);
				
        // 不用清除溢出标志，其它情况都需要清除			
		     bClearOverFlag = false;
	    }
    }
	if(bClearOverFlag){
	   READ_REG(GpuCom.Instance->ISR);
	   READ_REG(GpuCom.Instance->RDR);
	   __HAL_UART_CLEAR_OREFLAG(&GpuCom);
	   __HAL_UART_CLEAR_IDLEFLAG(&GpuCom);
	}
}

// 清空测试端口接收缓冲数据
void vEmptyGpuComRcvBuffData(void)
{
	int i;
	for(i=0;i < GpuCom_MAX_Rx_SIZE; i++)	g_sGpuComRxBuf[i] = 0;
}

// 初始化DMA模式的GpuCom(Usart3)。最关键问题是打开空闲中断
void InitGpuComDMARcv(void)
{
	// ReadGpuCom模型串口无更新
    g_bGpuComInputed = false;

	// WriteMC模型串口监测字符串更新标志清零
    g_bGpuComDataReady = false;
	
    for(uint8_t i = 0; i < g_iGpuInSignalNum; i++) g_fInGpuData[i] = 0.f;
	
	// 清空测试串口缓冲数据
	vEmptyGpuComRcvBuffData();
	__HAL_UART_DISABLE_IT(&GpuCom, UART_IT_ERR);
	__HAL_UART_DISABLE_IT(&GpuCom, UART_IT_PE);
	// 打开串口接收DMA传输中断，等待将GpuCom字符串流首先导入到缓冲区1中
	__HAL_UART_ENABLE_IT(&GpuCom, UART_IT_IDLE);
	 HAL_UART_Receive_DMA(&GpuCom, g_sGpuComRxBuf, GpuCom_MAX_Rx_SIZE);  
}

// 初始化GPU串口
void initWriteGpuCom(uint8_t iSigNum,uint8_t iFrameType)
{
	  g_iGpuOutSignalNum = iSigNum;
	  g_iGPUOutFrameType = iFrameType;
	  g_bUsedOfGpuCom = true;
}

// 传递指定的测试参数给GPU串口
void writeGpuCom(uint8_t iChannel,double fVal)
{
	 static int iSigNum = 0;
	 if(iChannel < MAX_SIGNAL_NUM_OF_GpuCom){
		 g_fOutGpuData[iChannel] = fVal;             // 该变量为从模型串口回送的信号
 		iSigNum++;
		if(iSigNum >= g_iGpuOutSignalNum){
			DMAWriteData2GpuCom();
			iSigNum = 0;
		}
	 }
}

// 以下两个函数initReadModelCom、getModelComVal是和readModelComAPI.c及.tlc程序配合使用的函数
void initReadGpuCom(uint8_t iSigNum,uint8_t iFrameType)
{
	  g_iGpuInSignalNum = iSigNum;
	  g_iGPUInFrameType = iFrameType;
	  g_bUsedOfGpuCom = true;
}

// 从GPU串口读取变量到GPU端口(ReadGPU模块)，供用户模型Simulink程序使用。
double readGpuCom(uint8_t iChannel)
{
	  double fVal = 0x00;
	  if(iChannel < g_iGpuInSignalNum){          // 没有数据更新(g_bGpuComInputed为假)时，读取以前数据
		  fVal = g_fInGpuData[iChannel];
	  }
  	  else{
		  fVal = (double) g_bGpuComInputed;  // 模型串口监测数据更新标志
		  g_bGpuComInputed = false;         // 模型串口监测数据更新标志清0
	  }

	  return fVal;
}
/************************ (C) COPYRIGHT ACG co. *****END OF FILE****/
