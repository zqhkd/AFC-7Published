#ifndef __AFCDio_H__
#define __AFCDio_H__

#include "stdint.h"
#include "stdbool.h"
#include "main.h"

#define LED_White_On     HAL_GPIO_WritePin(GPO1_GPIO_Port,GPO1_Pin,GPIO_PIN_SET)
#define LED_White_Off    HAL_GPIO_WritePin(GPO1_GPIO_Port,GPO1_Pin,GPIO_PIN_RESET)
#define LED_White_Toggle HAL_GPIO_TogglePin(GPO1_GPIO_Port,GPO1_Pin)

#define LED_Yellow_On     HAL_GPIO_WritePin(GPO2_GPIO_Port,GPO2_Pin,GPIO_PIN_SET)
#define LED_Yellow_Off    HAL_GPIO_WritePin(GPO2_GPIO_Port,GPO2_Pin,GPIO_PIN_RESET)
#define LED_Yellow_Toggle HAL_GPIO_TogglePin(GPO2_GPIO_Port,GPO2_Pin)

#define LED_Green_On      HAL_GPIO_WritePin(GPO3_GPIO_Port,GPO3_Pin,GPIO_PIN_SET)
#define LED_Green_Off     HAL_GPIO_WritePin(GPO3_GPIO_Port,GPO3_Pin,GPIO_PIN_RESET)
#define LED_Green_Toggle  HAL_GPIO_TogglePin(GPO3_GPIO_Port,GPO3_Pin)

#define LED_Blue_On       HAL_GPIO_WritePin(GPO4_GPIO_Port,GPO4_Pin,GPIO_PIN_SET)
#define LED_Blue_Off      HAL_GPIO_WritePin(GPO4_GPIO_Port,GPO4_Pin,GPIO_PIN_RESET)
#define LED_Blue_Toggle   HAL_GPIO_TogglePin(GPO4_GPIO_Port,GPO4_Pin)

#define LED_Red_On     	  HAL_GPIO_WritePin(GPO5_GPIO_Port,GPO5_Pin,GPIO_PIN_SET)
#define LED_Red_Off     	HAL_GPIO_WritePin(GPO5_GPIO_Port,GPO5_Pin,GPIO_PIN_RESET)
#define LED_Red_Toggle    HAL_GPIO_TogglePin(GPO5_GPIO_Port,GPO5_Pin)

void All_LED_Toggle(void);

// 置LED指示灯状态
void setLedSts(uint8_t iCh,bool sts);

// 置运行指示灯状态
void setRunSts(bool sts);
/* 读入开关（按钮）的状态  */
bool getStartSwitch(void);
// 时序指令开关
void setSeqSts(uint8_t iCh,bool sts);
#endif


