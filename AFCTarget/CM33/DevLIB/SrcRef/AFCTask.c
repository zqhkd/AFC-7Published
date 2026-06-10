/******************** (C) COPYRIGHT 2019 ACE Tech Co.*************************
 * 作  者 ： 曾庆华
 * 文件名 ： AFCTask.c
 * 版  本 ： 
 *          V5.01.240622 
 * 描  述 ：AFC-5任务管理模块
 * 官  网 ：www.acecreator.com
 * 淘  宝 ：acecreator.taobao.com
 * 公众号 ：无人飞行控制
*****************************************************************************/
#define __BASCIAC_USED__
#define __MODULEAC_USED__

#define BreakBoundsVal -1000

#include "string.h"
#include "GPIO.h"
#include "usart.h"
#include "Tim.h"
#include <stdio.h>
#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_rcc_ex.h"

#include "AFCTask.h"
#include "ICM42688.h"
#include "BMI088.h"
#include "ICM20602.h"
#include "DPS310.h"
#include "IST8310.h"
#include "MS5525.h"
#include "ADSample.h"
#include "SbusRC.h"
#include "AFCDTCom.h"
#include "AFCSimuCom.h"
#include "AFCGpsCom.h"
#include "SimuAlgorithm.h"
#include "FM25V01.h"
#include "W25QXX.h"
#include "AFCXXXCom.h"
#include "BSP5500.h"
#include "AFCScanner.h"

#include "ACGCommonAPI.h"

#include "AFCAPI.h"
#include "AFCTestFun.h"

#include "ahrs.h"
//#include "atti_control.h"
//#include "alt_control.h"

#include "SimMode.h"
//#undef __CONNECTED_IO__

#ifdef  __CONNECTED_IO__
#include "ConnectedIO_Proc.h"
#endif

uint8_t idxCurFrame;

// SDCard操作的虚函数，可外部重载
__weak void ModelVar2USBDisk(void)
{
}
// V5.04.240717: AFC-5控制器FRam保存参数处理
//#define __LOCAL_DEBUG_VER__
SSaveParamFlash debugDefaultPara(void)
{
		SSaveParamFlash pSUavDefaultPara;
	
	  pSUavDefaultPara.FirstInitFlg = 0xA5DA;
	
	// UAV总体配置参数
		pSUavDefaultPara.UavPara.UavId = 25201;
		pSUavDefaultPara.UavPara.UavFrame = K80B_UAV;
		pSUavDefaultPara.UavPara.FcsBoard = AFC6_BOARD;
		pSUavDefaultPara.UavPara.Remoter = WFLY_ETS6S;
		pSUavDefaultPara.UavPara.FlyMode = ALTITUDE_HOLD_MODE;
	
	  pSUavDefaultPara.UavPara.AuthorizationCode = 0x2CAD;

	// UAV飞控板相关传感器标校参数
	  pSUavDefaultPara.Icm42688.AccScale.x  = pSUavDefaultPara.Icm42688.AccScale.y  = pSUavDefaultPara.Icm42688.AccScale.z  = 1;
	  pSUavDefaultPara.Icm42688.AccOffset.x = pSUavDefaultPara.Icm42688.AccOffset.y = pSUavDefaultPara.Icm42688.AccOffset.z = 0;
	  pSUavDefaultPara.Icm42688.GyroScale.x  = pSUavDefaultPara.Icm42688.GyroScale.y  = pSUavDefaultPara.Icm42688.GyroScale.z  = 1;
	  pSUavDefaultPara.Icm42688.GyroOffset.x = pSUavDefaultPara.Icm42688.GyroOffset.y = pSUavDefaultPara.Icm42688.GyroOffset.z = 0;

	  pSUavDefaultPara.Icm20602.AccScale.x  = pSUavDefaultPara.Icm20602.AccScale.y  = pSUavDefaultPara.Icm20602.AccScale.z  = 1;
	  pSUavDefaultPara.Icm20602.AccOffset.x = pSUavDefaultPara.Icm20602.AccOffset.y = pSUavDefaultPara.Icm20602.AccOffset.z = 0;
	  pSUavDefaultPara.Icm20602.GyroScale.x  = pSUavDefaultPara.Icm20602.GyroScale.y  = pSUavDefaultPara.Icm20602.GyroScale.z  = 1;
	  pSUavDefaultPara.Icm20602.GyroOffset.x = pSUavDefaultPara.Icm20602.GyroOffset.y = pSUavDefaultPara.Icm20602.GyroOffset.z = 0;

	  pSUavDefaultPara.Dps310Offset = 0;
	  
		pSUavDefaultPara.Ist8310Bias.x = pSUavDefaultPara.Ist8310Bias.y = pSUavDefaultPara.Ist8310Bias.z = 0;
		
		pSUavDefaultPara.Ist8310Radius.x = pSUavDefaultPara.Ist8310Radius.y = pSUavDefaultPara.Ist8310Radius.z = 1;
		
		g_RollSensor0 = g_PitchSensor0 = 0;
		if(pSUavDefaultPara.UavPara.FcsBoard==K80B_UAV){
			 pSUavDefaultPara.Ist8310Bias.x = -0.88;
			 pSUavDefaultPara.Ist8310Bias.y =3.38;
			pSUavDefaultPara.Ist8310Neg= -1;
			
		}
		
 	  return pSUavDefaultPara;
}

