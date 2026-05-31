/******************** (C) COPYRIGHT 2017 ACE Tech Co.*************************
 * 作    者 ： 曾庆华
 * 文 件 名 ： ACGCommonAPI.c
 * 描    述 ： 工程中公用函数
 * 版    本 ：
 *     V5.01.230824  -- 添加ProcComRcvIRQ中断处理函数，各个串口模块如AFCSimuCom、AFCDtCom、AFCGPUCom函数均可调用此函数，或仿照该函数写中断处理函数
 *            
 * 官    网 ：www.acecreator.com
 * 淘    宝 ：acecreator.taobao.com
 * 公 众 号 ：无人飞行控制
*****************************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "ACGCommonAPI.h"
#include "AFCGlobalVar.h"

uint16_t getCurUAVId(void)
{
	  uint16_t iUavIdOfSwarms = g_UavFcsParam.UavPara.UavId & 0xFFFF;
	  return iUavIdOfSwarms;
}

// 根据变量符号返回1或-1的函数
int8_t sign(float fVal)
{
	 return (fVal > 0) - (fVal < 0);  
}

// 安装方向系数：bDir表正装，返回1；否则反装，返回-1
int8_t iXYZDir(bool bDir)
{
	  int8_t iDir = 1;
	  if(bDir) iDir = -1;
	  return iDir;
}

// 根据当前缓冲区字节数，计算包含多少个float（4字节）
uint8_t iCalFloatNums(uint8_t iBytes)
{
	  uint8_t iNumFloats;
	  iNumFloats = iBytes/4;
	  if(iBytes % 4 != 0) iNumFloats++;
    return iNumFloats;
}

// V5.04.240717: 添加函数用于将结构体转换为浮点数组
uint8_t structToFloatArray(float *g_fArray, const void* structPtr, size_t structSize) {
    // 计算需要多少个float来存储结构体数据（每个float 4字节）
    uint8_t numFloats = iCalFloatNums(structSize); // 向上取整
    // 分配内存
    float* floatArray = (float*)malloc(numFloats * sizeof(float));

		if (!floatArray) {
      perror("Memory allocation failed");
//        exit(EXIT_FAILURE);
			return 0;
    }
    // 使用memcpy将结构体内容复制到float数组
    memcpy(floatArray, structPtr, structSize);
    // 如果结构体大小不是4的整数倍，剩余的字节需要补0
    if (structSize % 4 != 0) {
        uint8_t paddingBytes = 4 - (structSize % 4);
        memset((char*)floatArray + structSize, 0, paddingBytes);
    }
		
		// 将转换结果传递给上一级浮点数组变量
		memcpy(g_fArray, floatArray, numFloats * sizeof(float));
		
		// 释放掉动态分配的内存
		free(floatArray);
		
    return numFloats;
}

// 函数用于将浮点数组转换回结构体
uint8_t floatArrayToStruct(void* structPtr, const float* floatArray, size_t structSize) {
    // 将浮点数组的内容复制回结构体
    memcpy(structPtr, floatArray, structSize);
	  
	  uint8_t numFloats = (structSize + 3) / 4; // 向上取整
	  return numFloats;
}

// DWT硬件精确延时Nus方案
// DWT (Data Watchpoint and Trace) 是Cortex-M内核的调试组件，其时钟周期计数器（CYCCNT）可用来实现高精度延时
void InitDWT(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

void DWTDelayNus(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t cycles = us * (SystemCoreClock / 1000000);
    while((DWT->CYCCNT - start) < cycles);
}

//延时us函数
void delay_us(uint32_t us) //利用CPU循环实现的非精准应用的微秒延时函数
{
    uint32_t delay = (HAL_RCC_GetHCLKFreq() / 8000000 * us); //使用HAL_RCC_GetHCLKFreq()函数获取主频值，经算法得到1微秒的循环次数
    while (delay--); //循环delay次，达到1微秒延时
}

uint32_t HAL_GetTick_us(void) 
{
  uint32_t ms, ms2;
  uint32_t load, val;
  uint32_t us_per_ms = 1000;
  
  // 解决SysTick溢出问题：读取毫秒值，然后再次读取确认
  do {
    ms = HAL_GetTick();
    load = SysTick->LOAD;
    val = SysTick->VAL;
    ms2 = HAL_GetTick();
  } while (ms != ms2);  // 如果ms发生变化，说明期间SysTick溢出了，重新读取
  
  // 计算当前毫秒内已经流逝的微秒数
  // 加1是为了避免除以0，同时补偿LOAD寄存器的值是包含0的
  uint32_t us = ((load - val) * us_per_ms) / (load + 1);
  
  // 将毫秒和微秒部分组合，得到总微秒数
  return (ms * 1000) + us;
}

//uint32_t HAL_GetTick_us(void) 
//{
//  // 获取毫秒部分
//  uint32_t ms = HAL_GetTick();
//  
//  // 读取SysTick的重装载值和当前值
//  uint32_t load = SysTick->LOAD;
//  uint32_t val = SysTick->VAL;
//  
//  // 计算当前毫秒内已经流逝的微秒数
//  uint32_t us_per_ms = 1000;
//  uint32_t us = ( (load - val) * us_per_ms ) / (load + 1);
//  
//  // 将毫秒和微秒部分组合，得到总微秒数
//  return (ms * 1000) + us;
//}

// 软件延时程序。 iTime单位为us，但延时不准，仅仅测量了延时20us的值
void SoftDelayNus(uint32_t iTime)
{
	 iTime = iTime*5;
	 for(int i=0;i<iTime;i++);
}

uint32_t SoftDelayNms(uint16_t iTime)
{
	 uint32_t iStartTime = HAL_GetTick();
	 uint16_t i,j;
	 for(i = 0; i < iTime; i++){
		 for(j = 0; j < 350; j++){
          sqrt(i*j);
     }
	 }
	 return (HAL_GetTick() - iStartTime);
}

uint8_t CalChkSum(uint8_t *str,uint8_t len)
{
	  uint8_t i,iChkSum=0;
	  for(i=0;i<len;i++) iChkSum += str[i];
	  iChkSum = ~iChkSum + 1;
	  return iChkSum;
}

bool bCalChkSum(uint8_t *str,uint8_t len)
{
    return (CalChkSum(str,len)==0);          // 校验和为0，则返回true
}

// 采用项目办CRC16大端模式
//#define __CRC16_DataInterface191022__  

uint16_t CalCRC16(uint8_t * data, int length)
{
	uint16_t crc_table [256] = {
		0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
		0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
		0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
		0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
		0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
		0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
		0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
		0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
		0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
		0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
		0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
		0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
		0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
		0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
		0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
		0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
		0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
		0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
		0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
		0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
		0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
		0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
		0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
		0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
		0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
		0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
		0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
		0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
		0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
		0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
		0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
		0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
	};

  uint16_t crc16 = 0x0000;
  unsigned short temp;
  int count;

  for (count = 0; count < length; ++count)
  {
    temp = (*data++ ^ (crc16 >> 8)) & 0xff;
    crc16 = crc_table[temp] ^ (crc16 << 8);
  }
#ifndef __CRC16_DataInterface191022__
// 以下程序为正常的CRC16校验。注意：项目办提供的CalCRC16Ref返回值采用的是大端模式存取，为确保正常CRC校验，需要CRC16计算值高低字节交换。Modified by zqh on 20191228
		crc16 = ((crc16 & 0xff)<<8) + ((crc16 & 0xff00)>>8);   // 高低字节交换
#endif
  return crc16;
}

// 采用项目办下发数据接口控制文件中的CRC16格式，CRC16计算结果值采用大端模式存取与传输，和系统采用的小端模式不一致，
//      直接使用的话整帧数据（含CRC16字）的CRC计算结果不为零，只能采用比较方式
bool bChkCRC16(uint8_t * data, int length)
{
	bool bFlg;
#ifdef __CRC16_DataInterface191022__
	uint16_t calCrc16 = CalCRC16(data,length-2);
	uint16_t rcvCrc16 = data[length-2] + (data[length-1] << 8);
	bFlg = (calCrc16==rcvCrc16);
#else
  bFlg = (CalCRC16(data,length)==0x0);
#endif
  return bFlg;
}

// 一个存储了ID标识码信息的缓冲idBuff, 本函数可在idBuff缓冲中找到标识码为idVal的数组索引值
int iFindIdBuffIdx(uint8_t idVal,uint8_t iMax,uint8_t *idBuff)
{
	  int idx = -1;
	  uint8_t i;
	  if(idVal > 0){    // 其帧类型码必须为大于0的数值
			for(i = 0; i < iMax; i++){
				if(idBuff[i] == idVal){
					idx = i;
					break;
				}
			}
		}
		return idx;   // 当返回-1时，表示未找到, 否则为其索引值
}

// 检查输出帧队列中是否包含iFrameTypeCode, 如果没有找到会返回最大值iMaxFrameNum。
uint8_t iChkFrameTypeIdx(uint8_t iFrameTypeCode,uint8_t iMaxFrameNum,uint8_t *iFrameType)
{
	  uint8_t i;
	  for(i = 0; i < iMaxFrameNum; i++){
			 if(iFrameTypeCode == iFrameType[i]){
				 break;
			 }
		}
		return i;
}

// 该函数可以取代HAL_UART_DMAStop，修正了STM32H7XX芯片的DMA操作时的某些不足
/**
 1. @brief Stop the DMA Receive.
 2. @param huart: UART handle.
 3. @retval HAL status
  */
