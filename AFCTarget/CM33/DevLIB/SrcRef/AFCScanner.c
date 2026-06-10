/******************** (C) COPYRIGHT 2019 ACE Tech Co.*************************
 * 作者	  ： 金长征，曾庆华
 * 文件名 ： AFCScanner.c
 * 版  本 ： 
 *    V1.01.250912 -- 
 *       (1) 压力扫描阀等效模拟器
 *    V1.02.250914 --
 *       (1) 增加ADG732BSUZ的片选、使能和地址锁存信号控制
 *    V1.03.250915 --
 *       (1) 完善温度扫描阀功能实现
 *    V1.04.250915 --
 *       (1) 将所有HAL_Delay(1)更换为delay_us(100)
 *    V1.05.251215 -- 金长征更改撰写驱动程序
 *       (1)  由于PC3_C控制不稳定，且模拟量采集时精度不高，故变更模拟开关为ADG408，
 *            通道使能引脚改为普通引脚，
 *      （2）串口数据量大，原串口达不到要求，更改为U5/422通信。
 * 
 * 描述   ：AFCPressScan接口底层函数
 * 官网   ：www.acecreator.com
 * 淘宝   ：acecreator.taobao.com
 * 公众号 ：无人飞行控制
 *
*****************************************************************************/
	
#include "AFCScanner.h"
#include "adc.h"
#include "main.h"

// #include "AFCGlobalVar.h"
// #include "ACGCommonAPI.h"


//三个采集通道组，每组16通道，每通道正负引脚各连接至模拟开关，共12个模拟开关
//压力组1，U1U3连接信号负引脚，U2U4连接信号正引脚
//压力组2，U8U10连接信号负引脚，U9U11连接信号正引脚
//温度组，U15U17连接信号负引脚，U16U18连接信号正引脚
//EN1使能各组通道0-7，EN2使能各组通道8-15，高电平有效,EN1/GPO4/PE10, EN2/GPO5/PB2
//A2A1A0值对应通道号0-7,A0/GPO1/PD11,A1/GPO2/PE15,A2/GPO3/PE12

//使用H743的adc外设进行模拟量转换
//模拟量采集引脚：压力组1：PC0/ADC1_INP10，压力组2：PC1/ADC2_INP11，温度组：PC3/ADC3_INP1,分辨率16位

#define PressScannerChannelNum        32
#define TempratureScannerChannelNum   12  // 16通道，匹配ADG506AKRZ-REEL

//定义各地址引脚
#define A0_PORT GPIOD
#define A1_PORT GPIOE
#define A2_PORT GPIOE
#define A0_PIN GPIO_PIN_11
#define A1_PIN GPIO_PIN_15
#define A2_PIN GPIO_PIN_12
//定义模拟开关使能引脚
#define EN1_PORT GPIOE
#define EN2_PORT GPIOB
#define EN1_PIN GPIO_PIN_10
#define EN2_PIN GPIO_PIN_2


extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern ADC_HandleTypeDef hadc3;

// 全局变量数据
bool g_bUsedOfPressScanner;    // 压力扫描阀使用标志
bool g_bUsedOfTempScanner;     // 温度扫描阀使用标志
uint16_t g_iPressScanner[PressScannerChannelNum];
uint16_t g_iTempratureScanner[TempratureScannerChannelNum];

#include "ACGCommonAPI.h"       // 若调用AFC系统共用函数delay_us时，需要注释掉下面的delay_us函数
////延时us函数
//void delay_us(uint32_t us)
//{
//    // 需要根据CPU频率校准
//    uint32_t loop_count = us * (SystemCoreClock / 2048000);  // 需要实际测试调整
//    
//    while(loop_count--);
//    
//}