void initAFCUavPara(void)
{
#ifdef __LOCAL_DEBUG_VER__
	  g_UavFcsParam = debugDefaultPara();
//		g_UavFcsParam = loadDefaultPara();  // 有效芯片但参数无效：加载默认值
//		g_UavFcsParam.UavPara.UavId = 25201;	
//		SaveToFM25V01(&g_UavFcsParam);	
#else	
	  bool bCrcOk = LoadFromFM25V01(&g_UavFcsParam);
    // 核心：一行判断结构体是否全0（利用memcmp对比全0缓冲区）
		if (!g_bUsedOfFM25V01){          // 电路板上没有配置FM25V01芯片：加载默认值
			  g_UavFcsParam = loadDefaultPara();  
				g_UavFcsParam.FirstInitFlg = 0x5AA5;  // 全0（无FM25V01芯片）：只设标志
		}
		else if ((!bCrcOk) || g_UavFcsParam.FirstInitFlg != 0xA5DA){
				g_UavFcsParam = loadDefaultPara();  // 有效芯片但参数无效：加载默认值
				SaveToFM25V01(&g_UavFcsParam);
		}
#endif
//		TProductConfig cfg;
//		memset(&cfg, 0, sizeof(cfg));  // 先清零所有成员（避免随机值）
//		cfg.ProductId = 25501;            // 对应0x00 00 62 d5
//		strcpy(cfg.ProductName,"AFC-6");   // 内存：41 46 43 2D 36 00 00 00 ...（补0）
//		cfg.BoardType = AFC6_BOARD;       // 对应0x0003
//		cfg.ProductAuthCode = 342890;    // 对应0x00 05 3b 6a
//		cfg.iRes1 = 0x9a8b7c5d;
//		cfg.UserAuthCode = 11145;        // 对应0x2b 89
//		cfg.iRes2 = 0x1a2b3c4d;
//		cfg.KeyCode = 7601;            // 对应0x1d b1  十进制7601
//		cfg.ProductCrc = 0x1234;
//		FlashWrite(&cfg);

		// 缺省为正装，AFC-5飞控组件为前(+X)右(+Y)下(+Z)定义，该标志表明是否反向。
	 g_UavFcsParam.UavPara.XSetup = false;			g_UavFcsParam.UavPara.YSetup = false;			g_UavFcsParam.UavPara.ZSetup = false;
	 
	 if(g_UavFcsParam.UavPara.UavFrame==K80A_UAV){
		  // K80A板为反装状态，前(+X)左(+Y)上(+Z)定义
			g_UavFcsParam.UavPara.XSetup = false;			g_UavFcsParam.UavPara.YSetup = true;			g_UavFcsParam.UavPara.ZSetup = true;
		 
			//   AFC-5A通道号定义
			g_iM1CHANEL = 1 - 1;
			g_iM2CHANEL = 9 - 1;
			g_iM3CHANEL = 7 - 1;     // 前面为8，疑似错误
			g_iM4CHANEL = 2 - 1;
	 }
	 
//	 g_UavFcsParam.UavPara.UavFrame = K80B_UAV;         // 后续用地面站设置本机为K80B_UAV后，需要把本行删除。20250522
	 
	 if(g_UavFcsParam.UavPara.UavFrame==K80B_UAV){
		 
		 // 为保持软件兼容性，临时将原来K80B_UAV作为ZF02号来用，需要强行指定电路板
		 g_UavFcsParam.UavPara.FcsBoard = AFC6_BOARD;
		  // K80B板为正装状态，前(+X)右(+Y)下(+Z)定义
	   g_UavFcsParam.UavPara.XSetup = true;			g_UavFcsParam.UavPara.YSetup = true;			g_UavFcsParam.UavPara.ZSetup = false;
		 
	   g_iSelfDetectPW = -10;   // 电调自检时最小脉宽调整值
		 //   ZF02通道号定义
			g_iM1CHANEL = 4 - 1;
			g_iM2CHANEL = 5 - 1;
			g_iM3CHANEL = 6 - 1;     
			g_iM4CHANEL = 3 - 1;
//		 	g_iM1CHANEL = 6 - 1;
//			g_iM2CHANEL = 3 - 1;
//			g_iM3CHANEL = 4 - 1;     
//			g_iM4CHANEL = 5 - 1;
	 }	 
	 
	 if(g_UavFcsParam.UavPara.UavFrame==F550_UAV){
		  // F550板为正装状态，前(+X)右(+Y)下(+Z)定义
		 
			//   AFC-5A通道号定义
				g_iM1CHANEL = 1 - 1;    // 机头在前，右上电机，反浆逆时针转
				g_iM2CHANEL = 9 - 1;    // 机头在前，左下电机，反浆逆时针转
				g_iM3CHANEL = 8 - 1;    // 机头在前，左上电机，正浆顺时针转？？？
				g_iM4CHANEL = 2 - 1;    // 机头在前，右下电机，正浆顺时针转
	 }
	 
	 if(g_UavFcsParam.UavPara.UavFrame==HS620_UAV){
		  // HS620板为前后反转状态，后(+X)左(+Y)下(+Z)定义
			g_UavFcsParam.UavPara.XSetup = true;			  g_UavFcsParam.UavPara.YSetup = true;			g_UavFcsParam.UavPara.ZSetup = false;
		 
	 	//  对于新制AFC板一定要充分重视此处检查，否则会造成无人机
	//  即使解锁也无法控制对应电机的现象。明确四旋翼无人机通道定义
		//   AFC-5A通道号定义
		  g_iM1CHANEL = 1 - 1;
			g_iM2CHANEL = 9 - 1;
			g_iM3CHANEL = 7 - 1;
      g_iM4CHANEL = 2 - 1;
	 }	 
 
	 // IMU滤波器频带的缺省参数如下：
	 g_GryoFilter.x = 50;  g_GryoFilter.y = 50;  g_GryoFilter.z = 20;  
	 g_AccFilter.x  = 20;  g_AccFilter.y  = 20;  g_AccFilter.z = 20;

   // HS620机架的陀螺、加计频带明显要小一些	 
	 if(g_UavFcsParam.UavPara.UavFrame==HS620_UAV){
//			 g_GryoFilter.x = 20;  g_GryoFilter.y = 20;  g_GryoFilter.z = 10;  
//			 g_AccFilter.x  = 10;  g_AccFilter.y  = 10;  g_AccFilter.z = 10;
			 g_GryoFilter.x = 30;  g_GryoFilter.y = 30;  g_GryoFilter.z = 15;  
			 g_AccFilter.x  = 15;  g_AccFilter.y  = 15;  g_AccFilter.z = 15;
	 }	 
	 
		// 各组件均工作为非校准状态
	 g_bICM42688Calibing = g_bICM20602Calibing = g_bDPS310Calibing = g_bIst8310Calibing = false;  //
}

