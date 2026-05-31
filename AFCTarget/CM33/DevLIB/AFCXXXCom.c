/******************** (C) COPYRIGHT 2021 AFC Tech Co.*************************
 * 作    者  ： 曾庆华
 * 文 件 名  ：AFCXXXCom.c
 * 版    本  ：
 *             
 * 描    述  ：通用串口接口函数
 * 
 * 官    网  ：www.acecreator.com
 * 淘    宝  ：acecreator.taobao.com
 * 公 众 号  ：飞行控制与仿真
 *
*****************************************************************************/
#include "AFCXXXCom.h"
#include "ACGCommonAPI.h"
#include "AFCGlobalVar.h"
#include "usart.h"

#define NumOfExtUart   5    // 外扩Uart总数量为5  

// 外部可使用全局变量
uint8_t  g_sXComTxBuf[NumOfExtUart][XCom_MAX_Tx_SIZE]; 		// 单帧DMA发送缓冲,  最大XCom_MAX_Tx_SIZE字节
uint8_t  g_sXComRxBuf[NumOfExtUart][XCom_MAX_Rx_SIZE]; 		// 单帧串口中断接收缓冲,  最大XCom_MAX_Rx_SIZE字节
uint8_t  g_dataComRxBuf[NumOfExtUart][XCom_MAX_Rx_SIZE]; 		// g_sXComRxBuf缓冲数据,  最大XCom_MAX_Rx_SIZE字节
uint8_t  g_iXComRcvLen[NumOfExtUart];           // huart5、7、8三个串口接收缓冲长度
// 新增：每个串口的DMA接收完成标志（true=DMA已写完，false=DMA还在写）
bool g_xComDmaDone[NumOfExtUart] = {false};

// 全局变量
volatile uint32_t g_lastUartActivity[NumOfExtUart] = {0}; // 每个串口的最后活动时间戳
volatile bool g_xComOverTimeFlg[NumOfExtUart] = {false};       // 超时标志
volatile uint8_t g_xComFrameLen[NumOfExtUart] = {0};
volatile uint8_t g_xComRcvLenByOverTime[NumOfExtUart] = {0};    // 当前通过空闲中断实际接收到的字节数

volatile bool g_bXComUsedFlg = false;

struct __UART_HandleTypeDef *extComPort[NumOfExtUart];

void initXComPara(void)
{
	 extComPort[0] = &huart4;
	 extComPort[1] = &huart5;
	 extComPort[2] = &huart6;
	 extComPort[3] = &huart7;
	 extComPort[4] = &huart8;
}

#define UART_ONE_BIT_SAMPLING_ENABLE  0x00000800 // 对应USART_CR3_ONEBIT位

// 初始化通用输出串口
void initXComOut(uint8_t idxComPort,uint32_t comBaudRate)
{
		UART_HandleTypeDef *XCom = extComPort[idxComPort];
		
		XCom->Init.BaudRate = comBaudRate;
//		XCom->Init.OverSampling = UART_OVERSAMPLING_8; // 从16倍改为8倍过采样
//		XCom->Init.OneBitSampling = UART_ONE_BIT_SAMPLING_ENABLE; // 启用1位采样，提升抗干扰
	
	  HAL_UART_Init(XCom);
}

// 通过模型端口发送信息，DMA方式
void DataDMA2XCom(uint8_t idxComPort, uint16_t iBytes)
{
		UART_HandleTypeDef *XCom = extComPort[idxComPort];

		__HAL_UART_DISABLE_IT(XCom, UART_IT_IDLE);
	
	 //等待上一次的数据发送完毕
		uint16_t i = 0;
		while(HAL_DMA_GetState(XCom->hdmatx) == HAL_DMA_STATE_BUSY){
			 if(i++ > 1000) break;   // 超时处理
		};  // 这种处理方式容易导致控制周期超时，甚至死机情况
		
		if(i<1000){
		/* 2020.10.24 防止出现因状态未被复位，导致无法发送的情况(过去由HAL_UART_DMAStop()关闭，现在。。。) */
			if(XCom->gState != HAL_UART_STATE_READY)	HAL_UART_DMAStop(XCom);
		
			//开始发送数据
			HAL_UART_Transmit_DMA(XCom,g_sXComTxBuf[idxComPort],iBytes);
		}
		
		__HAL_UART_ENABLE_IT(XCom, UART_IT_IDLE);
}

void writeXCom(uint8_t idxComPort, uint16_t iBytes,uint8_t iVal)
{
	 static int idxByte = 0;
	 if(idxByte < iBytes){
		 g_sXComTxBuf[idxComPort][idxByte++] = iVal;
		 if(idxByte == iBytes){
		// 最后一个字节，则将当前帧数据打包输出，且将帧序号清0
				DataDMA2XCom(idxComPort,iBytes);
				idxByte = 0;
			}
		}
}

