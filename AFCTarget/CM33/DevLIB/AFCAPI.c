#include "AFCAPI.h"

#include "AFCGlobalVar.h"
#include "ACGCommonAPI.h"
#include "Sensor.h"
#include "tim.h"

// 获取AFC-5系统内部时钟
double getAFCTimerVal(void)
{
	 return g_sRealTimeCount.fcsTime/1000.f;   // 返回综控时间，单位为秒
}

// 返回AFC-5飞控板的ID号
double getUavId(void)
{
	 return (double)g_UavFcsParam.UavPara.UavId;   // 返回AFC-5飞控板的ID号
}

// AD采集通道全局变量数据
bool g_bUsedOfADSample = false;   // ADSample使用标志
void initADSample(void) 
{
	  g_bUsedOfADSample = true;
}

#define AD_SCALE_VAL  1.195   // 暂未使用
double getADSampleData(uint8_t iChannel)
{
	double fVal = 0.0;
	if(iChannel < 1)
		fVal = (double)g_sRealTimeCount.fcsTime;   // 传感器采集时的第一个数据为采集时刻的控制器本地时标值
	else
		fVal = ad_vol[iChannel-1]*3.3f/65535.0f;
	return fVal;
}

// 读取IMU传感器滤波处理后数据
// 获取MEMS惯组指定端口的参数。
//    端口0：   传感器采集时标
//    端口1--3：陀螺仪采集值
//    端口4--6：加计采集值
//    端口7：   温度采集值
double Sensor_Data_Of_ChNo(uint8_t chNo,uint8_t iChannel)
{
	double fVal = 0.0;
	switch(iChannel){
		case 0:
			fVal = (double)g_sRealTimeCount.fcsTime;   // 传感器采集时的第一个数据为采集时刻的控制器本地时标值
		  break;
		case 1:
			fVal = gyro_lpf[chNo].x;
		  break;
		case 2:
			fVal = gyro_lpf[chNo].y;
		  break;
		case 3:
			fVal = gyro_lpf[chNo].z;
		  break;
		case 4:
			fVal = accel_lpf[chNo].x;
		  break;
		case 5:
			fVal = accel_lpf[chNo].y;
		  break;
		case 6:
			fVal = accel_lpf[chNo].z;
		  break;
		case 7:
			fVal = temp_raw[chNo];
		  break;
		default:
			fVal = 0;
		  break;
	}
	return fVal;
}

// 读取IMU传感器原始数据
double Sensor_Data_Of_ChNo_Org(uint8_t chNo,uint8_t iChannel)
{
	double fVal = 0.0;
	if(iChannel < 1)
		fVal = (double)g_sRealTimeCount.fcsTime;   // 传感器采集时的第一个数据为采集时刻的控制器本地时标值
	else{
		if(iChannel < 4)
			 fVal = gyro_raw[chNo][iChannel-1] * GYRO_SCALE_2000;   // 读取陀螺仪测量值，输出单位为弧度(1、2、3-->陀螺仪)
		else{
			if(iChannel<7)		fVal = accel_raw[chNo][iChannel-4] * ACCEL_SCALE_8G;    // 读取加计测量值，转换为加速度单位，表明m/s2。目前1g-->4096  (4、5、6-->加速度计)
			else	  fVal = temp_raw[chNo];              // 读取温度值。【7 -- 温度】	
		}
	}
	return fVal;
}

// ICM42688接口模块
// 获取MEMS惯组ICM42688的角速度、加速度信息。
double getICM42688Data(uint8_t iChannel)
{
	return Sensor_Data_Of_ChNo(0,iChannel);
}

// 获取MEMS惯组ICM20602的角速度、加速度信息
double getICM20602Data(uint8_t iChannel)
{
	return Sensor_Data_Of_ChNo(1,iChannel);
}

// 获取数字压力传感器DSP310信息。
double getDPS310Data(uint8_t iCh)
{
	double fVal = 0;
	
	switch(iCh){
		case 0:
			fVal = (double)g_sRealTimeCount.fcsTime;   // 传感器采集时的第一个数据为采集时刻的控制器本地时标值
		  break;
		case 1:
			fVal = pressure * 100.f;     // pressure单位为hPa, 转化为Pa输出
		  break;
		case 2:
			fVal = temperature;    // 气压计输出温度单位为摄氏°
		  break;
		case 3:
			fVal = baro_altitude;   // baro_altitude单位为m
		  break;
		default:
			break;
	}

	return fVal;      // 0 -- 采集时刻飞控时标，1 -- 压力， 2 -- 温度,  3 -- 气压高度
}