void vInitRealTaskParameter(void)
{
	g_sRealTimeCount.check_flag = 0;
	g_sRealTimeCount.err_flag = 0;

	g_sRealTimeCount.ctlStep  = 0;
	g_sRealTimeCount.testStep = 0;
	g_sRealTimeCount.sampStep  = 0;

	g_sRealTimeCount.relTime2PPS = 0;
	g_sRealTimeCount.relTime  = 0;

	g_sRealTimeCount.cnt_OutTime = 0;
	g_sRealTimeCount.fcsTime  = 0;
}

void TaskRealTimeCount(void)
{
	g_sRealTimeCount.ctlStep++;
	g_sRealTimeCount.testStep++;
	g_sRealTimeCount.sampStep++;

	g_sRealTimeCount.relTime2PPS++;
	g_sRealTimeCount.relTime++;                 
	g_sRealTimeCount.cnt_OutTime++;          //u16, 最大超时时间65535ms
	g_sRealTimeCount.fcsTime++;             // 综控对准时标复位后在本机计时

	if(g_sRealTimeCount.check_flag >= g_iSimulinkAlgorithmStep)
	{
		g_sRealTimeCount.err_flag ++;     //每累加一次，证明代码在预定周期g_iSimulinkAlgorithmStep ms内没有跑完。
	}
	else
	{	
		g_sRealTimeCount.check_flag += 1;	//该标志位在循环的最后被清零
	}
}