HAL_StatusTypeDef HAL_UART_DMAStopRx(UART_HandleTypeDef *huart)
{
  /* Stop UART DMA Rx request if ongoing */
  if ((huart->RxState == HAL_UART_STATE_BUSY_RX) &&
      (HAL_IS_BIT_SET(huart->Instance->CR3, USART_CR3_DMAR)))
  {
    CLEAR_BIT(huart->Instance->CR3, USART_CR3_DMAR);

    /* Abort the UART DMA Rx channel */
    if(huart->hdmarx != NULL)
    {
      HAL_DMA_Abort(huart->hdmarx);
    }

    //UART_EndRxTransfer(huart);
    /* Disable RXNE, PE and ERR (Frame error, noise error, overrun error) interrupts */
	CLEAR_BIT(huart->Instance->CR1, (USART_CR1_RXNEIE | USART_CR1_PEIE));
	CLEAR_BIT(huart->Instance->CR3, USART_CR3_EIE);

	/* At end of Rx process, restore huart->RxState to Ready */
	huart->RxState = HAL_UART_STATE_READY;
  }

  return HAL_OK;
}

// 由于同时处理串口发送和接收时，会导致丢帧，重写串口发送函数HAL_UART_Transmit_DMA()
void dma_send(UART_HandleTypeDef *huart, unsigned char *buffer,unsigned int length)
{
 //等待上一次的数据发送完毕
	uint16_t i = 0;
  while(HAL_DMA_GetState(huart->hdmatx) == HAL_DMA_STATE_BUSY){
	   if(i++ > 2000) break;   // 超时处理
	};  // 这种处理方式容易导致控制周期超时，甚至死机情况
	
	if(i<2000){
	/* 2020.10.24 防止出现因状态未被复位，导致无法发送的情况(过去由HAL_UART_DMAStop()关闭，现在。。。) */
	  if(huart->gState != HAL_UART_STATE_READY)	HAL_UART_AbortTransmit(huart);
	
    /* 关闭DMA */
    __HAL_DMA_DISABLE(huart->hdmatx);

    //开始发送数据
    HAL_UART_Transmit_DMA(huart,buffer,length);
	}
}

