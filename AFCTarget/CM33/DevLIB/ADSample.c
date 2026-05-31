/******************** (C) COPYRIGHT 2019 ACE Tech Co.*************************
 * 作者	  ： 曾庆华
 * 文件名  ADSample.c
 * 版  本 ： 
 *    
 * 
 * 描述   ：ADSample地磁传感器芯片底层接口函数
 * 官网   ：www.acecreator.com
 * 淘宝   ：acecreator.taobao.com
 * 公众号 ：无人飞行控制
 *
*****************************************************************************/
	
#include "main.h"
#include "adc.h"
#include "ADSample.h"

//获得ADC值
//ch: 通道值 0~16，取值范围为：ADC_CHANNEL_0~ADC_CHANNEL_16
//返回值:转换结果

// 读取ADC1值
uint16_t readADC1Value(void)
{
    // 启动ADC1转换
    if (HAL_ADC_Start(&hadc1) != HAL_OK)
    {
        Error_Handler();
    }
    
    // 等待转换完成
    if (HAL_ADC_PollForConversion(&hadc1, 100) != HAL_OK)
    {
        Error_Handler();
    }
    
    // 返回转换结果 (16位分辨率)
    return HAL_ADC_GetValue(&hadc1);
}

// 读取ADC2值
uint16_t readADC2Value(void)
{
    // 启动ADC2转换
    if (HAL_ADC_Start(&hadc2) != HAL_OK)
    {
        Error_Handler();
    }
    
    // 等待转换完成
    if (HAL_ADC_PollForConversion(&hadc2, 100) != HAL_OK)
    {
        Error_Handler();
    }
    
    // 返回转换结果 (16位分辨率)
    return HAL_ADC_GetValue(&hadc2);
}

uint16_t Get_Adc(uint32_t ch)   
{
		ADC_ChannelConfTypeDef ADC1_ChanConf;

		ADC1_ChanConf.Channel=ch;                                   //通道
		ADC1_ChanConf.Rank=ADC_REGULAR_RANK_1;                  	//1个序列
		ADC1_ChanConf.SamplingTime=ADC_SAMPLETIME_16CYCLES_5;      	//采样时间       
		ADC1_ChanConf.SingleDiff=ADC_SINGLE_ENDED;  				//单边采集          		
		ADC1_ChanConf.OffsetNumber=ADC_OFFSET_NONE;             	
		ADC1_ChanConf.Offset=0;   
		HAL_ADC_ConfigChannel(&hadc1,&ADC1_ChanConf);        //通道配置

		HAL_ADC_Start(&hadc1);                               //开启ADC

		HAL_ADC_PollForConversion(&hadc1,10);                //轮询转换
		return (uint16_t)HAL_ADC_GetValue(&hadc1);	            //返回最近一次ADC1规则组的转换结果
}

uint16_t ad_vol[2];
//void ADSample_ReadData(void) 
//{
//	static uint8_t vol_dir = 0;
//	if(vol_dir==0) {
//		ad_vol[0] = Get_Adc(ADC_CHANNEL_10);vol_dir = 1;
//	}else if(vol_dir==1) {
//		ad_vol[1] = Get_Adc(ADC_CHANNEL_11);vol_dir = 0;
//	}
//}

void ADSample_ReadData(void) 
{
	static uint8_t vol_dir = 0;
	if(vol_dir==0) {
		ad_vol[0] = readADC1Value();vol_dir = 1;
	}else if(vol_dir==1) {
		ad_vol[1] = readADC2Value(); vol_dir = 0;
	}
}
/************************ (C) COPYRIGHT ACE Co. about ADIS164XX *****END OF FILE****/