void BasicDriverInit(void)
{
	HAL_Delay(200);
	HAL_GPIO_WritePin(PWM_OE_GPIO_Port,PWM_OE_Pin,GPIO_PIN_SET);
	HAL_GPIO_WritePin(PWM_IE_GPIO_Port,PWM_IE_Pin,GPIO_PIN_SET);
	
	initIST8310();
	HAL_Delay(10);
	initDPS310();
	HAL_Delay(10);
	initICM20602();
	HAL_Delay(10);
	initICM42688();
	
	Sbus_Read_Start();
}

void vSysParamInit(void)
{
	uint8_t i;
	bInitDtComIn = false;
	bInitGpsComIn = false;
	
	bInitSBusComIn = false;
	g_bUsedOfAFCBasicAC = false;  // 缺少为不使用BasicAC模块库
	g_iProcessRunNo = 0;
	g_iWriteSDFailedCnt = 0;
	
	// 先假定所有遥控器通道都是未使用的，初始化该通道时将其置true
	for(i=0;i<NumOfSBusChannel;i++) g_bSBusUsed[i] = false;
	
	// V5.05.240815: g_bTestCalibOpening用于用户测试时，对于用户模型未使用设备做临时打开标志，测试或校准完成后再将其关闭，确保不影响用户模型程序。
	for(i=0;i<SENSOR_NUM;i++)   g_bTestCalibOpening[i] = false;
	
// 将数传串口输入/输出信号数量全部清零
	ClrDtComSignalNum();
	
	// 外扩串口初始化处理
	initXComPara();
	
}

void runResult2PC(double dcmAng[3],double fPositionZ)
{
	   // 为connectPC程序准备姿态角解算数据
	  Vector3i attiAngDegX100 = getAttiAngRad2DegX100(dcmAng);
		roll_sensor = attiAngDegX100.x;  pitch_sensor = attiAngDegX100.y; yaw_sensor = attiAngDegX100.z;
	//  为connectPC程序准备高度数据
		gfPositionZ = fPositionZ;
}

uint32_t overTimeTmp[120] = {0};
void testTickUs(void)
{
  for(uint8_t i = 0; i < 110; i++){	
		uint32_t t0, t1;
		uint32_t preMs = HAL_GetTick();
		
		// 等待即将发生毫秒溢出（SysTick接近0）
		while (SysTick->VAL > 1000);  // 假设SysTick时钟为1MHz，剩余1000计数≈1us
		
		// 在溢出前后进行测量
		t0 = HAL_GetTick_us();
		HAL_Delay(1);  // 确保跨越1ms边界
		t1 = HAL_GetTick_us();
		overTimeTmp[i] = t1 - t0;
	}	
}

#define FRAME_OVER_TIME 500  // 100us
void vOverTimePro(void)
{
	// 进行XCom串口帧接收超时处理
//    testTickUs();
	  vXComFrameOverTimePro(FRAME_OVER_TIME);
//	  HAL_Delay(5);
}

bool bSPI1IsUsing = false;
bool bSPI4IsUsing = false;
bool bI2C1IsUsing = false;

extern bool g_bSBusInputed;    // sbus有数据输入时才刷新数据
bool g_bWriteUSBDiskFlg = false;
//  该任务必须在最小仿真步长g_iSimulinkAlgorithmStep内执行完毕，否则会置错误标志erro_flag
void 	Task00Isr(void)
{
	   static uint8_t iLastRunMode = 0;
//		if((bInitSBusComIn) && (g_bSBusInputed)){
//				Sbus_ReadData();
//	//	  get_rc_channel_value();
//			  g_bSBusInputed = false;
//		}
		// 采集惯组数据
		if((g_bUsedOfICM42688) && (!bSPI1IsUsing)){
			bSPI1IsUsing = true;
			ICM42688_ReadData();                
			bSPI1IsUsing = false;
		}
		if(g_bUsedOfICM20602 && (!bSPI4IsUsing)){
			bSPI4IsUsing = true;
			ICM20602_ReadData();
			bSPI4IsUsing = false;
		}
		
		if(g_bUsedOfUSBDisk){
			g_bWriteUSBDiskFlg = true;
		}
			
		// 控制算法程序
		StepAlgorithmModel();
		
		// 组件测试或标定功能启动后，采集组件数据或组件原始数据从DTComOut输出到地面站的数据
		if((g_iCurRunMode==1)||(g_iCurRunMode==2)) CompTest2Gs();
		
		if((iLastRunMode > 0) && (g_iCurRunMode==0)) vCloseTestCalibUsedFlg();
		
		iLastRunMode = g_iCurRunMode;
}