bool comCalChkSum(uint8_t *data, uint8_t iChkMode, uint8_t length)
{
	bool bFlg;
	if(iChkMode == CRC_CHECK)
		 bFlg = bChkCRC16(data, length);
	else
		 bFlg = bCalChkSum(data, length);
		
	return bFlg;
}

// 检测帧数据的有效性,如果是一帧有效帧则返回true，否则返回false
bool bChkFrameValid(uint8_t *pStr,uint8_t iChkMode, uint16_t len)
{
	 bool bFlg = false;
	 if(len > FrameParaNumPos){    // 帧数据长度必定大于帧参数区域所在位置的长度
		 uint16_t tmpL = pStr[FrameLengthPos] + (pStr[FrameLengthPos + 1]<<8);
		 if(pStr[0]==(FRAME_HEADER_ID&0xff) && 
			 (pStr[1]==((FRAME_HEADER_ID&0xff00)>>8)) && 
		     (tmpL==len) &&
			 (comCalChkSum(pStr,iChkMode,len)))  bFlg = true;
	 }
   return bFlg;	
}

// V1.01.210829： 将帧头控制信息及指定数量的浮点数、以及CRC校验码等一起打包到pSendBuff缓冲区中，通过DMA中断方式发送出去。
//     帧头控制信息 pHeaderFram: 包括iFrameID、iFrameLen、iFrameTick、idSender、idReceiver、iFrameType。调用前主要是准备iFrameTick、idSender、idReceiver、iFrameType。
//     传输参数数量 iSignalNum:
//     传输参数值  pData:        为一浮点数组，每一个数组元素是需要传输的参数
//     发送缓冲区  pSendBuff：  本函数执行完毕，将把相关参数打包至该缓冲区中，待串口发送程序进行传输处理
//   校验和方式打包
uint16_t iPackData2Hex(THeaderFrame pHeaderFrame, uint8_t iSignalNum, float *pData, uint8_t *pSendBuff, uint8_t iChkMode)
{
	// 计算HEX传输时所需字符串长度，该字符串为：$TC,f1,f2,...,fn,*v  
	uint16_t len = sizeof(THeaderFrame) + iSignalNum * sizeof(float) + iChkMode + 1 ;    // 累加和校验和占1个字节，累加和1个字节(iChkMode为0），CRC占2字节(iChkMode为1）

	pHeaderFrame.iFrameID = FRAME_HEADER_ID;
	pHeaderFrame.iFrameLen = len;
	
	uint16_t i,iHeader;
	for(i = 0; i < sizeof(THeaderFrame); i++)
	{
		 pSendBuff[i] = ((uint8_t *)(&pHeaderFrame))[i];
	}
	iHeader = i;

	for(i = 0; i < iSignalNum;i++){
		pSendBuff[iHeader++] = ((uint8_t *)(&pData[i]))[0];   // 将测试数据转成在输出缓冲区中
		pSendBuff[iHeader++] = ((uint8_t *)(&pData[i]))[1];
		pSendBuff[iHeader++] = ((uint8_t *)(&pData[i]))[2];
		pSendBuff[iHeader++] = ((uint8_t *)(&pData[i]))[3];
	}
	if(iChkMode == SUM_CHECK){
		uint8_t iChkval;
		iChkval = CalChkSum(pSendBuff,len-1);
		pSendBuff[iHeader++] = 	iChkval;		
	}
	if(iChkMode == CRC_CHECK){
		uint16_t iChkval;
		iChkval = CalCRC16(pSendBuff,len-2);
		pSendBuff[iHeader++] = 	(uint8_t) (iChkval & 0x00ff);		
	    pSendBuff[iHeader++] = 	(uint8_t) ((iChkval & 0xff00) >> 8);		
	}

	return len;
}

