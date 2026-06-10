#include "AFCDio.h"
#include "AFCGlobalVar.h"

void All_LED_Toggle(void)
{
	LED_White_Toggle;
	LED_Yellow_Toggle;
	LED_Green_Toggle;
	LED_Blue_Toggle;
	LED_Red_Toggle;
}

void setLedSts(uint8_t iCh,bool sts)
{
	switch(iCh){
		case 0:
				if(sts) LED_Yellow_On;
		    else LED_Yellow_Off;
		    break;
		case 1:
				if(sts) LED_Green_On;
		    else LED_Green_Off;
		    break;
		case 2:
				if(sts) LED_Blue_On;
		    else LED_Blue_Off;
		    break;
		case 3:
				if(sts) LED_Red_On;
		    else LED_Red_Off;
		    break;
	}
}

void setSeqSts(uint8_t iCh,bool sts)
{
	switch(iCh){
		case 0:
				if(sts) HAL_GPIO_WritePin(GPO1_GPIO_Port,GPO1_Pin,GPIO_PIN_SET);
		    else HAL_GPIO_WritePin(GPO1_GPIO_Port,GPO1_Pin,GPIO_PIN_RESET);
		    break;
		case 1:
				if(sts) HAL_GPIO_WritePin(W25Q01JV_CS_GPIO_Port,W25Q01JV_CS_Pin,GPIO_PIN_SET);
		    else HAL_GPIO_WritePin(W25Q01JV_CS_GPIO_Port,W25Q01JV_CS_Pin,GPIO_PIN_RESET);
		    break;
		case 2:
				if(sts) HAL_GPIO_WritePin(AT7456E_CS_GPIO_Port,AT7456E_CS_Pin,GPIO_PIN_SET);
		    else HAL_GPIO_WritePin(AT7456E_CS_GPIO_Port,AT7456E_CS_Pin,GPIO_PIN_RESET);
		    break;
		case 3:
				if(sts) HAL_GPIO_WritePin(GPO2_GPIO_Port,GPO2_Pin,GPIO_PIN_SET);
		    else HAL_GPIO_WritePin(GPO2_GPIO_Port,GPO2_Pin,GPIO_PIN_RESET);
		    break;
		case 4:
				if(sts) HAL_GPIO_WritePin(GPO3_GPIO_Port,GPO3_Pin,GPIO_PIN_SET);
		    else HAL_GPIO_WritePin(GPO3_GPIO_Port,GPO3_Pin,GPIO_PIN_RESET);
		    break;
		case 5:
				if(sts) HAL_GPIO_WritePin(GPO4_GPIO_Port,GPO4_Pin,GPIO_PIN_SET);
		    else HAL_GPIO_WritePin(GPO4_GPIO_Port,GPO4_Pin,GPIO_PIN_RESET);
		    break;
		case 6:
				if(sts) HAL_GPIO_WritePin(GPO5_GPIO_Port,GPO5_Pin,GPIO_PIN_SET);
		    else HAL_GPIO_WritePin(GPO5_GPIO_Port,GPO5_Pin,GPIO_PIN_RESET);
		    break;
		case 7:
				if(sts) HAL_GPIO_WritePin(GPO6_GPIO_Port,GPO6_Pin,GPIO_PIN_SET);
		    else HAL_GPIO_WritePin(GPO6_GPIO_Port,GPO6_Pin,GPIO_PIN_RESET);
		    break;
	}
}

void setRunSts(bool sts)
{
		if(sts) LED_White_On;
		else LED_White_Off;
}

/* 读入开关（按钮）的状态  */
bool getStartSwitch(void)
{
	bool sts = false;
//	sts = (bool)(HAL_GPIO_ReadPin(ISI0_GPIO_Port,GPIO_PIN_7);
	return sts;
}