//关闭扫描器
void disableChannelSwitch(void)
{
    // 拉低扫描器CS引脚
    HAL_GPIO_WritePin(EN1_PORT, EN1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(EN2_PORT, EN2_PIN, GPIO_PIN_RESET);
}
//启用扫描器
void enableChannelSwitch(uint8_t channel)
{
    // 先禁用通道开关，防止地址线冲突（拉低）
	disableChannelSwitch();
    // 启用芯片，拉高使能引脚,低地址使能EN1，高地址使能EN2
    if((channel & 0x08) == 0){
        HAL_GPIO_WritePin(EN1_PORT, EN1_PIN, GPIO_PIN_SET);
    }else{
        HAL_GPIO_WritePin(EN2_PORT, EN2_PIN, GPIO_PIN_SET);
    }
    delay_us(2);  // 延时2us
}

//选择通道地址
void selectAddress(uint8_t channel)
{
    //确保地址在0-7
    uint8_t addr = channel % 8;

    // 设置A0-A2引脚状态
    HAL_GPIO_WritePin(A0_PORT, A0_PIN, (addr & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(A1_PORT, A1_PIN, (addr & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(A2_PORT, A2_PIN, (addr & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);

}

//初始化控制与通道GPIO
static void initAllControlPins(void)
{
    //EN1使能各组通道0-7，EN2使能各组通道8-15，高电平有效,EN1/GPO4/PE10, EN2/GPO5/PB2
    //A2A1A0值对应通道号0-7,A0/GPO1/PD11,A1/GPO2/PE15,A2/GPO3/PE12

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    //使能时钟
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // 配置共用地址线A0-A2,初始化为低电平
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    // 配置A0 (PD11)
    GPIO_InitStruct.Pin = A0_PIN;
    HAL_GPIO_Init(A0_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(A0_PORT, A0_PIN, GPIO_PIN_RESET);
    
    // 配置A1 (PE15)
    GPIO_InitStruct.Pin = A1_PIN;
    HAL_GPIO_Init(A1_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(A1_PORT, A1_PIN, GPIO_PIN_RESET);
    
    // 配置A2 (PE12)
    GPIO_InitStruct.Pin = A2_PIN;
    HAL_GPIO_Init(A2_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(A2_PORT, A2_PIN, GPIO_PIN_RESET);

    // 配置模拟开关使能引脚EN1(PE10)，EN2(PB2) - 初始为低电平
    GPIO_InitStruct.Pin = EN1_PIN;
    HAL_GPIO_Init(EN1_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(EN1_PORT, EN1_PIN, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin = EN2_PIN;
    HAL_GPIO_Init(EN2_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(EN2_PORT, EN2_PIN, GPIO_PIN_RESET);

}

//选择压力通道
void selectPressChannel(uint8_t channel)
{
    if (channel >= PressScannerChannelNum) return;
    
    // 先禁用芯片再更改地址，避免中间状态
    disableChannelSwitch();
    //选择通道地址
    selectAddress(channel);
    // 打开通道开关
    enableChannelSwitch(channel);
    
    // 延时等待通道稳定
    delay_us(50);  // 延时50us

}

//选择温度通道
void selectTemperatureChannel(uint8_t channel)
{
    if (channel >= TempratureScannerChannelNum) return;

    // 先禁用芯片再更改地址，避免中间状态
    disableChannelSwitch();
    //选择通道地址
    selectAddress(channel);
    // 打开通道开关
    enableChannelSwitch(channel);
    
    // 延时等待通道稳定
    delay_us(50);  // 延时50us

}

//采样通道高阻抗，需延长采样时间
//重新初始化adc时钟与采样时间，修改PLL2P分频系数为2，采样时间为64.5+12.5个周期
void initADCConfig(void)
{
    // 停止ADC
    HAL_ADC_Stop(&hadc1);
    HAL_ADC_Stop(&hadc2);
    HAL_ADC_Stop(&hadc3);

    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    ADC_ChannelConfTypeDef sConfig = {0};
    
    // 获取当前配置
    HAL_RCCEx_GetPeriphCLKConfig(&PeriphClkInitStruct);
    
    // 只修改ADC相关的PLL2P分频，不影响PLL2Q、PLL2R等其他分频
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL2;
    PeriphClkInitStruct.PLL2.PLL2P = 2;  // 只修改P分频，从1改为2
    
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    // 重新初始化ADC
    HAL_ADC_DeInit(&hadc1);
    MX_ADC1_Init();  
    HAL_ADC_DeInit(&hadc2);
    MX_ADC2_Init();
    HAL_ADC_DeInit(&hadc3);
    MX_ADC3_Init();

    // 配置采样时间
    sConfig.Channel = ADC_CHANNEL_10;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_64CYCLES_5;
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK )
    {
        Error_Handler();
			
    }
		sConfig.Channel = ADC_CHANNEL_11;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_64CYCLES_5;
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
		if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }
		sConfig.Channel = ADC_CHANNEL_1;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_64CYCLES_5;
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
		if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }
    // 重新启动ADC
    HAL_ADC_Start(&hadc1);
    HAL_ADC_Start(&hadc2);
    HAL_ADC_Start(&hadc3);
    
    
}


//以阻塞方式
//读取ADC转换结果，分辨率16位
uint16_t readADCValue(ADC_HandleTypeDef* hadc)
{
		uint16_t val=0;
    // 启动ADC转换
    if (HAL_ADC_Start(hadc) != HAL_OK)
    {
        Error_Handler();
    }
    
    // 等待转换完成
    if (HAL_ADC_PollForConversion(hadc, 100) != HAL_OK)
    {
        Error_Handler();
    }
    
		val = HAL_ADC_GetValue(hadc);
    
		// 停止 ADC 转换
		HAL_ADC_Stop(hadc);
		
    return val;
}


void initPressScan(void) 
{
    // 初始化所有控制引脚
    initAllControlPins();
    
    // 初始化时禁用扫描器,关闭通道使能引脚
    disableChannelSwitch();

    //重新配置ADC
    initADCConfig();
    
    // 标记压力扫描器为已使用
    g_bUsedOfPressScanner = true;
    
    // 初始化压力数据数组
    for (uint8_t i = 0; i < PressScannerChannelNum; i++)
    {
        g_iPressScanner[i] = 0;
    }
}

void ReadAllPressData(void)
{
    if (!g_bUsedOfPressScanner) return;
    
    // 依次读取32个通道的压力数据
    for (uint8_t channel = 0; channel < PressScannerChannelNum; channel++)
    {
        // 选择当前通道
        selectPressChannel(channel);
        
        // 读取ADC值
        uint16_t adcValue = readADCValue( (channel < 16) ? (&hadc1) : (&hadc2) );
        
        // 存储原始ADC值
        g_iPressScanner[channel] = adcValue;
    }
    
    // 所有通道读取完成后关闭通道
    disableChannelSwitch();
}

double getPressData(uint8_t iChannel)
{
    double fVal = 0.0;
    if(iChannel == PressScannerChannelNum)
        // fVal = (double)g_sRealTimeCount.fcsTime;   // 传感器采集时的第一个数据为采集时刻的控制器本地时标值
        fVal = 1.1;//无时标函数，用常量代替
    else if (iChannel < PressScannerChannelNum)
        fVal = g_iPressScanner[iChannel] * 5.0f / 65535.0f;      // 转换为电压值
    
    return fVal;
}


void initTempratureScan(void) 
{
    // 确保控制引脚已初始化
    initAllControlPins();
    
    // 初始化时禁用扫描器
    disableChannelSwitch();

    //重新配置ADC
    initADCConfig();
    
    // 标记温度扫描器为已使用
    g_bUsedOfTempScanner = true;
    
    // 初始化温度数据数组
    for (uint8_t i = 0; i < TempratureScannerChannelNum; i++)
    {
        g_iTempratureScanner[i] = 0;
    }
}

void ReadAllTemperatureData(void)
{
    if (!g_bUsedOfTempScanner) return;
    
    // 依次读取16个通道的温度数据
    for (uint8_t channel = 0; channel < TempratureScannerChannelNum; channel++)
    {
        // 选择当前通道
        selectTemperatureChannel(channel);
        
        // 存储原始ADC值
        g_iTempratureScanner[channel] = readADCValue(&hadc3);
    }
    
    // 所有通道读取完成后关闭通道
    disableChannelSwitch();
}

double getTempratureData(uint8_t iChannel)
{
	double fVal = 0.0;
  if(iChannel == TempratureScannerChannelNum)
        // fVal = (double)g_sRealTimeCount.fcsTime;   // 传感器采集时的第一个数据为采集时刻的控制器本地时标值
        fVal = 1.1;//无时标函数，用常量代替
	else if (iChannel < TempratureScannerChannelNum)
        fVal = g_iTempratureScanner[iChannel] * 5.0f / 65535.0f;      // 转换为电压值
	return fVal;
}
/************************ (C) COPYRIGHT ACE Co. about ICM42688 by ZengQinghua *****END OF FILE****/
