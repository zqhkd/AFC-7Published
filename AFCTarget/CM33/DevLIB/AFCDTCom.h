#ifndef __AFC_DT_Com_H__
#define __AFC_DT_Com_H__

#include <stdint.h>
#include <stdbool.h>
#include <usart.h>
#include "AFCGlobalDef.h"
#include "ACGRingBuffAPI.h"

#define DtCom_MAX_Tx_SIZE  255
#define DtCom_MAX_Rx_SIZE  255

// 外部可使用全局变量
extern bool g_bUsedOfDtCom, g_bUsedOfTWDtCom, bInitDtComIn;
//extern uint8_t g_iMaxDtOutFrameNum, g_idxCurDtFrame;
extern TRingBuffer *lpDtOutQueue;

// 将数传串口输入/输出信号数量全部清零
void ClrDtComSignalNum(void);

// 初始化模型输出串口
void initDtComOut(uint8_t iSigNum,uint8_t iFrameType);
//// 初始化数传串口输出的DtCom。
//void initTWDtComOut(bool bICM42688,bool bICM20602,bool bDPSIST,bool bGPS, bool bRemoter);
// 初始化数传串口输入的DtCom。
void initDtComIn(uint8_t iSigNum,uint8_t iFrameType,uint8_t iResFrameNum);

// 传递指定的测试参数给数传串口
void writeDtCom(uint8_t iFrameType,uint8_t iChannel,double fVal);
// 从DtCom端口读取数据至总线
double readDtCom(uint8_t idFrameType,uint8_t iChanel);

// 通过数传串口发送信息，DMA方式
void DataDMA2DtCom(uint8_t idxCurFrame);
// DtCom中断处理函数
void ProDtComRcvIRQ(void);

// 组件设备测试结果输出给地面站
void CompTest2Gs(void);

// 关闭测试校准时打开的相关设备使用标志，这些设备当前程序是未使用的，仅是在测试/校准状态而打开，因此正常运行时需要关闭
void vCloseTestCalibUsedFlg(void);

// 从地面站发送调节参数或请求参数指令
void initDtComCtrlPara(void);
// 处理飞控算法调节参数响应帧
void DtCom2CtrlParam(void);

// 为地面站系统级参数获取指令帧，做好初始化工作
void initGs2FcsCmdFrame(void);
// 处理地面站注入到飞控组件的指令帧
void ProGs2FcsCmdFrame(void);
#endif

/************************ (C) COPYRIGHT ACG co. *****END OF FILE****/
