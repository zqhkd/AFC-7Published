#include "PwmOut.h"
#include "tim.h"
#include "stm32h7xx_hal_tim.h"

#include "AFCGlobalVar.h"
#include "ACGCommonAPI.h"

// 以下初始化函数可确保：在更改PSC和ARR之前，停止定时器以避免在更新过程中出现错误
void initTimerPscArr(TIM_HandleTypeDef *htim, uint16_t psc, uint32_t arr,bool bStartTime)
{
    // 停止定时器
    __HAL_TIM_DISABLE(htim);

    // 更新预分频器和自动重载值
    htim->Init.Prescaler = psc - 1;
    htim->Init.Period = arr - 1;

    // 更新定时器配置
    if (HAL_TIM_Base_Init(htim) != HAL_OK)
    {
        // 初始化错误处理
        Error_Handler();
    }

    // 清除更新事件标志位（如果设置了UIF）
    __HAL_TIM_CLEAR_IT(htim, TIM_IT_UPDATE);

    // 对于PWM电调，需要启动定时器；而对于DShot电调，不要启动定时器
		if(bStartTime){
//			htim->Instance->CCER = 0x1111;    // 赋能CH1--4均为PWM输出
			__HAL_TIM_ENABLE(htim);
		}
}

void updateTimerFreq(uint8_t iCh,uint16_t preScaleVal,double escFreq,bool bStartTime)
{
	// APB1、APB2频率均为128MHz, 除以预分频值后即可得到定时器输出信号频率。
	//   对于PWM电调，预分频值取128，则计时器输入频率1MHz; 对于DShot电调，预分频值取8，则计时器输入频率16MHz
	double fPreFreq = (128.0/preScaleVal) * 1000000;   
	uint32_t iFreqVal = (uint32_t)(fPreFreq/escFreq + 0.5); 

	switch(iCh){
		case 0:
			initTimerPscArr(&htim3, preScaleVal,iFreqVal,bStartTime);          // PWM1、2均使用TIM3
		  HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_3);
      break;
		case 1:
			initTimerPscArr(&htim3, preScaleVal,iFreqVal,bStartTime);          // PWM1、2均使用TIM3
		  HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_4);
      break;
		case 2:
			initTimerPscArr(&htim1, preScaleVal,iFreqVal,bStartTime);          // PWM3、4、5、6均使用TIM1
			HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_1); 
		  break;
		case 3:
			initTimerPscArr(&htim1, preScaleVal,iFreqVal,bStartTime);          // PWM3、4、5、6均使用TIM1
			HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_2); 
		  break;
		case 4:
			initTimerPscArr(&htim1, preScaleVal,iFreqVal,bStartTime);          // PWM3、4、5、6均使用TIM1
			HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_3); 
		  break;
		case 5:
			initTimerPscArr(&htim1, preScaleVal,iFreqVal,bStartTime);          // PWM3、4、5、6均使用TIM1
			HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_4); 
		  break;
		case 6:
			initTimerPscArr(&htim4, preScaleVal,iFreqVal,bStartTime);          // PWM7、8、9、10均使用TIM4
			HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_1); 
		  break;
		case 7:
			initTimerPscArr(&htim4, preScaleVal,iFreqVal,bStartTime);          // PWM7、8、9、10均使用TIM4
			HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_2); 
		  break;
		case 8:
			initTimerPscArr(&htim4, preScaleVal,iFreqVal,bStartTime);          // PWM7、8、9、10均使用TIM4
			HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_3); 
		  break;
		case 9:
			initTimerPscArr(&htim4, preScaleVal,iFreqVal,bStartTime);          // PWM7、8、9、10均使用TIM4
			HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_4); 
		  break;
		case 10:
			initTimerPscArr(&htim17, preScaleVal,iFreqVal,bStartTime);          // PWM11 均使用TIM17_CH1N
		  HAL_TIM_PWM_Start(&htim17,TIM_CHANNEL_1);        // 先启动主通道    
		  HAL_TIMEx_PWMN_Start(&htim17,TIM_CHANNEL_1);    // 启动互补通道
      break;
		case 11:
			initTimerPscArr(&htim5, preScaleVal,iFreqVal,bStartTime);          // PWM12 均使用TIM5
		  HAL_TIM_PWM_Start(&htim5,TIM_CHANNEL_1);
      break;
	}
}