uint16_t iPackData2Hex2(THeaderFrame pHeaderFrame, uint8_t iSignalNum, uint8_t *pData, uint8_t *pSendBuff, uint8_t iChkMode)
{
	// 计算HEX传输时所需字符串长度，该字符串为：$TC,f1,f2,...,fn,*v  
	uint16_t len = sizeof(THeaderFrame) + iSignalNum + iChkMode + 1 ;    // 累加和校验和占1个字节，累加和1个字节(iChkMode为0），CRC占2字节(iChkMode为1）

	pHeaderFrame.iFrameID = FRAME_HEADER_ID;
	pHeaderFrame.iFrameLen = len;
	
	uint16_t i,iHeader;
	for(i = 0; i < sizeof(THeaderFrame); i++)
	{
		 pSendBuff[i] = ((uint8_t *)(&pHeaderFrame))[i];
	}
	iHeader = i;

	for(i = 0; i < iSignalNum;i++){
		pSendBuff[iHeader++] = *pData;//((uint8_t *)(&pData))[i];   // 将测试数据转成在输出缓冲区中
		pData++;
	}
	if(iChkMode == SUM_CHECK){
		uint8_t iChkval;
		iChkval = CalChkSum(pSendBuff,len-1);
		pSendBuff[iHeader++] = 	iChkval;		
	}
	if(iChkMode == CRC_CHECK){
		uint16_t iChkval;
		iChkval = CalCRC16(pSendBuff,len-2);
		pSendBuff[iHeader++] = 	(uint8_t) (iChkval & 0x00ff);		
	    pSendBuff[iHeader++] = 	(uint8_t) ((iChkval & 0xff00) >> 8);		
	}

	return len;
}