// 初始化XComIn
void initXComIn(uint8_t idxComPort,uint32_t comBaudRate,uint8_t iFrame)
{
		UART_HandleTypeDef *pXCom = extComPort[idxComPort];
		
		pXCom->Init.BaudRate = comBaudRate;
	  HAL_UART_Init(pXCom);
	
    g_xComOverTimeFlg[idxComPort] = false;
	  g_xComFrameLen[idxComPort] = iFrame;
	  g_xComRcvLenByOverTime[idxComPort] = 0;
		
		// 初始化DMA模式的XCom。最关键问题是打开空闲中断
 		InitComDMA(pXCom,g_sXComRxBuf[idxComPort],XCom_MAX_Rx_SIZE);
	
	  // 只要使用了一个接收串口，则置位该标志，表示是否启动串口接收超时操作。
	  g_bXComUsedFlg = true;
}

//// 处理XCom的空闲中断DMA：接收XCom字符串。
////   下面程序已于20251117利用TLostFrame251115.slx和TLostFrame251115slrt.slx(DeskTop realtime)通过了测试。
//void ProXComRcvIRQ(uint8_t idxComPort)
//{
//		UART_HandleTypeDef *pXCom = extComPort[idxComPort];
//      
//	  uint32_t isrflags   = READ_REG(pXCom->Instance->ISR);
//    uint32_t cr1its     = READ_REG(pXCom->Instance->CR1);
//    bool bClearOverFlag = true;
//	
//    if(((isrflags & USART_ISR_IDLE) != RESET) && ((cr1its & USART_CR1_IDLEIE) != RESET) && ((isrflags & USART_ISR_ORE) != USART_ISR_ORE)){
//	      __HAL_UART_CLEAR_IDLEFLAG(pXCom);
//	      uint32_t _len_dmarev = XCom_MAX_Rx_SIZE - __HAL_DMA_GET_COUNTER(pXCom->hdmarx);
//				if(_len_dmarev){
//					HAL_UART_AbortReceive(XCom);
//							
//					g_iXComRcvLen[idxComPort] = _len_dmarev;
//					memcpy(g_dataComRxBuf[idxComPort],g_sXComRxBuf[idxComPort],_len_dmarev);    // 拷贝到数据缓冲区
//          g_xComDmaDone[idxComPort] = true; 
//					memset(g_sXComRxBuf[idxComPort], 0, _len_dmarev);               // 清空接收缓冲区
//					
//					HAL_UART_Receive_DMA(XCom, g_sXComRxBuf[idxComPort], XCom_MAX_Rx_SIZE); 
//				}
//				__HAL_UART_DISABLE_IT(XCom, UART_IT_ERR);
//				__HAL_UART_DISABLE_IT(XCom, UART_IT_PE);
//			
//        // 不用清除溢出标志，其它情况都需要清除			
//		    bClearOverFlag = false;
//    }
//		if(bClearOverFlag){
//			 READ_REG(XCom->Instance->ISR);
//			 READ_REG(XCom->Instance->RDR);
//			 __HAL_UART_CLEAR_OREFLAG(XCom);
//		}
//}

void xComOverTimeHandle(uint8_t idxComPort)
{
	UART_HandleTypeDef *pXCom = extComPort[idxComPort];

	if(idxComPort == 0){
		//关闭接收后重启DMA
		HAL_UART_AbortReceive(pXCom);

// 关键新增：彻底清除UART接收寄存器和错误标志（解决虚假中断）
        // 1. 读取UART状态寄存器，判断是否有未处理的接收/错误标志
		uint32_t uart_isr = READ_REG(pXCom->Instance->ISR);
		// 2. 只要存在RXNE（接收非空）或ORE（溢出），就读取RDR寄存器清零（硬性要求）
		if((uart_isr & (USART_ISR_RXNE_RXFNE | USART_ISR_ORE)) != 0){
				(void)READ_REG(pXCom->Instance->RDR); // 必须读RDR，标志才会清零
		}
		// 3. 额外清除ORE标志（双重保险，避免个别芯片残留）
		__HAL_UART_CLEAR_OREFLAG(pXCom);

		// 原有代码保留：读取长度、重启DMA
		uint32_t _len_dmarev = XCom_MAX_Rx_SIZE - __HAL_DMA_GET_COUNTER(pXCom->hdmarx);
		g_iXComRcvLen[idxComPort] = _len_dmarev;
		
		// 把已读到数据拷贝到数据缓冲区。Modified by zqhkd 20251201
		memcpy(g_dataComRxBuf[idxComPort],g_sXComRxBuf[idxComPort],_len_dmarev); 
		g_xComDmaDone[idxComPort] = true; 
		
		HAL_UART_Receive_DMA(pXCom, g_sXComRxBuf[idxComPort], XCom_MAX_Rx_SIZE);

		// 更新最后活动时间(us)
		g_xComRcvLenByOverTime[idxComPort] = 0;
		g_lastUartActivity[idxComPort] = HAL_GetTick_us();
	}
	g_xComOverTimeFlg[idxComPort] =false;
}