#ifdef __CONNECTED_IO__
void CioDeviceSampleTask(void)
{
		if(g_sRealTimeCount.sampStep%2 == 1){         // 2ms采集一次
//			if(bInitSBusComIn){             //  V
//				Sbus_ReadData();
//			}
			// 采集惯组数据
			if((g_bUsedOfICM42688) && (!bSPI1IsUsing)){
				bSPI1IsUsing = true;
				ICM42688_ReadData();                
				bSPI1IsUsing = false;
			}
			if(g_bUsedOfICM20602 && (!bSPI4IsUsing)){
				bSPI4IsUsing = true;
				ICM20602_ReadData();
				bSPI4IsUsing = false;
			}
			
			if(g_bUsedOfMs5525 && (!bI2C1IsUsing)){
				bI2C1IsUsing = true;
				MS5525_ReadData();
				bI2C1IsUsing = false;
			}
		}
	
		if(g_sRealTimeCount.sampStep%4 == 1){         // 4ms采集一次
			if(g_bUsedOfADSample) ADSample_ReadData();
		}
		
		if(g_sRealTimeCount.sampStep%20 == 1){         // 20ms采集一次
			if(g_bUsedOfDPS310)  DPS310_ReadData();    // 采集气压计数据
		}
		
		if(g_sRealTimeCount.sampStep%20 == 11){         // 20ms采集一次
			if(g_bUsedOfIST8310) IST8310_ReadData();   // 
		}
//		if(g_sRealTimeCount.sampStep%100 == 51){         // 4ms采集一次
//				if(bInitGpsComIn)	 GPS_ReadData();
//		}

		if(g_bUsedOfSimuCom && g_bSimuComOutSucessful){
			 if(RingBufferGet(lpSimuOutQueue,&idxCurFrame,1)){
				 SoftDelayNus(1000);   // 延时0.1ms左右
				 DataDMA2SimuCom(idxCurFrame);
			 }
		}
		
		if(g_bUsedOfDtCom && g_bDtComOutSucessful && !g_bDtComOutBufIsUsing){
			 if(RingBufferGet(lpDtOutQueue,&idxCurFrame,1)){
				 SoftDelayNus(1000);   // 延时0.1ms左右
				 DataDMA2DtCom(idxCurFrame);
			 }
		}
}
#endif

//  该任务必须在最小仿真步长g_iSimulinkAlgorithmStep内执行完毕，否则会置错误标志erro_flag
void 	SysTask(void)
{
		if(g_sRealTimeCount.check_flag >= 1 )          // 1ms进入一次处理程序，每个调度周期超过2ms步长将置控制超时标志
		{
			g_sRealTimeCount.check_flag = 0;
			

			if(g_bUsedOfPressScanner && (g_sRealTimeCount.sampStep % AFC_SCANNER_PERIOD == 0))	ReadAllPressData();
			if(g_bUsedOfTempScanner && (g_sRealTimeCount.sampStep % AFC_SCANNER_PERIOD == 2))	ReadAllTemperatureData();
			
			if(g_sRealTimeCount.sampStep%4 == 1){         // 4ms采集一次
					if(g_bUsedOfADSample) ADSample_ReadData();
			}
			
			if(g_sRealTimeCount.sampStep%20 == 1){         // 20ms采集一次
				if(g_bUsedOfDPS310 && (!bSPI4IsUsing)){
					bSPI4IsUsing = true;
					DPS310_ReadData();    // 采集气压计数据
					bSPI4IsUsing = false;
				}
			}
					
			if(g_sRealTimeCount.sampStep%20 == 11){         // 20ms采集一次
				if(g_bUsedOfIST8310) IST8310_ReadData();   // 
			}

			// 1000ms采集1次
			if(g_sRealTimeCount.sampStep % 1000 == 0) {          //1Hz
					g_sRealTimeCount.sampStep = 0;

					if(bInitSBusComIn) Sbus_Read_Cnt();
			}

			// 安徽进场后，发现空速计采集延迟引起地面站帧延迟问题，将DtCom和SDCard程序调整至此。2025.07.23 modified by ZQingHua			
			if(g_bUsedOfDtCom && g_bDtComOutSucessful && (DtCom.gState == HAL_UART_STATE_READY)){
				 if(RingBufferGet(lpDtOutQueue,&idxCurFrame,1)){
					 SoftDelayNus(2000);   // 延时0.2ms左右
					 DataDMA2DtCom(idxCurFrame);
				 }
			}
			
			if(g_bUsedOfUSBDisk && g_bWriteUSBDiskFlg){
				 g_bWriteUSBDiskFlg = false;
				 ModelVar2USBDisk();
			}

	// 从DtCom读取从地面站发送过来的控制参数或指令
			if(g_bUsedOfAFCBasicAC) DtCom2CtrlParam();
			ProGs2FcsCmdFrame();			
		}
}