uint32_t iGetTimerArr(uint8_t iCh)
{
	uint32_t iEscPWMPeriod;
	if(iCh <= 1) iEscPWMPeriod = TIM3->ARR + 1;    
	else if(iCh <= 5) iEscPWMPeriod = TIM1->ARR + 1;
	else if(iCh <= 9) iEscPWMPeriod = TIM4->ARR + 1;
	else if(iCh == 10)iEscPWMPeriod = TIM17->ARR + 1;
	else if(iCh == 11)iEscPWMPeriod = TIM5->ARR + 1;
	
  return iEscPWMPeriod;
}

void Set_Motor_PWM(uint8_t iChanel,uint16_t value)
{
/*  以下为正确的通道配置，正常后可将AFCAPI.c中的映射变换iRealCh去掉  */
//	num = num - 1;
	if(iChanel==0) TIM3->CCR3 = value; 
	else if(iChanel==1) TIM3->CCR4=value;    // TIM1的预分频为128 - 1，计数周期为2500 - 1
	else if(iChanel==2) TIM1->CCR1=value; 
	else if(iChanel==3) TIM1->CCR2=value; 
	else if(iChanel==4) TIM1->CCR3=value;    // TIM4的预分频为128 - 1，计数周期为2500 - 1
	else if(iChanel==5) TIM1->CCR4=value; 
	else if(iChanel==6) TIM4->CCR1=value; 
	else if(iChanel==7) TIM4->CCR2=value; 
	else if(iChanel==8) TIM4->CCR3=value;    // TIM3的预分频为128 - 1，计数周期为2500 - 1
	else if(iChanel==9) TIM4->CCR4=value; 
	else if(iChanel==10) TIM17->CCR1=value;  // TIM17的预分频为128 - 1，计数周期为2500 - 1
	else if(iChanel==11) TIM5->CCR1=value;    // TIM5的预分频为3 - 1，计数周期为47400 - 1
}

typedef struct{
	TIM_HandleTypeDef *hTim;   // 需要使用指针变量
	uint32_t TIM_CHANNEL_No;
}SPwmTimeChannel;

const SPwmTimeChannel PWM_CHANNELS[] = {
    {&htim3, TIM_CHANNEL_3},  // tim3_channel 3
    {&htim3, TIM_CHANNEL_4},  // tim3_channel 4
    {&htim1, TIM_CHANNEL_1},  // tim1_channel 1
    {&htim1, TIM_CHANNEL_2},  // tim1_channel 2
    {&htim1, TIM_CHANNEL_3},  // tim1_channel 3
    {&htim1, TIM_CHANNEL_4},  // tim1_channel 4
    {&htim4, TIM_CHANNEL_1},  // tim4_channel 1
    {&htim4, TIM_CHANNEL_2},  // tim4_channel 2
    {&htim4, TIM_CHANNEL_3},  // tim4_channel 3
    {&htim4, TIM_CHANNEL_4},  // tim4_channel 4
    {&htim17, TIM_CHANNEL_1},  // tim17_channel 1
    {&htim5, TIM_CHANNEL_1},  // tim5_channel 1
};

SPwmTimeChannel* sGetChanelTimeInf(uint8_t iChannel)
{
    if(iChannel < sizeof(PWM_CHANNELS)/sizeof(PWM_CHANNELS[0])) {
        return (SPwmTimeChannel*)&PWM_CHANNELS[iChannel];
    }
    return NULL;
}

void PWM_Out_Test(void)
{
	static uint16_t out_value = 1000;
	static uint8_t dir = 0;
//	uint8_t i;
	
	if(dir) {
		out_value -= 10;
		if(out_value<1000) {
			out_value = 1000;dir = 0;
		}
	}
	else {
		out_value += 10;
		if(out_value>2000) {
			out_value = 2000;dir = 1;
		}
	}
	
	Set_Motor_PWM(g_iM2CHANEL,out_value);
	Set_Motor_PWM(g_iM3CHANEL,out_value);
	Set_Motor_PWM(g_iM1CHANEL,out_value);
	Set_Motor_PWM(g_iM4CHANEL,out_value);
}

// 以下程序为DShot协议电调实现程序。

// 假设 DShot 帧长度为 16 位（包含数据、请求返回位和校验和）
#define DSHOT_FRAME_LENGTH 16
// 基于BetaFlight源码
uint8_t dshotChecksum(uint16_t value, bool telemetry) {
    uint16_t frame = (value << 1) | (telemetry ? 1 : 0);
    return (~(frame ^ (frame >> 4) ^ (frame >> 8))) & 0x0F;
}

