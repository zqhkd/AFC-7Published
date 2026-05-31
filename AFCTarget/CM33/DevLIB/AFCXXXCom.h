#ifndef __AFC_XXX_Com_H__
#define __AFC_XXX_Com_H__

#include <stdint.h>
#include <stdbool.h>
#include <usart.h>
#include "ACGRingBuffAPI.h"

#define XCom_MAX_Tx_SIZE  255
#define XCom_MAX_Rx_SIZE  255

void initXComPara(void);

// 初始化通用输出串口
void initXComOut(uint8_t idxComPort,uint32_t comBaudRate);

void writeXCom(uint8_t idxComPort, uint16_t iBytes,uint8_t iVal);

// 初始化XComIn
void initXComIn(uint8_t idxComPort,uint32_t comBaudRate,uint8_t iFrame);

// 处理XCom的空闲中断DMA：接收XCom字符串
void ProXComRcvIRQ(uint8_t idxComPort);

// XCom的帧接收超时处理
void vXComFrameOverTimePro(uint16_t overTime);

// 从模型串口XCom读取变量到模型端口(ReadMC模块)，供用户模型Simulink程序使用。
uint8_t readXCom(uint8_t idxComPort,uint8_t idxByte);

#endif

/************************ (C) COPYRIGHT ACG co. *****END OF FILE****/
