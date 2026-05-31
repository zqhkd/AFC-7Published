#ifndef __AFC_GPU_Com_H__
#define __AFC_GPU_Com_H__

#include "stdint.h"
#include "AFCGlobalDef.h"

extern bool g_bUsedOfGpuCom;

// 初始化模型串口DMA接收通道
void InitGpuComDMARcv(void);
// 采用DMA模式接收ModelCom的信息任务
//void RcvGpuComInfTask(void);
// 处理ModelCom的空闲中断DMA：接收ModelCom字符串
void ProcGpuComRcvIRQ(void);
	
// 通过测试端口发送信息，DMA方式
void DMAWriteData2GpuCom(void);

// 将g_sModelComTxBuf中的len个字符输出
void DMAWriteStr2GpuCom(uint16_t len);

// 初始化GPU串口
void initWriteGpuCom(uint8_t iSigNum,uint8_t iFrameType);
// 传递指定的测试参数给GPU串口
void writeGpuCom(uint8_t iChannel,double fVal);
// 以下两个函数initReadModelCom、getModelComVal是和readModelComAPI.c及.tlc程序配合使用的函数
void initReadGpuCom(uint8_t iSigNum,uint8_t iFrameType);
// 从GPU串口读取变量到GPU端口(ReadGPU模块)，供用户模型Simulink程序使用。
double readGpuCom(uint8_t iChannel);

#endif

/************************ (C) COPYRIGHT ACG co. *****END OF FILE****/
