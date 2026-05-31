#include "PwmIn.h"
#include "tim.h"

uint32_t pwm_in_val[2];  //捕获值
uint16_t tim2_cnt[2];
uint8_t  TIM2CH1_CAPTURE_STA=0;
uint8_t  TIM2CH2_CAPTURE_STA=0;	//输入捕获状态	
uint32_t	TIM2CH1_CAPTURE_VAL;
uint32_t	TIM2CH2_CAPTURE_VAL;	//输入捕获值(TIM2/TIM5是32位)

//定时器更新中断（计数溢出）中断处理回调函数， 该函数在HAL_TIM_IRQHandler中会被调用
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)//更新中断（溢出）发生时执行
{
	tim2_cnt[0]++;
	if(htim->Channel==HAL_TIM_ACTIVE_CHANNEL_1) {
		if((TIM2CH1_CAPTURE_STA&0X80)==0)//还未成功捕获
		{
				if(TIM2CH1_CAPTURE_STA&0X40)//已经捕获到高电平了
				{
					if((TIM2CH1_CAPTURE_STA&0X3F)==0X3F)//高电平太长了
					{
						TIM2CH1_CAPTURE_STA|=0X80;		//标记成功捕获了一次
						TIM2CH1_CAPTURE_VAL=0XFFFFFFFF;
					}else TIM2CH1_CAPTURE_STA++;
				}	 
		}	
	} else if(htim->Channel==HAL_TIM_ACTIVE_CHANNEL_2) {
		if((TIM2CH2_CAPTURE_STA&0X80)==0)//还未成功捕获
		{
				if(TIM2CH2_CAPTURE_STA&0X40)//已经捕获到高电平了
				{
					if((TIM2CH2_CAPTURE_STA&0X3F)==0X3F)//高电平太长了
					{
						TIM2CH2_CAPTURE_STA|=0X80;		//标记成功捕获了一次
						TIM2CH2_CAPTURE_VAL=0XFFFFFFFF;
					}else TIM2CH2_CAPTURE_STA++;
				}	 
		}	
	}
}
//定时器输入捕获中断处理回调函数，该函数在HAL_TIM_IRQHandler中会被调用
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)//捕获中断发生时执行
{
	tim2_cnt[1]++;
	if(htim->Channel==HAL_TIM_ACTIVE_CHANNEL_1) {
		if((TIM2CH1_CAPTURE_STA&0X80)==0)//还未成功捕获
		{
			if(TIM2CH1_CAPTURE_STA&0X40)		//捕获到一个下降沿 		
			{	  			
				TIM2CH1_CAPTURE_STA = 0;
				TIM2CH1_CAPTURE_STA|=0X80;		//标记成功捕获到一次高电平脉宽
				TIM2CH1_CAPTURE_VAL=HAL_TIM_ReadCapturedValue(&htim2,TIM_CHANNEL_1);//获取当前的捕获值.
				TIM_RESET_CAPTUREPOLARITY(&htim2,TIM_CHANNEL_1);   //一定要先清除原来的设置！！
				TIM_SET_CAPTUREPOLARITY(&htim2,TIM_CHANNEL_1,TIM_ICPOLARITY_RISING);//上升沿捕获
			}else  								//还未开始,第一次捕获上升沿
			{
				TIM2CH1_CAPTURE_STA=0;			//清空
				TIM2CH1_CAPTURE_VAL=0;
				TIM2CH1_CAPTURE_STA|=0X40;		//标记捕获到了上升沿
				__HAL_TIM_DISABLE(&htim2);        //关闭定时器
				__HAL_TIM_SET_COUNTER(&htim2,0);
				TIM_RESET_CAPTUREPOLARITY(&htim2,TIM_CHANNEL_1);   //一定要先清除原来的设置！！
				TIM_SET_CAPTUREPOLARITY(&htim2,TIM_CHANNEL_1,TIM_ICPOLARITY_FALLING);//设置为下降沿捕获
				__HAL_TIM_ENABLE(&htim2);//使能定时器
			}		
		}
		
		if(TIM2CH1_CAPTURE_STA&0X80)//成功捕获到了一次上升沿
		{
			if(TIM2CH1_CAPTURE_VAL<0x0FFFFFFF)
				pwm_in_val[1] = TIM2CH1_CAPTURE_VAL;
			TIM2CH1_CAPTURE_STA=0;//开启下一次捕获
		}
		
	} else if(htim->Channel==HAL_TIM_ACTIVE_CHANNEL_2) {
		if((TIM2CH2_CAPTURE_STA&0X80)==0)//还未成功捕获
		{
			if(TIM2CH2_CAPTURE_STA&0X40)		//捕获到一个下降沿 		
			{	  			
				TIM2CH2_CAPTURE_STA = 0;
				TIM2CH2_CAPTURE_STA|=0X80;		//标记成功捕获到一次高电平脉宽
				TIM2CH2_CAPTURE_VAL=HAL_TIM_ReadCapturedValue(&htim2,TIM_CHANNEL_2);//获取当前的捕获值.
				TIM_RESET_CAPTUREPOLARITY(&htim2,TIM_CHANNEL_2);   //一定要先清除原来的设置！！
				TIM_SET_CAPTUREPOLARITY(&htim2,TIM_CHANNEL_2,TIM_ICPOLARITY_RISING);//配置上升沿捕获
			}else  								//还未开始,第一次捕获上升沿
			{
				TIM2CH2_CAPTURE_STA=0;			//清空
				TIM2CH2_CAPTURE_VAL=0;
				TIM2CH2_CAPTURE_STA|=0X40;		//标记捕获到了上升沿
				__HAL_TIM_DISABLE(&htim2);        //关闭定时器
				__HAL_TIM_SET_COUNTER(&htim2,0);
				TIM_RESET_CAPTUREPOLARITY(&htim2,TIM_CHANNEL_2);   //一定要先清除原来的设置！！
				TIM_SET_CAPTUREPOLARITY(&htim2,TIM_CHANNEL_2,TIM_ICPOLARITY_FALLING);//设置为下降沿捕获
				__HAL_TIM_ENABLE(&htim2);//使能定时器
			}		
		}
		
		if(TIM2CH2_CAPTURE_STA&0X80)//成功捕获到了一次上升沿
		{
			if(TIM2CH2_CAPTURE_VAL<0x0FFFFFFF)
				pwm_in_val[0] = TIM2CH2_CAPTURE_VAL;
			TIM2CH2_CAPTURE_STA=0;//开启下一次捕获
		}
	}		
}