// V1.01.210829： 从串口接收缓冲区中检验帧数据的有效性，并解包帧协议中的参数值。
//      返回值为当前处理帧的帧类型码
uint8_t iUnPackHex2Data(uint8_t *comRxBuf,uint8_t iRcvLen,float *pData,uint8_t iChkMode)
{
	uint8_t i,j,iVarNum;
	uint16_t iFrameLen;
    TChar2FloatStruct pChar2Data;
	
		iFrameLen = comRxBuf[FrameLengthPos] + (comRxBuf[FrameLengthPos + 1] << 8);
//		iFrameType = comRxBuf[FrameTypePos];
	 // 模型串口监测数据，最大g_iModelInSignalNum字节
	 // 用户实验模型的串口监测。通过Simulink工具箱中的ReadMC读取Com数据
		iVarNum = iCalFloatNums(iFrameLen - (sizeof(THeaderFrame) + iChkMode+1 ));    // 获取传输变量个数 m = (N - 12)/4, 取上边界

		// V1.03.211201: 将第一个变量设置为帧时标, 帧时标为32位整型数
//	    pData[0] = (float)(comRxBuf[FrameTickPos] + (comRxBuf[FrameTickPos + 1]<<8) + (comRxBuf[FrameTickPos + 2]<<16) + (comRxBuf[FrameTickPos + 3]<<24)) ;
	    pData[0] = g_sRealTimeCount.fcsTime/1000.f;
		
		for(i = 0; i < iVarNum; i++){
		   for(j = 0; j < 4; j++){
			  pChar2Data.sChar[j] =  comRxBuf[FrameParaPos + i*4 + j];
		   }
		   pData[i+1] = pChar2Data.fVal;  // pData[0]为帧时标
		}
	return iVarNum;     // 如果没有有效数据，则iVarNum为0
}

// V1.01.210829： 从串口接收缓冲区中检验帧数据的有效性，并解包帧协议中的参数值。
//      返回值为当前处理帧的帧类型码
uint8_t iUnPackHex2Data1(uint8_t *comRxBuf,uint8_t iRcvLen,float *pData,uint8_t iChkMode)
{
	uint8_t i,j,iVarNum;
	uint16_t iFrameLen;
  TChar2FloatStruct pChar2Data;
	
	iFrameLen = comRxBuf[FrameLengthPos] + (comRxBuf[FrameLengthPos + 1] << 8);
//	iFrameType = comRxBuf[FrameTypePos];
 // 模型串口监测数据，最大g_iModelInSignalNum字节
 // 用户实验模型的串口监测。通过Simulink工具箱中的ReadMC读取Com数据
	iVarNum = (uint8_t)((iFrameLen - (sizeof(THeaderFrame) + iChkMode+1 ) ) / 4);    // 获取传输变量个数 m = (N - 12)/4

	// V1.03.211201: 将第一个变量设置为帧时标, 帧时标为32位整型数
	 pData[0] = (float)(comRxBuf[FrameTickPos] + (comRxBuf[FrameTickPos + 1]<<8) + (comRxBuf[FrameTickPos + 2]<<16) + (comRxBuf[FrameTickPos + 3]<<24)) ;
	
	for(i = 0; i < iVarNum; i++){
		 for(j = 0; j < 4; j++){
			pChar2Data.sChar[j] =  comRxBuf[FrameParaPos + i*4 + j];
		 }
		 pData[i+1] = pChar2Data.fVal;  // pData[0]为帧时标
	}
	return true;     // 如果没有有效数据，则iFrameType为0
}