void proDiffPressZeroOff(bool bCaliSts, float fSampleTim)
{
	  static bool bLastSts = false,bCaliDoing = false;
	  static uint32_t iCaliCounter = 0;
	  if((!bLastSts || bCaliDoing) && bCaliSts){
			 uint32_t iSampleNum = fSampleTim / gMS5525SampleTim;
			 if(iCaliCounter++ < iSampleNum) bCaliDoing = true;
			 else bCaliDoing = false;
			 
			 calDiffPressZeroOff(iCaliCounter,bCaliDoing);
			 
			 bLastSts = true;
		}
		if(!bCaliSts){
			bLastSts = bCaliDoing = false;
			iCaliCounter = 0;
		}
}

// 获取空速计MS5525信息。
double getMS5525Data(uint8_t iCh)
{
	double fVal = 0;
	
	switch(iCh){
		case 0:
			fVal = (double)g_sRealTimeCount.fcsTime;   // 传感器采集时的第一个数据为采集时刻的控制器本地时标值
		  break;
		case 1:
			fVal = diff_press_pa_raw;     // 差压值（全压 - 静压），单位为Pa
		  break;
		case 2:
			fVal = ms5525_temperature;    // 空速计输出温度单位为摄氏°
		  break;
		case 3:
			fVal = diff_press_vel;   // 指示空速IAS,单位为m/s
		  break;
		default:
			break;
	}

	return fVal;      // 0 -- 采集时刻飞控时标，1 -- 压力， 2 -- 温度,  3 -- 气压高度
}

double getIST8310Data(uint8_t iChannel)
{
	double fVal = 0.0,fYVal;

	switch(iChannel){
		case 0:
			fVal = (double)g_sRealTimeCount.fcsTime;   // 传感器采集时的第一个数据为采集时刻的控制器本地时标值
		  break;
		case 1: 
			fVal = g_fMagRaw[0];   // 输出原始数据，可供用户自己编程或标校使用
		  break;
		case 2:
			fVal = g_fMagRaw[1];   // 
		  break;
		case 3:
			fVal = g_fMagRaw[2];
		  break;
		case 4:
			if(g_bIst8310Calibing)	fVal = ist8310_angle;
			else{
				// V5.05.240910: 防止除0操作，另外半径不可能为负值。防止用户装订错误数据，引起潜在内存除0或异常操作计算风险
				  if(g_UavFcsParam.Ist8310Radius.x < 0.0001f)  g_UavFcsParam.Ist8310Radius.x = 0.0001f;
				  if(g_UavFcsParam.Ist8310Radius.y < 0.0001f)  g_UavFcsParam.Ist8310Radius.y = 0.0001f;
				 
				  fVal = (g_fMagRaw[0] - g_UavFcsParam.Ist8310Bias.x)/g_UavFcsParam.Ist8310Radius.x;   // V5.05.240816: 以前错误将mag_raw[0]*MAG_XY_SCALE用fVal，下同。
					fYVal = (g_fMagRaw[1] - g_UavFcsParam.Ist8310Bias.y)/g_UavFcsParam.Ist8310Radius.y;
				  if(fabs(fVal)>0.0001)  fVal = -atan2f(fYVal, fVal)*RAD_TO_DEG;
				  else	fVal = 90 * sign(fVal);
				  fVal = fVal * g_UavFcsParam.Ist8310Neg;  // V5.05.240817: 航向角一致性系数
				  ist8310_angle = fVal;
			}
		  break;
	}
	return fVal;
}

uint8_t iSBusCmdCount = 0;
void initSBusCmd(uint8_t iChannel)
{
	 static bool bFirst = true;
	 if(bFirst){
		  Sbus_Read_Start();
		  bFirst = false;
	 }
	 if((iChannel < NumOfSBusChannel) && !g_bSBusUsed[iChannel]){   // 为防止同一通道多次放置SBusCmd模块，置通道使用标志
		  g_bSBusUsed[iChannel] = true;
		  iSBusCmdCount++;
	 }
}

// AFC-3V1.05.211202: 从SBusCom读取变量到模型端口(ReadSBus模块)，供用户模型Simulink程序使用，返回值：
//       . ch3:  0 ~ 100  其它：-100 ~ 100。
extern bool g_bSBusInputed;
double readSBusCmd(uint8_t iChannel)
{
	  static uint8_t iCount = 0;
	  double fVal = 0.;
	  static uint16_t old_channel_raw[16] = {0};
	
	  if(g_bSBusInputed){
			old_channel_raw[iChannel] = channel_raw[iChannel];
		}
		fVal = (double)old_channel_raw[iChannel];
		
    if(iCount++ >= iSBusCmdCount){    // 是否所有通道的数据已读取到simulink模型变量中
			g_bSBusInputed = false;
			iCount = 0;
		}
		
	  return fVal;
}