// 设置电调的DSHOT指令帧
void Set_Motor_DShot(uint8_t iChanel, uint16_t value)
{
    // 1. 获取定时器配置
    SPwmTimeChannel *sChanelTimeInf = sGetChanelTimeInf(iChanel);
    TIM_HandleTypeDef *htim = sChanelTimeInf->hTim;
    uint32_t channel = sChanelTimeInf->TIM_CHANNEL_No;
	
	// 2. DShot电调的定时计数器输入频率为16MHz, 即输入脉冲周期1/16 us，计数预装值为arrValue, 因此每位bit时长就是预装值加1。
      // 获取定时器的自动重装载值（ARR）
    uint32_t arrValue = iGetTimerArr(iChanel);
//	  uint16_t dshotBitTim = 1 + (arrValue / 16);   // 向上取整

	 // 3. 计算高低电平比较值（70%/30%占空比）: 计算高电平占空比的比较值（70%），和低电平占空比的比较值（30%）
	  uint16_t pwmHiVal,pwmLowVal;
    pwmHiVal = arrValue * 70 / 100; pwmLowVal = arrValue * 30 / 100;
	
	 // 4. 配置定时器参数，并启动，且等待稳定
    __HAL_TIM_SET_COMPARE(htim, channel, pwmLowVal); // 初始低电平
    HAL_TIM_PWM_Start(htim, channel);
    while (__HAL_TIM_GET_FLAG(htim, TIM_FLAG_UPDATE) == RESET); // 等待第一个周期稳定
		
 // 1. 构建18位DShot帧（18位：11数据 + 1请求 + 4CRC + 2帧间隔）
    uint32_t frameData = value;
		frameData = (frameData << 5) | (0 << 4); // [11数据+1请求]
    uint8_t checksum = dshotChecksum(value,false);
    frameData |= (checksum);         // [4CRC] + [2位0]
		
    // 6. 启动PWM并发送数据（MSB first）
    for (int i = DSHOT_FRAME_LENGTH - 1; i >=0 ; i--) {
        __HAL_TIM_SET_COMPARE(htim, channel, (frameData & (1 << i)) ? pwmHiVal : pwmLowVal);
        while (__HAL_TIM_GET_FLAG(htim, TIM_FLAG_UPDATE) == RESET); // 等待周期完成
        __HAL_TIM_CLEAR_FLAG(htim, TIM_FLAG_UPDATE);
    }

		// 发送2个停止位
    for (int i = 0; i < 2 ; i++) {
        __HAL_TIM_SET_COMPARE(htim, channel, pwmLowVal);
        while (__HAL_TIM_GET_FLAG(htim, TIM_FLAG_UPDATE) == RESET); // 等待周期完成
        __HAL_TIM_CLEAR_FLAG(htim, TIM_FLAG_UPDATE);
    }

		DWTDelayNus(1);
		
		// 通过DMA操作将DShot指令发送出去，包括11位数据位、1位请求返回位、4个校验位、2个连续低电平帧间隔位共计18位
	  HAL_TIM_PWM_Stop(htim,channel);
}

// DShot电调上电解锁函数
void DShot_Unlock(uint8_t iChanel) 
{
    // 发送CMD 0信号，持续300ms
    Set_Motor_DShot(iChanel, 0);
    HAL_Delay(300);

    // 发送解锁信号CMD 1
    Set_Motor_DShot(iChanel, 1);
}