void vEmptyComBuffData(uint8_t *pBuffData, uint16_t iMaxBuffSize)
{
	for(uint16_t i=0;i < iMaxBuffSize;i++) pBuffData[i] = 0;
}

void vGetMaxMin(double fMoveArr[],uint8_t iNum, double *fMax, double *fMin)
{
	*fMax = *fMin = fMoveArr[0];
	for(uint8_t i = 1; i < iNum; i++){
		if(fMoveArr[i] > *fMax) *fMax = fMoveArr[i];
		if(fMoveArr[i] < *fMin) *fMin = fMoveArr[i];
	}
}

// 滑动滤波器，参数定义：
//   fMoveArr[] --- 当前滑动缓冲区的原始数据；
//   iSlideWinLength ---- 滑动缓冲区长度
//   iPos  -- 当前指针位置
//   fIn    -- 最新数据
void fMovingAverage(double fMoveArr[],uint8_t iSlideWinLength, uint8_t *iPos, double fIn, double *fOut)
{
//	double last,fMax,fMin;
	double last;
	if(++*iPos >= iSlideWinLength) *iPos = 0;
	
	last = fMoveArr[*iPos];   // 把待替换的最早压入缓冲区的那个数暂存至last
	fMoveArr[*iPos] = fIn;   //  把最新数据压入缓冲区
	
	*fOut += (fIn - last)/ iSlideWinLength;
}

void vSlideWinAverage(double fMoveArr[],uint8_t iSlideWinLength, uint8_t *iPos, double fIn, double *fOut,bool *bSlideWinFull)
{
	if(!(*bSlideWinFull)){
		// 滑动窗口缓冲区未满时，仅仅把当前数据放入缓冲，指针不断调整，直至填满缓冲区
		fMoveArr[*iPos] = fIn;
		if(++*iPos >= iSlideWinLength){
			*iPos = 0;
			*bSlideWinFull = true;
			*fOut = 0;
			for(uint8_t i = 0; i < iSlideWinLength; i++) *fOut += fMoveArr[i];
			*fOut = *fOut / iSlideWinLength;
		}
		else *fOut = fIn;      // 滑动窗口未满时，仅输出最新数据，不做滑动平均
	}
	else fMovingAverage(fMoveArr,iSlideWinLength,iPos,fIn,fOut);
}

// V4.02.220504: 禁止全部中断, 在调试AFCDtCom程序中发现，如果仅仅只使用DtComOut, 发送数据后还是有可能产生空闲中断，
//   而此时又未调用DtComIn的初始化函数InitDtComDMA，从而可能导致响应中断处理函数ProcDtComRcvIRQ时出现内存g_sDtComRxBuf泄漏。
void DiableAllIrq(void)
{
	// 依据InitSimuComDMA打开的中断，需要首先关闭
	  __HAL_UART_DISABLE_IT(&SimuCom, UART_IT_ERR);
	  __HAL_UART_DISABLE_IT(&SimuCom, UART_IT_PE);
	  __HAL_UART_DISABLE_IT(&SimuCom, UART_IT_IDLE);	
	
	// 依据InitDtComDMA打开的中断，需要首先关闭
	  __HAL_UART_DISABLE_IT(&DtCom, UART_IT_ERR);
	  __HAL_UART_DISABLE_IT(&DtCom, UART_IT_PE);
	  __HAL_UART_DISABLE_IT(&DtCom, UART_IT_IDLE);	
	
	// 依据InitADIS16507打开的中断，无中断
	
	// 依据InitGpsComDMA打开的中断
	  __HAL_UART_DISABLE_IT(&GpsCom, UART_IT_ERR);
	  __HAL_UART_DISABLE_IT(&GpsCom, UART_IT_PE);
	  __HAL_UART_DISABLE_IT(&GpsCom, UART_IT_IDLE);	
	
	// 依据InitGpuComDMA打开的中断
	  __HAL_UART_DISABLE_IT(&GpuCom, UART_IT_ERR);
	  __HAL_UART_DISABLE_IT(&GpuCom, UART_IT_PE);
	  __HAL_UART_DISABLE_IT(&GpuCom, UART_IT_IDLE);	
	
	// 依据InitSBusComDMARcv
	  __HAL_UART_DISABLE_IT(&SBusCom, UART_IT_ERR);
	  __HAL_UART_DISABLE_IT(&SBusCom, UART_IT_PE);
	  __HAL_UART_DISABLE_IT(&SBusCom, UART_IT_IDLE);
	
	// AFCServo需要主动调用相关参数才能开启中断，无需处理
	
	// 依据initTof(InitHart4DMARcv)打开的中断
	  __HAL_UART_DISABLE_IT(&huart4, UART_IT_ERR);
	  __HAL_UART_DISABLE_IT(&huart4, UART_IT_PE);
	  __HAL_UART_DISABLE_IT(&huart4, UART_IT_IDLE);	
	
	// initICM20602未开启中断，无需处理
	// initICM42688未开启中断，无需处理
	// BMM150未开启中断，无需处理
	// BMP388未开启中断，无需处理
	// ADIS16488未开启中断，无需处理
}

