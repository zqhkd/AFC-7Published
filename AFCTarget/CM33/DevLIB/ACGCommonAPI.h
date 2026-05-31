#ifndef __ACG_Common_API_H__
#define __ACG_Common_API_H__

#include "stdint.h"
#include "usart.h"
#include <stdbool.h>

#include "AFCGlobalDef.h"

// 获取当前无人机集群内ID标识号（目前取UavId的低16位值）
uint16_t getCurUAVId(void);

// 安装方向系数：bDir表正装，返回1；否则反装，返回-1
int8_t iXYZDir(bool bDir);

// 根据变量符号返回1或-1的函数
int8_t sign(float fVal);

// V5.04.240717: 添加函数用于将结构体转换为浮点数组
uint8_t structToFloatArray(float *fArray, const void* structPtr, size_t structSize);
// 函数用于将浮点数组转换回结构体
uint8_t floatArrayToStruct(void* structPtr, const float* floatArray, size_t structSize);

void SoftDelayNus(uint32_t iTime);    // 软件延时us程序
uint32_t SoftDelayNms(uint16_t iTime);

void delay_us(uint32_t us); //C文件中的函数声明

uint32_t HAL_GetTick_us(void);   // 获取当前时刻值us

uint8_t CalChkSum(uint8_t *str,uint8_t len);  // 求取8位累加校验和

// 自V4.01.191219开始，遥测数据校验方式更改为CRC16-CCITT，引入CRC16计算函数。
uint16_t CalCRC16(uint8_t * data, int length);

bool bChkCRC16(uint8_t * data, int length);
bool bCalChkSum(uint8_t *str,uint8_t len);  // 检验累加校验和是否为0

// 检测帧数据的有效性,如果是一帧有效帧则返回true，否则返回false
bool bChkFrameValid(uint8_t *pStr,uint8_t iChkMode, uint16_t len);
// 该函数可以取代HAL_UART_DMAStop，修正了STM32H7XX芯片的DMA操作时的某些不足
HAL_StatusTypeDef HAL_UART_DMAStopRx(UART_HandleTypeDef *huart);
// 由于同时处理串口发送和接收时，会导致丢帧，重写串口发送函数HAL_UART_Transmit_DMA()
void dma_send(UART_HandleTypeDef *huart, unsigned char *buffer,unsigned int length);

// V1.01.210829： 将帧头控制信息及指定数量的浮点数、以及CRC校验码等一起打包到pSendBuff缓冲区中，通过DMA中断方式发送出去。
//     帧头控制信息 pHeaderFram: 包括iFrameID、iFrameLen、iFrameTick、idSender、idReceiver、iFrameType。调用前主要是准备iFrameTick、idSender、idReceiver、iFrameType。
//     传输参数数量 iSignalNum:
//     传输参数值  pData:        为一浮点数组，每一个数组元素是需要传输的参数
//     发送缓冲区  pSendBuff：  本函数执行完毕，将把相关参数打包至该缓冲区中，待串口发送程序进行传输处理
uint16_t iPackData2Hex(THeaderFrame pHeaderFrame, uint8_t iSignalNum, float *pData, uint8_t *pSendBuff, uint8_t iChkMode);
uint16_t iPackData2Hex2(THeaderFrame pHeaderFrame, uint8_t iSignalNum, uint8_t *pData, uint8_t *pSendBuff, uint8_t iChkMode);
// V1.01.210829： 从串口接收缓冲区中检验帧数据的有效性，并解包帧协议中的参数值。
//         返回值为当前处理帧的帧类型码
uint8_t iUnPackHex2Data(uint8_t *comRxBuf,uint8_t iRcvLen,float *pData, uint8_t iChkMode);

// 一个存储了ID标识码信息的缓冲idBuff, 本函数可在idBuff缓冲中找到标识码为idVal的数组索引值
int iFindIdBuffIdx(uint8_t idVal,uint8_t iMax,uint8_t *idBuff);

// 检查输出帧队列中是否包含iFrameTypeCode, 如果没有找到会返回最大值iMaxFrameNum。
uint8_t iChkFrameTypeIdx(uint8_t iFrameTypeCode,uint8_t iMaxFrameNum,uint8_t *iFrameType);

// 初始化DMA模式串口huart
void InitComDMA(UART_HandleTypeDef *huart, uint8_t *pDmaRcvBuff,uint16_t iMaxBuffSize);

// 处理串口接收中断
uint8_t ProcComRcvIRQ(UART_HandleTypeDef *huart , uint8_t *pDmaRcvBuff,uint16_t iMaxBuffSize);

// 滑动滤波器
void vSlideWinAverage(double fMoveArr[],uint8_t iSlideWinLength, uint8_t *iPos, double fIn, double *fOut,bool *bSlideWinFull);

// V4.02.220504: 禁止所有中断
void DiableAllIrq(void);

// 使能或禁止任务定时器中断
void EnTaskTimerIRQ(bool bEnable);

// 根据当前缓冲区字节数，计算包含多少个float（4字节）
uint8_t iCalFloatNums(uint8_t iBytes);

// DWT硬件精确延时Nus方案
void InitDWT(void);
void DWTDelayNus(uint32_t us);

// 系统配置参数信息处理
bool FlashWrite(TProductConfig *cfg);
bool FlashRead(TProductConfig *cfg);
#endif

/************************ (C) COPYRIGHT ACG co. *****END OF FILE****/