// 设置电调的DSHOT指令帧
void Set_Motor_DShot_bak(uint8_t iChanel, uint16_t value)
{
 // 1. 获取定时器配置（绕过HAL的启动延迟）
    SPwmTimeChannel *sChanelTimeInf = sGetChanelTimeInf(iChanel);
    TIM_TypeDef *TIMx = sChanelTimeInf->hTim->Instance;
//    uint32_t channel = sChanelTimeInf->TIM_CHANNEL_No;
    
    // 2. 直接配置寄存器（避免HAL延迟）
    TIMx->ARR = 26;                       // 1.67μs周期
    TIMx->CCR1 = 8;                       // 初始低电平
    TIMx->EGR |= TIM_EGR_UG;              // 强制更新寄存器
    TIMx->CR1 |= TIM_CR1_CEN;             // 启动定时器

    // 3. 构建18位帧数据
    uint32_t frameData = (value << 7) | (0 << 6); // [11数据+1请求]
    uint8_t checksum = (~(value ^ (value >> 4) ^ (value >> 8))) & 0x0F;
    frameData |= (checksum << 2);         // [4CRC] + [2位0]

    // 4. 发送18位（严格周期控制）
    for (int i = 17; i >= 0; i--) {
        TIMx->CCR1 = (frameData & (1 << i)) ? 18 : 8;
        while (!(TIMx->SR & TIM_SR_UIF)); // 硬等待周期结束
        TIMx->SR &= ~TIM_SR_UIF;          // 清除标志
    }

    // 5. 安全停止（等待最后一个周期完成）
    while (!(TIMx->SR & TIM_SR_UIF));     // 确保第18位完成
    TIMx->SR &= ~TIM_SR_UIF;
    TIMx->CR1 &= ~TIM_CR1_CEN;           // 停止定时器
    TIMx->CCR1 = 0;                       // 强制输出低电平
}

// 以下程序为采用DMA方式进行DShot电调的指令帧传输方式。以下程序未经调试20250420 by zengQingHua
//    DMA方式的最大问题是无法在cubeMx中预先定义DMA操作（因为用户可选择12个电调通道的任一个做DShot操作，
//    这样就需要12个DMA通道，H743全部才有2个DMA控制器，每个控制器也只有8个通道，全部一起才16通道），需要动态关联DMA。
//    以下程序思路应该正确，后续对于电调通道数较少情况可考虑使用，该方式高效、程序简洁。

// 假设 DShot 帧长度为 16 位（包含数据、请求返回位和校验和）
#define DSHOT_FRAME_LENGTH1 16
uint16_t g_iDShotCmdBuff[DSHOT_FRAME_LENGTH1 + 2]={0};

// 如果设置PWM通道的DMA为正常模式，而不是循环模式，则不需要设置DMA中断，并对中断完成操作做处理。
void vStopDShotDMA(uint8_t iChanel)
{
	  SPwmTimeChannel *sChanelTimeInf = sGetChanelTimeInf(iChanel);   // 获取指定通道iChanel的定时器通道句柄信息
		// 通过DMA操作将DShot指令发送出去，包括11位数据位、1位请求返回位、4个校验位、2个连续低电平帧间隔位共计18位
		HAL_TIM_PWM_Stop_DMA(sChanelTimeInf->hTim,sChanelTimeInf->TIM_CHANNEL_No);
}

void vSendDShotFrameByDMA(uint8_t iChanel)
{
	  SPwmTimeChannel *sChanelTimeInf = sGetChanelTimeInf(iChanel);   // 获取指定通道iChanel的定时器通道句柄信息
	  
		// 通过DMA操作将DShot指令发送出去，包括11位数据位、1位请求返回位、4个校验位、2个连续低电平帧间隔位共计18位
		HAL_TIM_PWM_Start_DMA(sChanelTimeInf->hTim,sChanelTimeInf->TIM_CHANNEL_No,(uint32_t *)g_iDShotCmdBuff,DSHOT_FRAME_LENGTH1+2);
}

// 设置电调的DSHOT指令帧
void Set_Motor_DShot_DMA(uint8_t iChanel, uint16_t value)
{
	  uint8_t requestReturn = 0;    // 不请求
    uint16_t frameData = (value & 0x7FF) | ((requestReturn & 0x1) << 11);
    uint8_t checksum = dshotChecksum(value,false);
    frameData |= (checksum << 12);

    // 获取定时器的自动重装载值（ARR）
    uint32_t arrValue = iGetTimerArr(iChanel);
	  uint16_t pwmHiVal,pwmLowVal;
	
    // 计算高电平占空比的比较值（70%），和低电平占空比的比较值（10%）
    pwmHiVal = arrValue * 70 / 100; pwmLowVal = arrValue * 10 / 100;

    for (int i = 0; i < DSHOT_FRAME_LENGTH1; i++) {
        if (frameData & (1 << i)) g_iDShotCmdBuff[i] = pwmHiVal;
        else  g_iDShotCmdBuff[i] = pwmLowVal;
    }
		
		// 通过DMA操作将DShot指令发送出去，包括11位数据位、1位请求返回位、4个校验位、2个连续低电平帧间隔位共计18位
		vSendDShotFrameByDMA(iChanel);
}