// 初始化DMA模式串口huart
void InitComDMA(UART_HandleTypeDef *huart, uint8_t *pDmaRcvBuff,uint16_t iMaxBuffSize)
{
	  vEmptyComBuffData(pDmaRcvBuff, iMaxBuffSize);
	
	  __HAL_UART_DISABLE_IT(huart, UART_IT_ERR);
	  __HAL_UART_DISABLE_IT(huart, UART_IT_PE);

	// 打开串口接收DMA传输中断，等待将Com字符串流首先导入到缓冲区1中
	  __HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);    // V4.02.220505： 注意有些串口如DtCom第一次打开空闲中断时，会进入串口中断，此时需要强行清空闲中断，否则会导致死循环。
	
	// 打开串口接收DMA传输中断
	  HAL_UART_Receive_DMA(huart, pDmaRcvBuff, iMaxBuffSize); 
}

// 处理当前Com的空闲中断DMA：接收当前串口字符串
uint8_t ProcComRcvIRQ(UART_HandleTypeDef *huart , uint8_t *pDmaRcvBuff,uint16_t iMaxBuffSize)  
{  
	  uint32_t isrflags   = READ_REG(huart->Instance->ISR);
    uint32_t cr1its     = READ_REG(huart->Instance->CR1);
    bool bClearOverFlag = true;
	  uint8_t _len_dmarev = 0;
	  
    if(((isrflags & USART_ISR_IDLE) != RESET) && ((cr1its & USART_CR1_IDLEIE) != RESET) && ((isrflags & USART_ISR_ORE) != USART_ISR_ORE)){
				__HAL_UART_CLEAR_IDLEFLAG(huart);
				_len_dmarev = (uint8_t)(iMaxBuffSize - __HAL_DMA_GET_COUNTER(huart->hdmarx));
        if(_len_dmarev){
				//            HAL_UART_DMAStopRx(huart);
				/* 这里以前使用HAL_UART_DMAStop(huart)，但这个函数会导致TX的DMA被关闭，小概率DMA发送会丢数据 */
				/* 2021.10.24 修改为HAL_UART_AbortReceive(huart)，对应DMA发送也需要做一些处理 */
		//	2023.08.23 更改HAL_UART_AbortReceive(huart)至if(_len_dmarev)语句以外
					HAL_UART_AbortReceive(huart);
					HAL_UART_Receive_DMA(huart, pDmaRcvBuff, iMaxBuffSize); 

					__HAL_UART_DISABLE_IT(huart, UART_IT_ERR);
					__HAL_UART_DISABLE_IT(huart, UART_IT_PE);

			// 不用清除溢出标志，其它情况都需要清除			
					bClearOverFlag = false;
				}
    }
	if(bClearOverFlag){
	   READ_REG(huart->Instance->ISR);
	   READ_REG(huart->Instance->RDR);
	   __HAL_UART_CLEAR_OREFLAG(huart);
//	   __HAL_UART_CLEAR_IDLEFLAG(huart);
	}
	return _len_dmarev;
}

