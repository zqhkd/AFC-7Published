#ifndef __AFC_Cdio_Com_H__
#define __AFC_Cdio_Com_H__

#include <stdint.h>
#include <stdbool.h>
#include <usart.h>

#define CdioCom_MAX_Tx_SIZE  255
#define CdioCom_MAX_Rx_SIZE  150

// 外部可使用全局变量
extern uint8_t g_iMaxCdioOutFrameNum, g_idxCurCdioFrame;


// 将数传串口输入/输出信号数量全部清零
void ClrCdioComSignalNum(void);

// 初始化数传串口输出的CdioCom。
void initCdioComOut(uint8_t iSigNum,uint8_t iFrameType);
// 初始化数传串口输入的CdioCom。
void initCdioComIn(uint8_t iSigNum,uint8_t iFrameType,uint8_t iResFrameNum);

// 传递指定的测试参数给数传串口
void writeCdioCom(uint8_t iFrameType,uint8_t iChannel,double fVal);
// 从CdioCom端口读取数据至总线
double readCdioCom(uint8_t iPortNo,uint8_t idx);

// CdioCom中断处理函数
void ProCdioComRcvIRQ(void);

#endif

/************************ (C) COPYRIGHT ACG co. *****END OF FILE****/
