#ifndef __AFC_Test_Fun_H__
#define __AFC_Test_Fun_H__

#include "stm32h7xx_hal.h"
#include "stdio.h"
#include "string.h"

#define CurTestCom             huart5
#define MAXCOMTESTBUFFLENGTH    100

extern char gTestComSendBuff[MAXCOMTESTBUFFLENGTH];
extern uint8_t gTestComRcvBuff[MAXCOMTESTBUFFLENGTH];
extern uint8_t gTestComRcvLen;

void InitTestComRcv(void);
void ProTestComRcvIRQ(UART_HandleTypeDef *huart);
void vMainTestCom(void);

void vBasicFuncTest(void);

#endif