uint16_t iCounter = 0;
// AFC-1主流程调度任务，包括：SimuCom、DtCom和SdCard等操作
void vAFCTaskSchedule(void)
{
		if(g_bUsedOfSimuCom && g_bSimuComOutSucessful){
			 if(RingBufferGet(lpSimuOutQueue,&idxCurFrame,1)){
				 SoftDelayNus(100);   // 延时0.1ms左右。注意1ms = 1000us
				 DataDMA2SimuCom(idxCurFrame);
			 }
//			 	// 从环形缓冲区中提取帧内容，解析SimuCom的传送数据
//				vRcvSimuComInfTask();
		}
		
		if(g_bUsedOfMs5525 && (!bI2C1IsUsing)){
			bI2C1IsUsing = true;
			MS5525_ReadData();   // 空速计采集任务执行周期较长，至少在30ms以上，故将其采集任务置于主流程中
			iCounter++;
			bI2C1IsUsing = false;
		}
		
		if(g_bUsedOfWLan01 && bWLanRecvReady01 && 
			// V6.01.260104 金长征在调试网口时发现，网口接收线程vStartWLanRecvThread原来是11ms调度一次，在这种情况下丢帧非常严重；
		   // 后面改为每1ms处理一次网络端口输入监测即可正常工作，不再出现网络通讯丢帧现象
			((g_sRealTimeCount.sampStep%(1000 / g_iSimulinkAlgorithmStep) == 0))){  // 每1ms处理一次网络端口输入监测
			   vStartWLanRecvThread(0);
		}

		vOverTimePro();
}

void MainProc(void)
{
#ifndef __CONNECTED_IO__
	// 根据启动特征选择进入参数设置/标定模块或正常飞行的任务调度模块
		vAFCTaskSchedule();  // AFC-5正常运行任务调度模块
#else
		CioDeviceSampleTask();
		ConnectedIOProc();
#endif
}

// 任务初始化模块
void AFCTaskInit(void)
{
	InitDWT();  // 采用硬件精确延时Nus方案，初始化

	// 初始化无人机配置参数，包括无人机所用电路板、机架类型、传感器标校参数等，参数来源于A35保存的上次参数或默认值
	initAFCUavPara();

	// 系统参数初始化
	vSysParamInit();
	
	// 任务调度用的计数参数初始化
	vInitRealTaskParameter();
	//	仅打开任务定时器htim12中断, TIM6/13/14在任务调度模块初始化中处理，生成相关任务，则打开否则不打开
	//		HAL_TIM_Base_Start_IT(&htim12);
	// GPS、MEMS等的初始化测试以最后用户模型设置为准。
	// 初始化Simulink算法模型。GPS、MEMS等的初始化测试以最后用户模型设置为准。
	InitializeModel();

	initSysChipIdValid();

	// 当使用BasicAC算法库时，初始化DtCom控制参数，这些参数是完全与BasicAC库配套的，如果不使用此库或此库控制系统结构有调整则  V5.03.240229
	if(g_bUsedOfAFCBasicAC) initDtComCtrlPara();

	initGs2FcsCmdFrame();

	initTask0Period();
	// 等待自检完成，特别是电调约在2s后听到正常“滴滴....滴”声音后才正常		
			// 等待系统自检完成
	HAL_Delay(3000);
	// 初始化完成，清零超时故障监测标志	
	g_sRealTimeCount.err_flag = 0;
}
/************************ (C) COPYRIGHT ACG co. *****END OF FILE****/