int16_t Max_Throttle, iEscPWMWidthRange;  //最大油门, 电调PWM值调节范围
uint8_t giChNo[4], g_iEscType[MAX_SERV_CH_NUM];
uint32_t g_iMinPW[MAX_SERV_CH_NUM],g_iMaxPW[MAX_SERV_CH_NUM];     // 电调最小、最大脉冲宽度计数值
bool bEscMainSwitch = false;

void setupEscPara(uint8_t iChanel, double minPulseRate, double maxPulseRate)
{
	  if(!bEscMainSwitch){
			HAL_GPIO_WritePin(PWM_OE_GPIO_Port,PWM_OE_Pin,GPIO_PIN_SET);
			giChNo[0] = g_iM1CHANEL;  giChNo[1] = g_iM2CHANEL;  giChNo[2] = g_iM3CHANEL; giChNo[3] = g_iM4CHANEL;
			bEscMainSwitch = true;
		}
		
		if(g_iEscType[iChanel]==0){   // PWM电调，需要计算其脉宽值
			uint32_t pwmPeriodCounter = iGetTimerArr(iChanel);
			g_iMinPW[iChanel] = (uint32_t) (pwmPeriodCounter * minPulseRate/100.);
			g_iMaxPW[iChanel] = (uint32_t) (pwmPeriodCounter * maxPulseRate/100.);
		}
		else{  // DShot电调属于数字电调，直接赋值
			g_iMinPW[iChanel] = (uint32_t)minPulseRate;
			g_iMaxPW[iChanel] = (uint32_t)maxPulseRate;
		}
		
		setEscVal(iChanel,0.0);   // 单个电调解锁指令，有时可能需要调整该值
		
		// 以下为四旋翼飞行器下处理方式，后续可能需要改变
		Max_Throttle = g_iMaxPW[iChanel] - g_iMinPW[iChanel];  //最大油门, 任选一个电调通道
    iEscPWMWidthRange = Max_Throttle;      // 各个通道电调油门PWM波信号的宽度任意取一个通道作为调节范围，此处取油门
		
		HAL_Delay(500);
}

// AFC-3V1.05.211202: 初始化电调参数: 输入为通道号、最小开度对应PWM脉冲占空比、最大开度对应PWM脉冲占空比。
//                                    最小最大开度对应遥控器打到底时用示波器测量得到的脉冲信号
void initEscPara(double escFreq,uint8_t iChanel, double minPulseRate, double maxPulseRate)
{
	  g_iEscType[iChanel] = 0;
		updateTimerFreq(iChanel,128,escFreq,true);  // 初始化定时计数器并启动它。对于PWM电调，预分频值取128，则计时器输入频率1MHz
	  setupEscPara(iChanel,minPulseRate,maxPulseRate);
}

// AFC-3V1.05.211202: 调节电调
//   电调调节范围为openVal∈[0--100],通道号索引值从0开始
void setEscVal(uint8_t iChanel, double openVal)
{
	  uint32_t pwmVal;

// 电调PWM设置值的最小值为1000，最大值为2000，需要根据当前物理值openVal（物理值的最大最小值在g_fMaxPW、g_fMinPW中），计算出当前pwmVal
		if(openVal > 0.001)
			 pwmVal = (uint32_t)(g_iMinPW[iChanel] + (g_iMaxPW[iChanel] - g_iMinPW[iChanel]) * openVal/100);
		else
			 pwmVal = g_iMinPW[iChanel] + g_iSelfDetectPW;  // 当开度小于0.001%时，则将不产生PWM信号，该信号输出低电平。确定一个自检最小脉宽，HS用1000存在问题，主要是电调上电自检脉宽值的调整值。g_iMinPW一般为1000，有时需要稍微大一些

	 if(g_iEscType[iChanel] == 0)		Set_Motor_PWM(iChanel,pwmVal);   // 设置电机的PWM指令 
	 else Set_Motor_DShot(iChanel,pwmVal);   // 设置电机的DShot指令 
}

double fGetEscVal(uint8_t idxCh,uint16_t iMotorsPWM)
{
	  double fEscVal;
	  uint8_t  iChNo = giChNo[idxCh];
	 // 0 -- 100的开度对应g_iMinPW ~ g_iMaxPW脉宽
	  fEscVal = (iMotorsPWM - g_iMinPW[iChNo])* 100.0/(g_iMaxPW[iChNo] - g_iMinPW[iChNo]);   // V5.05.250424: *100更改为*100.0, 否则是整数计算，结果可能不准确。
	  return fEscVal;
}

// DShot 格式枚举
enum {
    DSHOT100, DSHOT150, DSHOT300,  DSHOT600, DSHOT1200,
	  NUM_PARAMS
};

double fGetDShotFreq(uint8_t iEscType)
{
	  // 定义各种典型DShotXXX协议格式的传输速率，单位为kbps
	  double fEscFreq[NUM_PARAMS] = {100,150,300,600,1200};
		return fEscFreq[iEscType - 1] * 1000;    // 工具箱中选择的DShot格式起始值为1,0表示为PWM信号。
}

