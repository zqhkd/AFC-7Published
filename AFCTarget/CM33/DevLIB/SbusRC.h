#ifndef __SBUSRC_H__
#define __SBUSRC_H__

#include "stdint.h"
#include "stdbool.h"
//	
//#ifdef CRSF_ENABLE
//	#define SBusChMax   0x712
//	#define SBusChMin   0x0AC
//#else
//	#define SBusChMax   0x69F
//	#define SBusChMin   0x160
//#endif
//extern uint16_t RC_Input[16];

extern uint16_t channel_raw[16];
extern uint16_t iSbusErrNum;

void Sbus_Read_Start(void);
void Sbus_Store_Byte(uint8_t byte);
void Sbus_Read_Cnt(void);

// 初始化DMA模式的SBusCom(Usart1)。最关键问题是打开空闲中断
void InitSBusComDMARcv(void);
void ProcSBusComRcvIRQ(void);

#endif