uint32_t curTime;
int32_t tmp;
// 	进行XCom串口帧接收超时处理
void vXComFrameOverTimePro(uint16_t overTime)
{
	  if(g_bXComUsedFlg){  // 使用了XCom的串口输入功能，才启动串口接收超时处理
			uint8_t i;
		
			for(i = 0; i < NumOfExtUart; i++){
				 if((g_xComFrameLen[i] > 0) && (g_xComRcvLenByOverTime[i] > 0)){
					 curTime = HAL_GetTick_us();
					 tmp = curTime - g_lastUartActivity[i];
					 if((tmp > 0 ) && (tmp > overTime)) g_xComOverTimeFlg[i] = true;
					 if(g_xComOverTimeFlg[i]) xComOverTimeHandle(i);
				 }
			}
		}
}

// 处理XCom的空闲中断DMA：接收XCom字符串
void ProXComRcvIRQ(uint8_t idxComPort)
{
		UART_HandleTypeDef *pXCom = extComPort[idxComPort];

	  uint32_t isrflags   = READ_REG(pXCom->Instance->ISR);
    uint32_t cr1its     = READ_REG(pXCom->Instance->CR1);
    bool bClearOverFlag = true;
	
    if(((isrflags & USART_ISR_IDLE) != RESET) && ((cr1its & USART_CR1_IDLEIE) != RESET)){
	    __HAL_UART_CLEAR_IDLEFLAG(pXCom);
			  uint32_t _len_dmarev = XCom_MAX_Rx_SIZE -  __HAL_DMA_GET_COUNTER(pXCom->hdmarx);
				
				if(_len_dmarev){
					// 更新最后活动时间(us)
					g_lastUartActivity[idxComPort] = HAL_GetTick_us();
					g_xComRcvLenByOverTime[idxComPort]  = _len_dmarev;
					if(_len_dmarev >= g_xComFrameLen[idxComPort] || g_xComOverTimeFlg[idxComPort]){    // 接收到一个完整帧，或者超时处理g_xComOverTimeFlg[idxComPort]被置位
							HAL_UART_AbortReceive(pXCom);
							g_iXComRcvLen[idxComPort] = _len_dmarev;
							memcpy(g_dataComRxBuf[idxComPort],g_sXComRxBuf[idxComPort],_len_dmarev);    // 拷贝到数据缓冲区
							g_xComDmaDone[idxComPort] = true; 
							memset(g_sXComRxBuf[idxComPort], 0, _len_dmarev);               // 清空接收缓冲区

							HAL_UART_Receive_DMA(pXCom, g_sXComRxBuf[idxComPort], XCom_MAX_Rx_SIZE);
							g_xComRcvLenByOverTime[idxComPort] = 0;   // 用于超时监测的当前串口接收帧长度
							g_xComOverTimeFlg[idxComPort] = false; // 重置超时标志
					}
				}
				__HAL_UART_DISABLE_IT(pXCom, UART_IT_ERR);
				__HAL_UART_DISABLE_IT(pXCom, UART_IT_PE);
				
				// ORE标志单独处理，不影响数据接收
				if((isrflags & USART_ISR_ORE) != RESET){
						__HAL_UART_CLEAR_OREFLAG(pXCom);
				}			
        // 不用清除溢出标志，其它情况都需要清除			
		    bClearOverFlag = false;
    }
		if(bClearOverFlag){
			 READ_REG(pXCom->Instance->ISR);
			 READ_REG(pXCom->Instance->RDR);
			 __HAL_UART_CLEAR_OREFLAG(pXCom);
		}
}

// 从模型串口XCom读取变量到模型端口(ReadMC模块)，供用户模型Simulink程序使用。
uint8_t readXCom(uint8_t idxComPort,uint8_t idxByte)
{
		uint8_t iVal = 0x00;
		if(idxByte < g_iXComRcvLen[idxComPort] ){    
			 iVal = g_dataComRxBuf[idxComPort][idxByte];
		}
		
	  return iVal;
}
/************************ (C) COPYRIGHT ACG co. *****END OF FILE****/
