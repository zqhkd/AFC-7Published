#include "AFCTestFun.h"
#include "AFCGlobalVar.h"
#include "main.h"
#include "AFCDio.h"
#include "IST8310.h"
#include "DPS310.h"
#include "ICM20602.h"
#include "ICM42688.h"
#include "usart.h"
#include "SbusRC.h"
#include "PwmOut.h"
#include "PwmIn.h"
#include "ADSample.h"
#include "FM25V01.h"

#include "ACGCommonAPI.h"

uint8_t dma_buf[23];
uint32_t loop_test_cnt = 0;
void vBasicFuncTest(void)  
{
	if(g_sRealTimeCount.check_flag >= 1 ) {  //1kHz
		g_sRealTimeCount.check_flag = 0;
		loop_test_cnt++;
		
		if(loop_test_cnt%20==0) {  //50Hz
			DPS310_ReadData();
			IST8310_ReadData();
			ICM20602_ReadData();
			ICM42688_ReadData();
		}
		
		if(loop_test_cnt%100==0) {  //10Hz
			SDtest();
			PWM_Out_Test();
			ADSample_ReadData();
		}
		
		if(loop_test_cnt%1000==0) {  //1Hz
			All_LED_Toggle();
			uint8_t buf[5] = {0x11,0x22,0x33,0x44,0x55};
			HAL_UART_Transmit_DMA(&huart5,buf,5);//开启DMA传输
			HAL_UART_Receive_DMA(&huart5,dma_buf,10);
			
			FM25V01_Test();
		}
	}
}

char gTestComSendBuff[MAXCOMTESTBUFFLENGTH];
uint8_t gTestComRcvBuff[MAXCOMTESTBUFFLENGTH];
uint8_t gTestComRcvLen=0;

void InitTestComRcv(void)
{
	  InitComDMA(&CurTestCom,gTestComRcvBuff,MAXCOMTESTBUFFLENGTH);
}

void ProTestComRcvIRQ(UART_HandleTypeDef *huart)
{
	  if(huart == &CurTestCom)	gTestComRcvLen = ProcComRcvIRQ(huart,gTestComRcvBuff,MAXCOMTESTBUFFLENGTH);
}

void vMainTestCom(void)
{
	  static uint8_t i=0,j=100;
	  uint8_t len;
	  i++,j++;
		
		static uint16_t iRcvNum = 1;
		sprintf(gTestComSendBuff,"I%d,    J%d\r\n",i,j);  // 必须加上\r\n才能达到xCom端显示换行效果
		
		len = strlen(gTestComSendBuff);
		//  串口查询发送操作，只用如下这句话即可。
//		HAL_UART_Transmit(&CurTestCom,(uint8_t *)gTestComSendBuff,len,0xffff);

		// 进行串口DMA接收操作时，需要进行DMA接收的相关初始化操作，即前述调用InitComDMA。
		if(gTestComRcvLen>0){
	// 仅进行串口DMA发送的话，只要在cubeMx中设置好发送DMA、打开全部串口接收及DMA中断即可，不用进行其它初始化操作。
			HAL_Delay(100);
			sprintf(gTestComSendBuff,"第%03d次收到串口数据，内容为：%s \r\n",iRcvNum++,gTestComRcvBuff);
			len = strlen(gTestComSendBuff);
			gTestComRcvLen = 0;
		}
		
		// 仅进行串口DMA发送的话，只要在cubeMx中设置好发送DMA、打开全部串口接收及DMA中断即可，不用进行其它初始化操作。
		HAL_UART_Transmit_DMA(&CurTestCom,(uint8_t *)gTestComSendBuff,len);  
		HAL_Delay(500);
}