void initDShotPara(uint8_t iEscType, uint8_t iChanel)
{
	  double escFreq = fGetDShotFreq(iEscType);    // 由于DShot格式中对占空比精度要求不高，此时各通道定时器仍然沿用PWM信号处理的定时器预分频值128，确保计时器输入信号频率为1MHz。
	  g_iEscType[iChanel] = iEscType;  // 该语句必须在initEscPara之后，确保电调类型码正确赋值
	
		updateTimerFreq(iChanel,8, escFreq,false);   // 只初始化定时计数器，不要启动。对于DShot电调，预分频值取8，则计时器输入频率16MHz。
	  setupEscPara(iChanel,0,2047);   // 调用该函数时，会将g_iEscType[iChanel]初始化为0，这是由于兼容性设计导致
	
	// DShot电调上电后需要连续发送3次0x000(全0帧)解锁
	  DShot_Unlock(iChanel);
}

// 舵系统采集通道
bool bPWMInSwitch = false;
void initPwmSample(uint8_t chNum)
{
	  if(!bPWMInSwitch){
			HAL_GPIO_WritePin(PWM_IE_GPIO_Port,PWM_IE_Pin,GPIO_PIN_SET);
			HAL_TIM_IC_Start_IT(&htim2,TIM_CHANNEL_1);   //开始捕获
			HAL_TIM_IC_Start_IT(&htim2,TIM_CHANNEL_2);
			__HAL_TIM_ENABLE_IT(&htim2,TIM_IT_UPDATE);
			bPWMInSwitch = true;
		}
}

#define PWM_IN_CALI_OFFSET 0.165
double getPwmVal(uint8_t ch)
{
	double fVal = 0.;
 // PWM采集值pwm_in_val的最小值为1000，最大值为2000，需要根据当前物理值最大（-100）最小值（100），计算出当前物理值fVal
//	fVal = pwm_in_val[ch]*(100./CUR_PWM_PERIOD_COUNT);
	fVal = pwm_in_val[ch]*(100./CUR_PWM_PERIOD_COUNT) + PWM_IN_CALI_OFFSET;
	return fVal;
}

// 初始化舵系统采集通道
void initSrvoIn(uint8_t chNum)
{
	
}
// 获取给定指定通道的角度值
double getSrvoVal(uint8_t ch)
{
	 return ch;
}

// 初始化舵系统输出通道
void initSrvoOut(uint8_t chNum)
{
}

// 设置给定指定通道的角度值
void setSrvoVal(uint8_t ch, double fVal)
{
	
}

void reSetTimerPeriod(TIM_HandleTypeDef *htim, uint8_t iTaskPeriod)
{
	if(iTaskPeriod > 0){
// 1. 先停止定时器和中断（避免修改参数时计数器紊乱）
		HAL_TIM_Base_Stop_IT(htim);

// 2. 配置定时器时基参数（保持你的原逻辑，补充关键配置）		
		htim->Init.Prescaler = 512-1;
		htim->Init.CounterMode = TIM_COUNTERMODE_UP;
		uint32_t iPeriod = iTaskPeriod * 250;
		htim->Init.Period = iPeriod;
		htim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;	

// 3. 重新初始化定时器（此时已停止，初始化安全）
		if (HAL_TIM_Base_Init(htim) != HAL_OK)
		{
				Error_Handler(); // 增加错误处理，避免配置失败无反馈
		}

// 4. 清除可能残留的中断标志位（避免重启后误触发）
		__HAL_TIM_CLEAR_IT(htim, TIM_IT_UPDATE);
		
// 5. 打开当前定时器中断
		HAL_TIM_Base_Start_IT(htim);
	}
	else{
		// 关闭当前定时器中断
		HAL_TIM_Base_Stop_IT(htim);
	  // 清除中断标志位（避免停止后残留标志导致后续异常）
		__HAL_TIM_CLEAR_IT(htim, TIM_IT_UPDATE);
	}
}

void initTaskScheduler(uint8_t iTask01Period,uint8_t iTask02Period,uint8_t iTask03Period)
{
 // 注意：TIM6是基础定时器（仅更新中断，无输出通道），配置逻辑与TIM12/13/14一致
	// 更新TASK01(TIM13)的任务周期
	reSetTimerPeriod(&htim13,iTask01Period);
	// 更新TASK02(TIM14)的任务周期
	reSetTimerPeriod(&htim14,iTask02Period);
	// 更新TASK03(TIM6)的任务周期
	reSetTimerPeriod(&htim6,iTask03Period);
}

void initTask0Period(void)
{
	reSetTimerPeriod(&htim12,g_iSimulinkAlgorithmStep);
}

/*   End of AFCAPI.c    */
