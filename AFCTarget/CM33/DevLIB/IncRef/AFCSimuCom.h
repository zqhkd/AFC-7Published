#ifndef __AFC_Simu_Com_H__
#define __AFC_Simu_Com_H__

#include <stdint.h>
#include <stdbool.h>
#include <usart.h>
#include "ACGRingBuffAPI.h"

#define SimuCom_MAX_Tx_SIZE  255
#define SimuCom_MAX_Rx_SIZE  255

// 外部可使用全局变量
extern bool g_bUsedOfSimuCom,bInitSimuComIn;
extern TRingBuffer *lpSimuOutQueue;


// 将数传串口输入/输出信号数量全部清零
void ClrSimuComSignalNum(void);

// 初始化数传串口输出的SimuCom。
void initSimuComOut(uint8_t iSigNum,uint8_t iFrameType);
// 初始化数传串口输入的SimuCom。
void initSimuComIn(uint8_t iSigNum,uint8_t iFrameType,uint8_t iResFrameNum);

// 传递指定的测试参数给数传串口
void writeSimuCom(uint8_t iFrameType,uint8_t iChannel,double fVal);
// 从SimuCom端口读取数据至总线
double readSimuCom(uint8_t iPortNo,uint8_t idx);

// 通过模型串口发送信息，DMA方式
void DataDMA2SimuCom(uint8_t idxCurFrame);

// SimuCom中断处理函数
void ProSimuComRcvIRQ(void);

// 从环形缓冲区中提取帧内容，解析SimuCom的传送数据
void vRcvSimuComInfTask(void);

#endif

/************************ (C) COPYRIGHT ACG co. *****END OF FILE****/