#include <tim.h>
void EnTaskTimerIRQ(bool bEnable)
{
	 if(bEnable){
			HAL_TIM_Base_Start_IT(&htim6);
			HAL_TIM_Base_Start_IT(&htim14);
			HAL_TIM_Base_Start_IT(&htim13);
	 }
	 else{
			HAL_TIM_Base_Stop(&htim13);
			HAL_TIM_Base_Stop(&htim14);
			HAL_TIM_Base_Stop(&htim6);
	 }
}

#define TARGET_ADDR       0x081E0000UL // 你要写入的目标地址

/**
 * @brief  擦除目标扇区（和 ST-LINK 擦“127”是同一个128KB扇区）
 * @retval true=擦除成功，false=失败
 */
static bool FlashErase(void)
{
    FLASH_EraseInitTypeDef erase_cfg = {0};
    uint32_t err_sector = 0;

    // 完全用 CubeMX 生成的默认宏，不额外加自定义参数
    erase_cfg.TypeErase   = FLASH_TYPEERASE_SECTORS;
    erase_cfg.Banks       = FLASH_BANK_2;
    erase_cfg.Sector      = FLASH_SECTOR_7;
    erase_cfg.NbSectors   = 1U;
    erase_cfg.VoltageRange= FLASH_VOLTAGE_RANGE_3; // 若 Power 配置是 Scale2，就改 FLASH_VOLTAGE_RANGE_2

    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS_BANK2); // CubeMX 例程标准步骤，避免残留错误

    // 执行擦除+等待硬件完成
    HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&erase_cfg, &err_sector);
    if (status == HAL_OK)
    {
        status = FLASH_WaitForLastOperation(100U,FLASH_BANK_2); // 等待擦除结束，100ms 足够
    }

    HAL_FLASH_Lock();
    return (status == HAL_OK);
}

/**
 * @brief  写入数据到目标地址（先擦除再写入，稳定可靠）
 * @param  cfg：要写入的 TProductConfig 类型数据
 * @retval true=写入成功，false=失败
 */
bool FlashWrite(TProductConfig *cfg)
{
    if (!cfg) return false;

    // 步骤1：先擦除扇区（Flash 必须先擦后写）
    if (!FlashErase())
    {
        return false; // 擦除失败直接返回
    }

    // 步骤2：写入数据（32字节编程，和你原来的模式一致）
    HAL_FLASH_Unlock();
    uint32_t write_addr = TARGET_ADDR;
    uint32_t *p_data = (uint32_t *)cfg;
    uint32_t write_count = sizeof(TProductConfig) / 32U; // 32字节/次

    for (uint32_t i = 0; i < write_count; i++)
    {
        // 编程前确保 Flash 空闲
        if (FLASH_WaitForLastOperation(50U,FLASH_BANK_2) != HAL_OK)
        {
            HAL_FLASH_Lock();
            return false;
        }

        // 用你原来的 32字节编程模式（FLASH_TYPEPROGRAM_FLASHWORD）
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, write_addr, (uint32_t)(p_data + 8*i)) != HAL_OK)
        {
            HAL_FLASH_Lock();
            return false;
        }
        write_addr += 32U; // 每次偏移32字节
    }

    // 等待编程完成+锁定
    FLASH_WaitForLastOperation(50U,FLASH_BANK_2);
    HAL_FLASH_Lock();

    // 步骤3：读回校验，确保写入正确
    TProductConfig cfg_read;
    memcpy(&cfg_read, (void *)TARGET_ADDR, sizeof(TProductConfig));
    return (memcmp(cfg, &cfg_read, sizeof(TProductConfig)) == 0);
}

/**
 * @brief  从目标地址读取数据
 * @param  cfg：存储读取结果的缓冲区
 * @retval true=读取成功，false=失败
 */
bool FlashRead(TProductConfig *cfg)
{
    if (!cfg) return false;

    // 直接读取目标地址数据
    memcpy(cfg, (void *)TARGET_ADDR, sizeof(TProductConfig));
    return true;
}
/************************ (C) COPYRIGHT ACG co. *****END OF FILE****/
