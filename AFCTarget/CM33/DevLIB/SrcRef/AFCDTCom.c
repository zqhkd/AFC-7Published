/******************** (C) COPYRIGHT 2021 AFC Tech Co.*************************
 * 作    者  ： 曾庆华
 * 文 件 名  ：AFCDTCom.c
 * 版    本  ：
 *   AFC-5V5.02.230920: 根据仲栋ConnectPC.c进行全面升级更改 
 *   AFC-6V1.01.251023: 反馈存在丢帧，恢复至AFC-5V5.05.250419/0401P之前的老版本。但后续发现使用网口会死机，且是AFCDtCom引起。    
 *   AFC-6V1.01.251112: 引入网口BSP5500模块，251023版本死机，网口助手scomm在TCP Server下都不能检测到本机IP:192.168.1.18
 *         但仅仅将AFC-6V1.01.250917中AFCDtCom.c拷贝过来更新，则使用网口正常，仔细对比，也未根因。
 *             
 * 描    述  ：数传模块串口接口函数
 * 
 * 官    网  ：www.acecreator.com
 * 淘    宝  ：acecreator.taobao.com
 * 公 众 号  ：飞行控制与仿真
 *
*****************************************************************************/
#include "AFCDTCom.h"
#include "ACGCommonAPI.h"
#include "AFCGlobalVar.h"

#include "ICM42688.h"
#include "ICM20602.h"
#include "DPS310.h"
#include "IST8310.h"
#include "SbusRC.h"
#include "Sensor.h"
#include "AFCGpsCom.h"
#include "AFCAPI.h"

#include "atti_control.h"
#include "alt_control.h"

#define AttitudeCtlParaFrameNo     0xE1
#define GetAttitudeCtlParaFrameNo  0xEB
#define HeightCtlParaFrameNo       0xE2
#define GetHeightCtlParaFrameNo    0xEC

#define GetUserModelInfCmd         0xF1
#define GetCompTestCmd             0xF2
#define GetCompCalibCmd            0xF3
#define GetCalibResCmd             0xF4
#define GetSetupParaCmd            0xF5
#define GetSetFlyStsCmd            0xFA

#define ICM42688FrameCmd           0xAA
#define ICM20602FrameCmd           0xAB
#define DPS310FrameCmd             0xAC
#define IST8310FrameCmd            0xAD
#define GpsFrameCmd                0xAE
#define RemoterFrameCmd            0xAF


#define DtComChkMode       SUM_CHECK
//#define BaseInFrameType    0x70
//#define BaseOutFrameType   0xAA      // 

//#define MaxResponseNum2InDtFrame  3   // 对于有串口中断输入时的回复帧次数

#define MaxDtInFrameTypeNum   20
#define MaxDtOutFrameTypeNum  30
#define MaxSignalNumOfDtFrame 50         // 假设串口波特率为115200bps, 单帧1ms约传输11byte, 则10ms约传输110byte, 约27个浮点数，因此需要注意传输数据数量与波特率的匹配性

// 外部可使用全局变量
uint8_t  g_sDtComTxBuf[DtCom_MAX_Tx_SIZE]; 		// 单帧DMA发送缓冲,  最大DtCom_MAX_Tx_SIZE字节
uint8_t  g_sDtComRxBuf[DtCom_MAX_Rx_SIZE]; 		// 单帧串口中断接收缓冲,  最大DtCom_MAX_Rx_SIZE字节
int8_t  g_curDtComRcvLen;

TRingBuffer *lpDtRingBuff;

uint8_t g_iMaxDtInFrameNum = 0;
uint8_t g_iResDtFrameNum[MaxDtInFrameTypeNum];
uint8_t g_iDtInFrameType[MaxDtInFrameTypeNum];   // 输入帧的帧类型码或帧序列码
int8_t g_iDtInSignalNum[MaxDtInFrameTypeNum];
bool g_bDtInFrameFlg[MaxDtInFrameTypeNum];
float g_fDtInData[MaxDtInFrameTypeNum][MaxSignalNumOfDtFrame];     // 模型串口输入变量

uint8_t g_iMaxDtOutFrameNum = 0;
uint8_t g_iDtOutFrameType[MaxDtOutFrameTypeNum];   // 输出帧的帧类型码或帧序列码
int8_t g_iDtOutSignalNum[MaxDtOutFrameTypeNum];
float g_fDtOutData[MaxDtOutFrameTypeNum][MaxSignalNumOfDtFrame];    // 通过DtCom串口输出的参数

//bool g_bDtComTxBufIsUsing = false;

uint8_t send_same_dtpack_times = 0;
uint8_t resDtInFrameType = 0;  //是否发送相同数据包标志位

// 进行DtCom多帧多周期处理, 最多允许一个仿真周期30个帧排队发送
#define MAX_DTQUEUE_SIZE  40   // V5.05.241109: 更改为最多40个帧排队队列数据
bool g_bUsedOfDtCom = false,g_bUsedOfTWDtCom = false;
TRingBuffer *lpDtOutQueue;

void ClrDtComSignalNum(void)
{
	uint8_t i;
	for(i = 0; i < MaxDtInFrameTypeNum; i++)  g_iDtInSignalNum[i]  = -1;
	for(i = 0; i < MaxDtOutFrameTypeNum; i++) g_iDtOutSignalNum[i] = -1;
}

// 初始化模型输出串口
void initDtComOut(uint8_t iSigNum,uint8_t iFrameType)
{
	  // 先检查当前帧iFrameType是否在输出帧队列中
	  uint8_t iCurFrameNo = iChkFrameTypeIdx(iFrameType, g_iMaxDtOutFrameNum,g_iDtOutFrameType);
	  if(iCurFrameNo == g_iMaxDtOutFrameNum) g_iMaxDtOutFrameNum++;   // 输出帧数量, 如果是在队列尾部添加帧类型码，则增加帧数量
	
	  if(iSigNum > MaxSignalNumOfDtFrame) iSigNum = MaxSignalNumOfDtFrame;
	  g_iDtOutSignalNum[iCurFrameNo] = iSigNum;
	  g_iDtOutFrameType[iCurFrameNo] = iFrameType;

	  for(uint8_t i = 0; i < iSigNum; i++) g_fDtOutData[iCurFrameNo][i] = 0.f;

	  g_bUsedOfDtCom = g_bDtComOutSucessful = true;
	
	  if(lpDtOutQueue==NULL) lpDtOutQueue = InitRingBuffer(lpDtOutQueue,MAX_DTQUEUE_SIZE);
}

// 初始化DMA模式的DtCom(huart2)。最关键问题是打开空闲中断
void InitDtComDMA(void)
{
	  lpDtRingBuff = InitRingBuffer(lpDtRingBuff,DtCom_MAX_Rx_SIZE*2); 
	
		InitComDMA(&DtCom,g_sDtComRxBuf,DtCom_MAX_Rx_SIZE);
}

// 以下两个函数initReadDtCom、getDtComVal是和readDtComAPI.c及.tlc程序配合使用的函数
void initDtComIn(uint8_t iSigNum,uint8_t iFrameType,uint8_t iResFrameNum)
{
	  // 先检查当前帧iFrameType是否在输入帧队列中，返回索引。没有就返回输入帧数量值
	  uint8_t iCurFrameNo = iChkFrameTypeIdx(iFrameType, g_iMaxDtInFrameNum,g_iDtInFrameType);
	  if(iCurFrameNo==g_iMaxDtInFrameNum) g_iMaxDtInFrameNum++;   // 输入帧数量, 如果是在队列尾部添加帧类型码，则增加帧数量
	
	  if(iSigNum > MaxSignalNumOfDtFrame) iSigNum = MaxSignalNumOfDtFrame;
	  g_iDtInSignalNum[iCurFrameNo] = iSigNum;
	  g_iDtInFrameType[iCurFrameNo] = iFrameType;
	  g_bDtInFrameFlg[iCurFrameNo]  = false;
	  g_iResDtFrameNum[iCurFrameNo] = iResFrameNum;    // 
	
		//  初始化数传串口接收DMA
	  if(!bInitDtComIn){
	     if(lpDtRingBuff==NULL) InitDtComDMA();    // V4.02.220422：将该初始化程序从main函数中移至此处，确保当DtCom打开时才使用。
			 
	   // V5.03.240226添加对DtComIn数据的初始化赋0操作
			for(uint8_t i = 0; i < MaxDtInFrameTypeNum; i++)
					for(uint8_t j =0; j < MaxSignalNumOfDtFrame; j++)
			             g_fDtInData[i][j] = 0.0f;
    }
			
		bInitDtComIn = true;
}

// 根据帧类型码查找该帧类型码在g_iDtOutFrameType中的索引值
uint8_t iFindDtOutFrameIdx(uint8_t iFrameType)
{
	  return iFindIdBuffIdx(iFrameType,g_iMaxDtOutFrameNum,g_iDtOutFrameType);
}

uint8_t iFindDtInFrameIdx(uint8_t iFrameType)
{
		return iFindIdBuffIdx(iFrameType,g_iMaxDtInFrameNum,g_iDtInFrameType);
}

void dma_sendbuff_dtcom(uint16_t len)
{
 //等待上一次的数据发送完毕
	uint16_t i = 0;
  while(HAL_DMA_GetState(DtCom.hdmatx) == HAL_DMA_STATE_BUSY){
	   if(i++ > 1000) break;   // 超时处理
	};
	
	if(i<1000){
	/* 2020.10.24 防止出现因状态未被复位，导致无法发送的情况(过去由HAL_UART_DMAStop()关闭，现在。。。) */
	  if(DtCom.gState != HAL_UART_STATE_READY)	HAL_UART_DMAStop(&DtCom);
	
    /* 关闭DMA */
 //   __HAL_DMA_DISABLE(SimuCom.hdmatx);
		
		g_bDtComOutSucessful = false;
    //开始发送数据
    HAL_UART_Transmit_DMA(&DtCom,g_sDtComTxBuf,len);
  }
}

//void dma_sendbuff_dtcom(uint16_t len)
//{
//  // 核心修改1：延长超时至10ms（适配所有板卡的硬件响应）
//  uint32_t timeout = HAL_GetTick() + 6;
//  // 核心修改2：同时等待DMA空闲 + UART就绪 + TC标志置位
//  while((HAL_DMA_GetState(DtCom.hdmatx) == HAL_DMA_STATE_BUSY) || 
//        (DtCom.gState != HAL_UART_STATE_READY) ||
//        (!__HAL_UART_GET_FLAG(&DtCom,UART_FLAG_TC))) {
//    if(HAL_GetTick() > timeout) break; 
//  };

//  if(HAL_GetTick() <= timeout){
//    if(DtCom.gState != HAL_UART_STATE_READY) HAL_UART_DMAStop(&DtCom);
//    
//    g_bDtComOutSucessful = false;
//    HAL_UART_Transmit_DMA(&DtCom,g_sDtComTxBuf,len);
//  }
//  // 核心修改3：超时后强制复位状态，避免卡死
//  else {
//    HAL_UART_DMAStop(&DtCom);
//    __HAL_UART_CLEAR_FLAG(&DtCom,UART_FLAG_TC);
//    g_bDtComOutSucessful = true;
//  }
//}

// 根据当前不同工作模式（地面测试、标定、地面待飞、飞行等）确定给定帧是否输出
bool bChkFrameOutEn(uint8_t iFrameTypeCode)
{
	  bool bFlg = false;
	  
	// 在地面测试或标定模式下，帧类型码>0x70则可输出。目前主要是0xF1--0xFA的帧类型码数据、以及飞控算法参数0x71相关数据可输出
	  if(((g_iCurRunMode == 1) || (g_iCurRunMode == 2)) && (iFrameTypeCode >=0x70)) bFlg = true;
	
	// 在地面待飞模式下，帧类型码< 0x90则可输出，以及>=E0以上时可输出，测试和校准的参数不允许输出。
	//  目前主要是1--20的用户帧类型码数据、以及飞控算法参数0x71--7C相关数据可输出
	  if((g_iCurRunMode == 0) && ((iFrameTypeCode < 0x90) || (iFrameTypeCode >= 0xE0))) bFlg = true;

	// 飞行时仅允许用户帧类型码数据、以及飞控算法参数0x71--7C相关数据可输出。遥控解锁时g_iCurRunMode置为100
	  if((g_iCurRunMode == 100) && (iFrameTypeCode < 0x90)) bFlg = true;
	
	  return bFlg;
}

// 通过模型端口发送信息，DMA方式
void DataDMA2DtCom(uint8_t idxCurFrame)
{
//	  g_bDtComTxBufIsUsing = true;
	  uint8_t iFrameTypeCode = g_iDtOutFrameType[idxCurFrame];
	// 根据当前不同工作模式（地面测试、标定、地面待飞、飞行等）确定给定帧是否输出
 	  if(bChkFrameOutEn(iFrameTypeCode)){
			uint16_t len;
			THeaderFrame pHeaderFrame;
			__HAL_UART_DISABLE_IT(&DtCom, UART_IT_IDLE);

			pHeaderFrame.iFrameTick = g_sRealTimeCount.fcsTime;
			pHeaderFrame.idSender = getCurUAVId();             // V5.05.241003
			//			pHeaderFrame.idReceiver = ID_OF_GCS;

			if((resDtInFrameType!=0)&&(send_same_dtpack_times > 0)) {    // 如果从串口接收到数据，则暂停正常发送，而将刚刚收到数据发送MaxResponseNum2InFrame遍
					pHeaderFrame.iFrameType = resDtInFrameType;
					uint8_t idxFrame = iFindDtInFrameIdx(resDtInFrameType);
					len = iPackData2Hex(pHeaderFrame,g_iDtInSignalNum[idxFrame] + 1,g_fDtInData[idxFrame],g_sDtComTxBuf,SUM_CHECK); 
				
					if(send_same_dtpack_times > 1) send_same_dtpack_times--;
					else resDtInFrameType = 0;
			}
			else{
				pHeaderFrame.iFrameType = iFrameTypeCode;  // 发送给DtCom的帧类型码
				
			// 原来为虚函数，需用它才能将外部虚函数实化。
				len = iPackData2Hex(pHeaderFrame,g_iDtOutSignalNum[idxCurFrame],g_fDtOutData[idxCurFrame],g_sDtComTxBuf,SUM_CHECK); 
			}

//			g_bDtComTxBufIsUsing = false;

			// 授权管控, 20250218。(正式授权则直接有效，或者帧类型码大于0x9F以上的管理帧等)
			if(g_UavFcsParam.UavPara.bAuthorizedFlg || fGetRandVal() > 0.07f || iFrameTypeCode > 0x9F){
				dma_sendbuff_dtcom(len);   // DMA发送缓冲区一定要使用全局变量，否则极易引起内存泄漏
				__HAL_UART_ENABLE_IT(&DtCom, UART_IT_IDLE);
			}
	 }
}

// 传递指定参数给数传串口
void writeDtCom(uint8_t iFrameType,uint8_t iChannel,double fVal)
{
// 根据当前不同工作模式（地面测试、标定、地面待飞、飞行等）确定给定帧是否输出
	if(bChkFrameOutEn(iFrameType)){
		 static int iSigNum = 0;
		 g_bDtComOutBufIsUsing = true;
		 uint8_t idxCurDtFrame = iFindDtOutFrameIdx(iFrameType);
		 if(g_iDtOutSignalNum[idxCurDtFrame] >=0){
			 if(iChannel < g_iDtOutSignalNum[idxCurDtFrame]){
					g_fDtOutData[idxCurDtFrame][iChannel] = fVal;
					iSigNum++;
					if(iSigNum >= g_iDtOutSignalNum[idxCurDtFrame]){
	// 当前帧的最后一个变量，则将当前帧数据打包输出，且将帧序号清0
						RingBufferPut(lpDtOutQueue,&idxCurDtFrame,1);
						iSigNum = 0;
						g_bDtComOutBufIsUsing = false;
					}
			 }
		 }
	 }
}

// 从环形缓冲区中提取帧内容，解析DtCom的传送数据
void vRcvDtComInfTask(void)
{
	static uint8_t iState = 1;
	static uint8_t pBuff[DtCom_MAX_Rx_SIZE];
	static bool bExitFlg = false;
	uint16_t iFrameLen,iBuffLen;

	while(!(bExitFlg || RingBufferEmpty(lpDtRingBuff)))		// 判断环形缓冲区是否为空,或者是退出标志？
	{
	    // 非空，则开始处理数据
		switch (iState)
    {
			// 开始判断帧头第1字节
      case 1:
				RingBufferGet(lpDtRingBuff,pBuff,1);
				if(pBuff[0] == (FRAME_HEADER_ID&0xff)){
					iState = 2;
				}
      	break;
			// 开始判断帧头第2字节
      case 2:
				RingBufferGet(lpDtRingBuff,pBuff + 1,1);
				if(pBuff[1] == ((FRAME_HEADER_ID&0xff00)>>8)){
					iState = 3;
				}
				else iState = 1;   // 如果第2字节不是帧头，则返回至状态0，重新开始寻找帧头
      	break;
			case 3:
			// 读取帧长度
				if(GetRingBufferLen(lpDtRingBuff) >= 2){   // 不到2个字节时，则需等待第二次进入
					RingBufferGet(lpDtRingBuff,pBuff+2,2);
					iFrameLen = pBuff[FrameLengthPos] + (pBuff[FrameLengthPos + 1] << 8);
					iBuffLen = GetRingBufferLen(lpDtRingBuff) + 4;           //  AFC-5V5.03.240522更改帧长度异常注入的问题
					if((iFrameLen < DtCom_MAX_Rx_SIZE) && (iFrameLen <= iBuffLen)){        //  AFC-5V5.03.240522更改帧长度异常注入的问题，添加iFrameLen <= iBuffLen条件
						iState = 4;
					}
					else{
						iState = 1;  // 如果帧长度不对，抛弃该帧。
						bExitFlg = true;  // 20240301再次证明：帧长度在DtCom_MAX_Rx_SIZE范围内，但不对会导致死机，需要处理。20240418增加此行，强行退出读取缓冲区代码
					}
				}
				break;
			case 4:
			// 确保环形缓冲区有iFrameLen - 4个数据，表示一个完整数据帧
				if(GetRingBufferLen(lpDtRingBuff) >= iFrameLen - 4){
					RingBufferGet(lpDtRingBuff, pBuff + 4, iFrameLen - 4);

					int idxFrame = iFindDtInFrameIdx(pBuff[FrameTypePos]);
					if((bChkFrameValid(pBuff,DtComChkMode,iFrameLen))&&(idxFrame >= 0)){
						 uint8_t iVar = iUnPackHex2Data(pBuff,iFrameLen,g_fDtInData[idxFrame],DtComChkMode);
						 
						 // V5.05.240830: 动态调整帧队列中信号个数
						 g_iDtInSignalNum[idxFrame] = iVar;
						 
	 					 g_bDtInFrameFlg[idxFrame] = true;

						 // 收到数据后，回复装订指定次数的帧
						 send_same_dtpack_times = g_iResDtFrameNum[idxFrame];
						 resDtInFrameType = pBuff[FrameTypePos];
					}
					iState = 1;  bExitFlg = true;
				}
				break;
      default:
     		break;
     }
	}
	bExitFlg = false;
}

// 处理DtCom的空闲中断DMA：接收DtCom字符串
void ProDtComRcvIRQ(void)
{  
	  uint32_t isrflags   = READ_REG(DtCom.Instance->ISR);
    uint32_t cr1its     = READ_REG(DtCom.Instance->CR1);
    bool bClearOverFlag = true;
	
    if(((isrflags & USART_ISR_IDLE) != RESET) && ((cr1its & USART_CR1_IDLEIE) != RESET) && ((isrflags & USART_ISR_ORE) != USART_ISR_ORE)){
	    __HAL_UART_CLEAR_IDLEFLAG(&DtCom);
	    uint32_t _len_dmarev = DtCom_MAX_Rx_SIZE - __HAL_DMA_GET_COUNTER(DtCom.hdmarx);
	    if(_len_dmarev){
				// V5.05.241007: 关闭接收中断
        __HAL_UART_DISABLE_IT(&DtCom, UART_IT_RXNE);
//            HAL_UART_DMAStopRx(&DtCom);
			/* 这里以前使用HAL_UART_DMAStop(huart)，但这个函数会导致TX的DMA被关闭，小概率DMA发送会丢数据 */
			/* 2021.10.24 修改为HAL_UART_AbortReceive(huart)，对应DMA发送也需要做一些处理 */
				HAL_UART_AbortReceive(&DtCom);

       // 在未打开串口接收功能时，由于存在乱码引起的错误接收中断，需要对此进行特殊处理，否则由于对未初始化缓冲区操作，会导致内存泄漏，引起HardFault中断
				if(bInitDtComIn){
						// 向环形缓冲区写数据
						RingBufferPut(lpDtRingBuff,g_sDtComRxBuf,_len_dmarev);
						// 从环形缓冲区中提取帧内容，解析DtCom的传送数据
						vRcvDtComInfTask();
					
		        // 清空DMA接收缓冲区
            memset(g_sDtComRxBuf, 0, DtCom_MAX_Rx_SIZE);

						HAL_UART_Receive_DMA(&DtCom, g_sDtComRxBuf, DtCom_MAX_Rx_SIZE); 
				}
				
	        __HAL_UART_DISABLE_IT(&DtCom, UART_IT_ERR);
	        __HAL_UART_DISABLE_IT(&DtCom, UART_IT_PE);
			
        // 不用清除溢出标志，其它情况都需要清除			
		    bClearOverFlag = false;
	    }
    }
 	  if(bClearOverFlag){
			 READ_REG(DtCom.Instance->ISR);
			 READ_REG(DtCom.Instance->RDR);       // 抛弃所接受的数据
			 __HAL_UART_CLEAR_OREFLAG(&DtCom);    // 清除ORE中断
	//	   __HAL_UART_CLEAR_IDLEFLAG(&DtCom);
		}
}

// 从模型串口DtCom读取变量到模型端口(ReadMC模块)，供用户模型Simulink程序使用。
double readDtCom(uint8_t idFrameType,uint8_t iChanel)
{
	  double fVal = 0x00;
	  static int iSigNum = 0;
	  
	  int idxFrame = iFindDtInFrameIdx(idFrameType);
//	  if(g_bDtInFrameFlg[idxFrame]){  // 用该方法的话，读取一次后数据会回0
	  if(idxFrame >= 0 ){    
			  fVal = g_fDtInData[idxFrame][iChanel];
			  iSigNum++;
			  if(iSigNum >= g_iDtInSignalNum[idxFrame] + 1){          // 
				   g_bDtInFrameFlg[idxFrame] = false;
				   iSigNum = 0;
			  }
		}
	  return fVal;
}

// 清除串口DtCom给定帧给定端口的数据内容
void clrDtComInVal(uint8_t idFrameType,uint8_t iChanel)
{
	  int idxFrame = iFindDtInFrameIdx(idFrameType);
	  if(idxFrame >= 0 ){    
			  g_fDtInData[idxFrame][iChanel] = 0.f;
		}
}

void DtCom2ModelVar_Atti(void)
{
 // time = readDtCom(81,0);
	uint8_t i = 1;
	float fUpdateFlg = readDtCom(AttitudeCtlParaFrameNo,i++);
	if(fUpdateFlg > 100.f){
		clrDtComInVal(AttitudeCtlParaFrameNo ,1);
		angle_kP[0] =  readDtCom(AttitudeCtlParaFrameNo,i++);
		angle_kP[1] =  readDtCom(AttitudeCtlParaFrameNo,i++);
		angle_kP[2] =  readDtCom(AttitudeCtlParaFrameNo,i++);
		
		pid_rate[0].kp =  readDtCom(AttitudeCtlParaFrameNo,i++);
		pid_rate[0].ki =  readDtCom(AttitudeCtlParaFrameNo,i++);
		pid_rate[0].kd =  readDtCom(AttitudeCtlParaFrameNo,i++);
		
		pid_rate[1].kp =  readDtCom(AttitudeCtlParaFrameNo,i++);
		pid_rate[1].ki =  readDtCom(AttitudeCtlParaFrameNo,i++);
		pid_rate[1].kd =  readDtCom(AttitudeCtlParaFrameNo,i++);
		
		pid_rate[2].kp  =  readDtCom(AttitudeCtlParaFrameNo,i++);
		pid_rate[2].ki  = readDtCom(AttitudeCtlParaFrameNo,i++);
		pid_rate[2].kd  = readDtCom(AttitudeCtlParaFrameNo,i++);	
		
		// V5.05.240827: 增加油门相关参数：巡航油门ThrottleHover、怠速油门IdlingSpeedWidth、
		//  降落关机油门ShutOffThrottle、Z向加速度Azcms以及向上/向下运动时的最大误差限制Leash_up_z、Leash_down_z等
		fThrottleHover  =  readDtCom(AttitudeCtlParaFrameNo,i++);
		g_iIdlingSpeedWidth  = (uint16_t)readDtCom(AttitudeCtlParaFrameNo,i++);
		fShutOffThrottle  = readDtCom(AttitudeCtlParaFrameNo,i++);	
		accel_z_cms  =  readDtCom(AttitudeCtlParaFrameNo,i++);
		leash_down_z  = readDtCom(AttitudeCtlParaFrameNo,i++);
		leash_up_z  = readDtCom(AttitudeCtlParaFrameNo,i++);	
	}
}

void ModelVar2DtCom_Atti(void)
{
	  uint8_t i =0;
		writeDtCom(AttitudeCtlParaFrameNo,i++,101.0f);
		writeDtCom(AttitudeCtlParaFrameNo,i++,angle_kP[0]);
		writeDtCom(AttitudeCtlParaFrameNo,i++,angle_kP[1]);
		writeDtCom(AttitudeCtlParaFrameNo,i++,angle_kP[2]);
	
		writeDtCom(AttitudeCtlParaFrameNo,i++,pid_rate[0].kp);
		writeDtCom(AttitudeCtlParaFrameNo,i++,pid_rate[0].ki);
		writeDtCom(AttitudeCtlParaFrameNo,i++,pid_rate[0].kd);
	
		writeDtCom(AttitudeCtlParaFrameNo,i++,pid_rate[1].kp);
		writeDtCom(AttitudeCtlParaFrameNo,i++,pid_rate[1].ki);
		writeDtCom(AttitudeCtlParaFrameNo,i++,pid_rate[1].kd);
	
		writeDtCom(AttitudeCtlParaFrameNo,i++,pid_rate[2].kp);
		writeDtCom(AttitudeCtlParaFrameNo,i++,pid_rate[2].ki);
		writeDtCom(AttitudeCtlParaFrameNo,i++,pid_rate[2].kd);

		// V5.05.240827: 增加油门相关参数：巡航油门ThrottleHover、怠速油门IdlingSpeedWidth、
		//  降落关机油门ShutOffThrottle、Z向加速度Azcms以及向上/向下运动时的最大误差限制Leash_up_z、Leash_down_z等
	  writeDtCom(AttitudeCtlParaFrameNo,i++,fThrottleHover);
	  writeDtCom(AttitudeCtlParaFrameNo,i++,(float)g_iIdlingSpeedWidth);
	  writeDtCom(AttitudeCtlParaFrameNo,i++,fShutOffThrottle);
	  writeDtCom(AttitudeCtlParaFrameNo,i++,accel_z_cms);
	  writeDtCom(AttitudeCtlParaFrameNo,i++,leash_down_z);
	  writeDtCom(AttitudeCtlParaFrameNo,i++,leash_up_z);
//	  writeDtCom(AttitudeCtlParaFrameNo,i++,0.0f);
}

void DtCom2ModelVar_AttiGet(void)
{
 // time = readDtCom(81,0);
  uint16_t iGetAttiCmd =  (uint16_t) readDtCom(GetAttitudeCtlParaFrameNo ,1);

	if(iGetAttiCmd == 101){
		 clrDtComInVal(GetAttitudeCtlParaFrameNo ,1);
		 ModelVar2DtCom_Atti();
		
//// 考虑到上位机有时不能正常接收到刷新数据，对回令数据连续发两帧，确保数据正常接收
//		 ModelVar2DtCom_Atti();
	}
	else if(iGetAttiCmd == 102){
// 参数保存功能
		
	}
}

// 
void DtCom2ModelVar_Height(void)
{
 // time = readDtCom(81,0);
	uint8_t i = 1;
	float fUpdateFlg = readDtCom(HeightCtlParaFrameNo,i++);

	if(fUpdateFlg>100.f){
		clrDtComInVal(HeightCtlParaFrameNo ,1);
		alt_pos_kP      =  readDtCom(HeightCtlParaFrameNo,i++);
		
		alt_rate.kp =  readDtCom(HeightCtlParaFrameNo,i++);
		alt_rate.ki =  readDtCom(HeightCtlParaFrameNo,i++);
		alt_rate.kd =  readDtCom(HeightCtlParaFrameNo,i++);
		
		alt_accel.kp =  readDtCom(HeightCtlParaFrameNo,i++);
		alt_accel.ki =  readDtCom(HeightCtlParaFrameNo,i++);
		alt_accel.kd =  readDtCom(HeightCtlParaFrameNo,i++);
		
	//  IMU_pos_kp =  readDtCom(AttitudeCtlParaFrameNo,i++);
	//  IMU_rate_kp =  readDtCom(AttitudeCtlParaFrameNo,i++);
	//  IMU_rate_ki =  readDtCom(AttitudeCtlParaFrameNo,i++);
	//  IMU_rate_kd =  readDtCom(AttitudeCtlParaFrameNo,i++);
	//	
	//  batt_V_set  =  readDtCom(AttitudeCtlParaFrameNo,i++);
	//  batt_V_div  = readDtCom(AttitudeCtlParaFrameNo,i++);
	}
}

void ModelVar2DtCom_Height(void)
{
	  uint8_t i =0;
	
		writeDtCom(HeightCtlParaFrameNo,i++,101.0f);
	
		writeDtCom(HeightCtlParaFrameNo,i++,alt_pos_kP);
	
		writeDtCom(HeightCtlParaFrameNo,i++,alt_rate.kp);
		writeDtCom(HeightCtlParaFrameNo,i++,alt_rate.ki);
		writeDtCom(HeightCtlParaFrameNo,i++,alt_rate.kd);
	
		writeDtCom(HeightCtlParaFrameNo,i++,alt_accel.kp);
		writeDtCom(HeightCtlParaFrameNo,i++,alt_accel.ki);	
		writeDtCom(HeightCtlParaFrameNo,i++,alt_accel.kd);
	
		writeDtCom(HeightCtlParaFrameNo,i++,0.0f);
		writeDtCom(HeightCtlParaFrameNo,i++,0.0f);
		writeDtCom(HeightCtlParaFrameNo,i++,0.0f);
		writeDtCom(HeightCtlParaFrameNo,i++,0.0f);
	
		writeDtCom(HeightCtlParaFrameNo,i++,0.0f);
		writeDtCom(HeightCtlParaFrameNo,i++,0.0f);
//		writeDtCom(HeightCtlParaFrameNo,i++,0.0f);
}

void DtCom2ModelVar_HeightGet(void)
{
 // time = readDtCom(81,0);
  uint16_t iGetHeightCmd = (uint16_t) readDtCom(GetHeightCtlParaFrameNo ,1);
	if(iGetHeightCmd == 101){
		 clrDtComInVal(GetHeightCtlParaFrameNo ,1);
		 ModelVar2DtCom_Height();
	}
	else if(iGetHeightCmd == 102){
		// 参数保存功能
		
	}
}

// 从地面站接收控制算法库BasicAC的调节参数
void DtCom2CtrlParam(void)
{
	  DtCom2ModelVar_Atti();
	  DtCom2ModelVar_Height();
	  DtCom2ModelVar_AttiGet();
	  DtCom2ModelVar_HeightGet();
}

// 为地面站接收控制算法库BasicAC调节参数获取指令帧，做好初始化工作
void initDtComCtrlPara(void)
{
//	// 配置装订初始化姿态控制调节参数
	  initDtComIn(19,AttitudeCtlParaFrameNo,1);   // 地面站-->AFC5飞控组件发送更改后的姿态调节参数，收到后给一帧回令进行确认。V5.05.240828增加6个油门指令
	//  请求获取姿态控制调节参数指令帧
	  initDtComIn(1,GetAttitudeCtlParaFrameNo,0);
	// 当接收到姿态控制调节参数获取指令，需要将这些参数发送至地面站
	  initDtComOut(19,AttitudeCtlParaFrameNo);    // AFC飞控组件-->地面站发送回读的姿态调节参数，V5.05.240828增加6个油门指令
	
	// 初始化定高控制调节参数
	  initDtComIn(14,HeightCtlParaFrameNo,1);  // 地面站-->AFC5飞控组件发送更改后的姿态调节参数，收到后给一帧回令进行确认

	// 请求获取高度控制调节参数指令帧
	  initDtComIn(1,GetHeightCtlParaFrameNo,0);
	
	// 当接收到姿态控制调节参数获取指令，需要将这些参数发送至地面站
	  initDtComOut(14,HeightCtlParaFrameNo);   // AFC飞控组件-->地面站发送回读的姿态调节参数
}

// 准备用户模型信息回令帧的数据
void initUserModelInfDtComOut(void)
{
	  uint8_t i,j,iLen;
	  uint8_t iCharLen = strlen(g_sModelName);
	  
	  if(iCharLen > 0){
//				float *fVal;
			  float fVal[MaxSignalNumOfDtFrame];
			
				TChar2FloatStruct pChar2Data;
				uint8_t iSigNum = iCalFloatNums(iCharLen);    // 将用户模型文件名字符串按4个一组进行处理
//			  fVal = (float *)malloc((iSigNum + 5)*sizeof(float));  // 创建1个指令码 + 用户模型名称等效浮点数量iSigNum + 另外4个版本信息

			  fVal[0] = 101;    // 表示当前为获取用户模型信息指令的回令帧
				for(i = 0; i < iSigNum; i++){
					for(j = 0; j < 4; j++){
						 iLen = i*4 + j;
						 if(iLen < iCharLen)
								pChar2Data.sChar[j] = g_sModelName[iLen];
						 else
							  pChar2Data.sChar[j] = 0x0;
				  }
					fVal[i + 1] = pChar2Data.fVal;
		    }
				
				fVal[++iSigNum] = atof((const char*)g_sModelVersion);
				fVal[++iSigNum] = atof((const char*)g_sAFCToolBoxVersion);
				fVal[++iSigNum] = AFC_SYSTEM_MAIN_VER;
				fVal[++iSigNum] = AFC_SYSTEM_DATE_VER;
				
				initDtComOut(iSigNum + 1, GetUserModelInfCmd);   // 初始化用户模型信息的响应输出帧

        // 将用户模型信息的内容输出至DtCom				
				for(i = 0; i < iSigNum + 1; i++) 		writeDtCom(GetUserModelInfCmd,i,fVal[i]);
				
//				free(fVal);
		}
}

// 安全的atof实现
float safeAtof(const char* str)
{
    if(str == NULL || *str == '\0') return 0.0f;
    char* endptr;
    float val = strtof(str, &endptr);
    return (endptr != str) ? val : 0.0f;
}

void initUserModelInfDtComOutBk(void)
{
    uint8_t i, j, iLen;
    uint8_t iCharLen = strlen(g_sModelName);
    
    if(iCharLen > 0){
        // 计算所需浮点数数量: 1(指令码) + ceil(iCharLen/4) + 4(版本信息)
        uint8_t iSigNum = iCalFloatNums(iCharLen);
        uint8_t totalSignals = 1 + iSigNum + 4; // 指令码 + 名称 + 版本
        
        // 检查缓冲区是否足够
        if(totalSignals > MaxSignalNumOfDtFrame) {
            // 错误处理或截断
            totalSignals = MaxSignalNumOfDtFrame;
            iSigNum = totalSignals - 5; // 调整可处理的字符数
        }

        float fVal[MaxSignalNumOfDtFrame] = {0}; // 初始化为0
        
        fVal[0] = 101; // 指令码
        
        // 安全处理模型名称转换
        TChar2FloatStruct pChar2Data = {0}; // 明确初始化
        for(i = 0; i < iSigNum; i++){
            for(j = 0; j < 4; j++){
                iLen = i*4 + j;
                pChar2Data.sChar[j] = (iLen < iCharLen) ? g_sModelName[iLen] : 0x0;
            }
            fVal[i + 1] = pChar2Data.fVal;
        }
        
        // 安全处理版本转换
        uint8_t currentIndex = 1 + iSigNum;
        fVal[currentIndex++] = safeAtof(g_sModelVersion);
        fVal[currentIndex++] = safeAtof(g_sAFCToolBoxVersion);
        fVal[currentIndex++] = AFC_SYSTEM_MAIN_VER;
        fVal[currentIndex++] = AFC_SYSTEM_DATE_VER;
        
        // 初始化通信帧
        initDtComOut(totalSignals, GetUserModelInfCmd);
        
        // 发送数据
        for(i = 0; i < totalSignals; i++) {
            writeDtCom(GetUserModelInfCmd, i, fVal[i]);
        }
    }
}

// 发送用户模型信息到地面站
void ModelVar2DtCom_ModelVer(uint16_t iCmd)    // 此时会置位g_iCurRunMode状态为1，表示组件测试状态
{
	  if(iCmd > 100){
			initUserModelInfDtComOut();  // 临时解决恢复飞行后有可能不能访问用户模型信息问题。由于lpDtOutQueue为循环缓冲队列，按理不会冲掉，后续需改进此处
		}
}

void vCloseTestCalibUsedFlg(void)
{
	 if(g_bTestCalibOpening[0]) g_bUsedOfICM42688 = false;
	 if(g_bTestCalibOpening[1]) g_bUsedOfICM20602 = false;
	 if(g_bTestCalibOpening[2]) g_bUsedOfDPS310     = false;
	 if(g_bTestCalibOpening[3]) g_bUsedOfIST8310 = false;
	 if(g_bTestCalibOpening[4]) bInitGpsComIn = false;
	 if(g_bTestCalibOpening[5]) bInitSBusComIn = false;
}

void initDeviceRawOutEn(bool bICM42688,bool bICM20602,bool bDPS310,bool bIST8310,bool bGPS, bool bRemoter)
{
		// 初始化ICM42688组件测试的响应输出
	 if(bICM42688){
		 if(!g_bUsedOfICM42688){
			 initICM42688();  g_bTestCalibOpening[0] = true;
		 }
		 initDtComOut(15,ICM42688FrameCmd);
	 }
	 
	// 初始化ICM20602组件测试的响应输出
	 if(bICM20602){
		 if(!g_bUsedOfICM20602){
			 initICM20602();	g_bTestCalibOpening[1] = true;
		 }
		 initDtComOut(15,ICM20602FrameCmd);
	 }

	// 初始化DPS310组件测试的响应输出
	 if(bDPS310){
		 if(!g_bUsedOfDPS310){
			 initDPS310();   g_bTestCalibOpening[2] = true;
		 }
		 initDtComOut(3,DPS310FrameCmd);
	 }

	// 初始化IST8310组件测试的响应输出
	 if(bIST8310){
		 if(!g_bUsedOfIST8310){
			 initIST8310();    g_bTestCalibOpening[3] = true;
		 }
		 initDtComOut(4,IST8310FrameCmd);
	 }
	 
	// 初始化GPS组件测试的响应输出
	 if(bGPS){
		 if(!bInitGpsComIn){
			 initGps(0);    g_bTestCalibOpening[4] = true;
		 }
		 initDtComOut(16,GpsFrameCmd);
	 }

	// 初始化遥控器组件测试的响应输出
	 if(bRemoter){
		 if(!bInitSBusComIn){
			  for(uint8_t i = 0; i < 10; i++)	initSBusCmd(i);
			  g_bTestCalibOpening[5] = true;
		 }
		 initDtComOut(10,RemoterFrameCmd);
	 }
}

// 初始化组件设备输出串口
void initTWDtComOut(bool bICM42688,bool bICM20602,bool bDPS310,bool bIST8310,bool bGPS, bool bRemoter)
{
	  g_bUsedOfDtCom = true;
	  if(lpDtOutQueue==NULL) lpDtOutQueue = InitRingBuffer(lpDtOutQueue,MAX_DTQUEUE_SIZE);
	  
	  initDeviceRawOutEn(bICM42688,bICM20602,bDPS310,bIST8310,bGPS,bRemoter);
	  g_bUsedOfTWDtCom =  g_bDtComOutSucessful = true;
}

// 发送组件设备测试结果给地面站
bool bICM42688=false,bICM20602=false,bDPS310=false,bIST8310=false,bGPS=false,bRemoter=false;

void CompTest2Gs(void)
{
	  uint8_t i;
	  if(bICM42688){
			i=0;
			writeDtCom(ICM42688FrameCmd,i++,icm_42688_acc[0]);    writeDtCom(ICM42688FrameCmd,i++,icm_42688_acc[1]);		writeDtCom(ICM42688FrameCmd,i++,icm_42688_acc[2]);
			writeDtCom(ICM42688FrameCmd,i++,icm_42688_gyr[0]);    writeDtCom(ICM42688FrameCmd,i++,icm_42688_gyr[1]);		writeDtCom(ICM42688FrameCmd,i++,icm_42688_gyr[2]);
			writeDtCom(ICM42688FrameCmd,i++,accel_offset[0].x);   writeDtCom(ICM42688FrameCmd,i++,accel_offset[0].y);	  writeDtCom(ICM42688FrameCmd,i++,accel_offset[0].z);
			writeDtCom(ICM42688FrameCmd,i++,gyro_offset[0].x);    writeDtCom(ICM42688FrameCmd,i++,gyro_offset[0].y);	  writeDtCom(ICM42688FrameCmd,i++,gyro_offset[0].z);
			writeDtCom(ICM42688FrameCmd,i++,roll_sensor/100.f);    			writeDtCom(ICM42688FrameCmd,i++,pitch_sensor/100.f);				writeDtCom(ICM42688FrameCmd,i++,yaw_sensor/100.f);
		}
	  if(bICM20602){
			i=0;
			writeDtCom(ICM20602FrameCmd,i++,icm_20602_acc[0]);    writeDtCom(ICM20602FrameCmd,i++,icm_20602_acc[1]);		writeDtCom(ICM20602FrameCmd,i++,icm_20602_acc[2]);
			writeDtCom(ICM20602FrameCmd,i++,icm_20602_gyr[0]);    writeDtCom(ICM20602FrameCmd,i++,icm_20602_gyr[1]);		writeDtCom(ICM20602FrameCmd,i++,icm_20602_gyr[2]);
			writeDtCom(ICM20602FrameCmd,i++,accel_offset[1].x);   writeDtCom(ICM20602FrameCmd,i++,accel_offset[1].y);	  writeDtCom(ICM20602FrameCmd,i++,accel_offset[1].z);
			writeDtCom(ICM20602FrameCmd,i++,gyro_offset[1].x);    writeDtCom(ICM20602FrameCmd,i++,gyro_offset[1].y);	  writeDtCom(ICM20602FrameCmd,i++,gyro_offset[1].z);
			writeDtCom(ICM20602FrameCmd,i++,roll_sensor/100.f);    			writeDtCom(ICM20602FrameCmd,i++,pitch_sensor/100.f);				writeDtCom(ICM20602FrameCmd,i++,yaw_sensor/100.f);
		}
	  if(bDPS310){
			i=0;
			writeDtCom(DPS310FrameCmd,i++,temperature);    writeDtCom(DPS310FrameCmd,i++,pressure);		writeDtCom(DPS310FrameCmd,i++,baro_altitude);
		}
	  
	  if(bIST8310){
			i=0;
			writeDtCom(IST8310FrameCmd,i++,g_fMagRaw[0]);    writeDtCom(IST8310FrameCmd,i++,g_fMagRaw[1]);		writeDtCom(IST8310FrameCmd,i++,g_fMagRaw[2]);
			writeDtCom(IST8310FrameCmd,i++,ist8310_angle);
		}
	  if(bGPS){
			i=0;
			writeDtCom(GpsFrameCmd,i++,g_sGpsInf.status);    writeDtCom(GpsFrameCmd,i++,g_sGpsInf.num_sats);
			writeDtCom(GpsFrameCmd,i++,g_sGpsInf.longitude); writeDtCom(GpsFrameCmd,i++,g_sGpsInf.latitude);   writeDtCom(GpsFrameCmd,i++,g_sGpsInf.alt); 
			writeDtCom(GpsFrameCmd,i++,g_sGpsInf.vx); 				writeDtCom(GpsFrameCmd,i++,g_sGpsInf.vy);  			 writeDtCom(GpsFrameCmd,i++,g_sGpsInf.vz);   // 速度 mm/s
			writeDtCom(GpsFrameCmd,i++,g_sGpsInf.ground_speed);       // 对地速度  mm/s 
			writeDtCom(GpsFrameCmd,i++,g_sGpsInf.ground_course_cd);   // 航向角 deg
			writeDtCom(GpsFrameCmd,i++,g_sGpsInf.hdop);               // 位置精度因子
			
		 // 地面坐标系下X,Y,Z位置和速度参数
			writeDtCom(GpsFrameCmd,i++,gfPositionX); 				writeDtCom(GpsFrameCmd,i++,gfPositionY);  			 writeDtCom(GpsFrameCmd,i++,gfPositionZ); 
			writeDtCom(GpsFrameCmd,i++,gfVelocityX); 				writeDtCom(GpsFrameCmd,i++,gfVelocityY);  			 writeDtCom(GpsFrameCmd,i++,gfVelocityZ); 
		}
		
	  if(bRemoter){
			 for(i=0; i < 10; i++) writeDtCom(RemoterFrameCmd,i,channel_raw[i]);
		}
}

// 启动组件测试指令帧
void ModelVar2DtCom_CompTest(uint16_t iCmd)    // 此时会置位g_iCurRunMode状态为1，表示组件测试状态
{
	  bool bNoTst = false;
		switch(iCmd){
			case 100:
				bICM42688 = bICM20602 = bDPS310 = bIST8310 = bGPS = bRemoter = true;
			  break;
			case 101:
				bICM42688 = true;
			  break;
			case 102:
				bICM20602 = true;
			  break;
			case 103:
				bDPS310 = true;
			  break;
			case 104:
				bIST8310 = true;
			  break;
			case 105:
				bGPS = true;
			  break;
			case 106:
				bRemoter = true;
			  break;
			default:
				bNoTst = true;
			  break;
		}
		
		// 设置相关组件的测试结果输出
		if(!bNoTst){
			initTWDtComOut(bICM42688,bICM20602,bDPS310,bIST8310,bGPS,bRemoter);
	
			// 各组件均工作为非校准状态
      g_bICM42688Calibing=g_bICM20602Calibing=g_bDPS310Calibing=g_bIst8310Calibing = false;  //
			g_iCurRunMode = 1;  // 表示组件测试状态
		}
}

// 启动校准程序的响应处理。
// 每一帧采集的原始数据是根据当前初始化initTWDtComOut设置标志，共用组件测试功能的串口输出函数CompTest2Gs来实现的。
void  ModelVar2DtCom_CompCalib(uint16_t iCmd)    // 此时会置位g_iCurRunMode状态为2，表示组件标定状态
{
	  bool bNoCalib = false;
		switch(iCmd){
			case 101:
			case 102:
				bICM42688 = true;				bICM20602 = true;
				g_bICM42688Calibing = true;   // 表示ICM42688为校准工作状态
				g_bICM20602Calibing = true;   // 表示ICM20602为校准工作状态
			  break;
			case 103:
				bDPS310 = true;
				g_bDPS310Calibing = true;   // 表示DPS310为校准工作状态
			  break;
			case 104:
				bIST8310 = true;
				g_bIst8310Calibing = true;   // 表示磁力计为校准工作状态
			  break;
			// GPS不用校准
			case 105:
			// 遥控器不用校准
			case 106:
			default:
				bNoCalib = true;
			  break;
		}
		
		// 设置相关组件的测试结果输出
		if(!bNoCalib){
			initTWDtComOut(bICM42688,bICM20602,bDPS310,bIST8310,bGPS,bRemoter);
			g_iCurRunMode = 2;  // 表示工作在组件标定状态
		}
}

  // 传地面站校准结果的响应处理
void DtCom2ModelVar_CalibRes(uint16_t iCmd)
{
	  uint8_t i = 0;
    switch(iCmd){
			case 101:
				initDtComOut(13,GetCalibResCmd);
			  writeDtCom(GetCalibResCmd,i++,101);
			  writeDtCom(GetCalibResCmd,i++,g_UavFcsParam.Icm42688.AccScale.x);
			  writeDtCom(GetCalibResCmd,i++,g_UavFcsParam.Icm42688.AccScale.y);
			  writeDtCom(GetCalibResCmd,i++,g_UavFcsParam.Icm42688.AccScale.z);
			  writeDtCom(GetCalibResCmd,i++,g_UavFcsParam.Icm42688.AccOffset.x);
			  writeDtCom(GetCalibResCmd,i++,g_UavFcsParam.Icm42688.AccOffset.y);
			  writeDtCom(GetCalibResCmd,i++,g_UavFcsParam.Icm42688.AccOffset.z);
			
			  writeDtCom(GetCalibResCmd,i++,g_UavFcsParam.Icm42688.GyroScale.x);
			  writeDtCom(GetCalibResCmd,i++,g_UavFcsParam.Icm42688.GyroScale.y);
			  writeDtCom(GetCalibResCmd,i++,g_UavFcsParam.Icm42688.GyroScale.z);
			  writeDtCom(GetCalibResCmd,i++,g_UavFcsParam.Icm42688.GyroOffset.x);
			  writeDtCom(GetCalibResCmd,i++,g_UavFcsParam.Icm42688.GyroOffset.y);
			  writeDtCom(GetCalibResCmd,i++,g_UavFcsParam.Icm42688.GyroOffset.z);
			  break;
			case 102:
				initDtComOut(13,GetCalibResCmd);
			  writeDtCom(GetCalibResCmd,i++,102);
			  writeDtCom(GetCalibResCmd,i++,g_UavFcsParam.Icm20602.AccScale.x);
			  writeDtCom(GetCalibResCmd,i++,g_UavFcsParam.Icm20602.AccScale.y);
			  writeDtCom(GetCalibResCmd,i++,g_UavFcsParam.Icm20602.AccScale.z);
			  writeDtCom(GetCalibResCmd,i++,g_UavFcsParam.Icm20602.AccOffset.x);
			  writeDtCom(GetCalibResCmd,i++,g_UavFcsParam.Icm20602.AccOffset.y);
			  writeDtCom(GetCalibResCmd,i++,g_UavFcsParam.Icm20602.AccOffset.z);
			
			  writeDtCom(GetCalibResCmd,i++,g_UavFcsParam.Icm20602.GyroScale.x);
			  writeDtCom(GetCalibResCmd,i++,g_UavFcsParam.Icm20602.GyroScale.y);
			  writeDtCom(GetCalibResCmd,i++,g_UavFcsParam.Icm20602.GyroScale.z);
			  writeDtCom(GetCalibResCmd,i++,g_UavFcsParam.Icm20602.GyroOffset.x);
			  writeDtCom(GetCalibResCmd,i++,g_UavFcsParam.Icm20602.GyroOffset.y);
			  writeDtCom(GetCalibResCmd,i++,g_UavFcsParam.Icm20602.GyroOffset.z);
			  break;
			case 103:
				initDtComOut(2,GetCalibResCmd);
			  writeDtCom(GetCalibResCmd,i++,103);
			  writeDtCom(GetCalibResCmd,i++,g_UavFcsParam.Dps310Offset);
			  break;
			case 104:
				initDtComOut(8,GetCalibResCmd);
			  writeDtCom(GetCalibResCmd,i++,104);
			  writeDtCom(GetCalibResCmd,i++,g_UavFcsParam.Ist8310Bias.x);
			  writeDtCom(GetCalibResCmd,i++,g_UavFcsParam.Ist8310Bias.y);
			  writeDtCom(GetCalibResCmd,i++,g_UavFcsParam.Ist8310Bias.z);
			  writeDtCom(GetCalibResCmd,i++,g_UavFcsParam.Ist8310Radius.x);
			  writeDtCom(GetCalibResCmd,i++,g_UavFcsParam.Ist8310Radius.y);
			  writeDtCom(GetCalibResCmd,i++,g_UavFcsParam.Ist8310Radius.z);
			  writeDtCom(GetCalibResCmd,i++,g_UavFcsParam.Ist8310Neg);
			  break;
			
			case 201:
				i = 2;
				g_UavFcsParam.Icm42688.AccScale.x  = readDtCom(GetCalibResCmd,i++);
			  g_UavFcsParam.Icm42688.AccScale.y  = readDtCom(GetCalibResCmd,i++);
			  g_UavFcsParam.Icm42688.AccScale.z  = readDtCom(GetCalibResCmd,i++);
				g_UavFcsParam.Icm42688.AccOffset.x = readDtCom(GetCalibResCmd,i++);
			  g_UavFcsParam.Icm42688.AccOffset.y = readDtCom(GetCalibResCmd,i++);
			  g_UavFcsParam.Icm42688.AccOffset.z = readDtCom(GetCalibResCmd,i++);
			
				g_UavFcsParam.Icm42688.GyroScale.x  = readDtCom(GetCalibResCmd,i++);
			  g_UavFcsParam.Icm42688.GyroScale.y  = readDtCom(GetCalibResCmd,i++);
			  g_UavFcsParam.Icm42688.GyroScale.z  = readDtCom(GetCalibResCmd,i++);
				g_UavFcsParam.Icm42688.GyroOffset.x = readDtCom(GetCalibResCmd,i++);
			  g_UavFcsParam.Icm42688.GyroOffset.y = readDtCom(GetCalibResCmd,i++);
			  g_UavFcsParam.Icm42688.GyroOffset.z = readDtCom(GetCalibResCmd,i++);
			  break;
			case 202:
				i = 2;
				g_UavFcsParam.Icm20602.AccScale.x  = readDtCom(GetCalibResCmd,i++);
			  g_UavFcsParam.Icm20602.AccScale.y  = readDtCom(GetCalibResCmd,i++);
			  g_UavFcsParam.Icm20602.AccScale.z  = readDtCom(GetCalibResCmd,i++);
				g_UavFcsParam.Icm20602.AccOffset.x = readDtCom(GetCalibResCmd,i++);
			  g_UavFcsParam.Icm20602.AccOffset.y = readDtCom(GetCalibResCmd,i++);
			  g_UavFcsParam.Icm20602.AccOffset.z = readDtCom(GetCalibResCmd,i++);
			
				g_UavFcsParam.Icm20602.GyroScale.x  = readDtCom(GetCalibResCmd,i++);
			  g_UavFcsParam.Icm20602.GyroScale.y  = readDtCom(GetCalibResCmd,i++);
			  g_UavFcsParam.Icm20602.GyroScale.z  = readDtCom(GetCalibResCmd,i++);
				g_UavFcsParam.Icm20602.GyroOffset.x = readDtCom(GetCalibResCmd,i++);
			  g_UavFcsParam.Icm20602.GyroOffset.y = readDtCom(GetCalibResCmd,i++);
			  g_UavFcsParam.Icm20602.GyroOffset.z = readDtCom(GetCalibResCmd,i++);
			  break;
			case 203:
				i = 2;
			  g_UavFcsParam.Dps310Offset = readDtCom(GetCalibResCmd,i++);
			  break;
			case 204:
				i = 2;
				g_UavFcsParam.Ist8310Bias.x = readDtCom(GetCalibResCmd,i++);
			  g_UavFcsParam.Ist8310Bias.y = readDtCom(GetCalibResCmd,i++);
			  g_UavFcsParam.Ist8310Bias.z = readDtCom(GetCalibResCmd,i++);
				g_UavFcsParam.Ist8310Radius.x = readDtCom(GetCalibResCmd,i++);
			  g_UavFcsParam.Ist8310Radius.y = readDtCom(GetCalibResCmd,i++);
			  g_UavFcsParam.Ist8310Radius.z = readDtCom(GetCalibResCmd,i++);
			  g_UavFcsParam.Ist8310Neg = readDtCom(GetCalibResCmd,i++);
			  break;
		}
		if((iCmd > 200) && (iCmd < 210)) SaveToFM25V01(&g_UavFcsParam);
}

  // 处理地面站配置参数
void  DtCom2ModelVar_SetupPara(uint16_t iCmd)
{
	  uint8_t i;
					// 从本机读取当前无人机的配置信息，并将其推入输出缓冲区
		SUavSetupPara pSUavSetupPara;
		size_t structSize = sizeof(SUavSetupPara);

	  // 地面站发来配置信息请求查询指令帧
	  if(iCmd == 101){
			// 处理无人机配置信息请求的回令帧，仅回一帧
//			uint8_t idxCurDtFrame = iFindDtOutFrameIdx(GetSetupParaCmd);
			
			float *fSetupPara = (float *)malloc(structSize);
		  memset(fSetupPara, 0, structSize);

			pSUavSetupPara = g_UavFcsParam.UavPara;
			
			// 从Flash读取产品配置参数。2025.11.17添加代码 by zqhkd
			TProductConfig cfg;
			if(FlashRead(&cfg)){
				pSUavSetupPara.UavId = cfg.ProductId;
				pSUavSetupPara.AuthorizationCode = cfg.UserAuthCode;
				pSUavSetupPara.CRCCode = cfg.KeyCode;
				pSUavSetupPara.bAuthorizedFlg = bChkProductAuth(cfg);
			}
			
			uint8_t iParaNum = structToFloatArray(fSetupPara,&pSUavSetupPara,structSize);
			
			// 初始化输出帧参数
			initDtComOut(iParaNum + 1,GetSetupParaCmd);
			// 发送响应帧到地面站
			writeDtCom(GetSetupParaCmd,0,101);
			for(uint8_t i = 0; i < iParaNum; i++) writeDtCom(GetSetupParaCmd,i+1,fSetupPara[i]);
			
			free(fSetupPara);  // 释放动态分配的内存
		}
		
	  if(iCmd == 201){
				// 处理无人机配置信息参数设置，并回令
			uint8_t idxDtInFrame = iFindDtInFrameIdx(GetSetupParaCmd);
			
			float fVal[20];
			
			for(i = 0; i < g_iDtInSignalNum[idxDtInFrame];i++){
				 fVal[i] =  readDtCom(GetSetupParaCmd,i + 2);
			}
			
			// 从地面站注入数据的缓冲区获取数据
			uint8_t iParaNum = floatArrayToStruct(&pSUavSetupPara,fVal,structSize);
			
			// 防止AuthorizationCode和CRCCode信息丢失，Modified by ZQingHua at 20250630		
			//    由于用户配置信息和AuthorizationCode、CRCCode在地面站是有两个配置页面，但它们均在一帧信息内，可能要区别处理。
//			uint16_t iAuthorizationCode = g_UavFcsParam.UavPara.AuthorizationCode;
//			uint16_t iCRCCode = g_UavFcsParam.UavPara.CRCCode;
//			g_UavFcsParam.UavPara = pSUavSetupPara;  // 如果结构体中包含位字段（bit-fields）或其他不能保证赋值操作的成员，直接赋值可能会出现问题。
			// 结构体变量赋值采用如下操作可以避免以上问题。以下操作会将破坏AuthorizationCode和CRCCode
			memcpy(&g_UavFcsParam.UavPara,&pSUavSetupPara,sizeof(SUavSetupPara));

		  // 将产品配置信息保存到flash中，modified by zqhkd
			TProductConfig cfg;
		  cfg.ProductId = g_UavFcsParam.UavPara.UavId;
		  cfg.UserAuthCode = g_UavFcsParam.UavPara.AuthorizationCode;
		  cfg.KeyCode = g_UavFcsParam.UavPara.CRCCode;
			FlashWrite(&cfg);
			
	// 如果当前是未授权状态，则重新计算授权标志，确保更改后授权立即生效。Modified by zqh 20251115
		 pSUavSetupPara.bAuthorizedFlg = bChkProductAuth(cfg);  // 授权标志
		 // 将结构体内容重新拷贝会fVal。Modified by zqh 20251115
		 iParaNum = structToFloatArray(fVal,&pSUavSetupPara,structSize);
			
			// 保存到FRAM25V01中
			g_UavFcsParam.UavPara.bAuthorizedFlg = pSUavSetupPara.bAuthorizedFlg;
			SaveToFM25V01(&g_UavFcsParam);
			
			// 回传参数至地面站
			uint8_t idxDtOutFrame = iFindDtOutFrameIdx(GetSetupParaCmd);
			g_iDtOutSignalNum[idxDtOutFrame] =  g_iDtInSignalNum[idxDtInFrame];

			// 发送回令帧到地面站
			initDtComOut(iParaNum + 1,GetSetupParaCmd);  // 初始化输出帧参数
			writeDtCom(GetSetupParaCmd,0,101);
			for(uint8_t i = 0; i < iParaNum; i++) writeDtCom(GetSetupParaCmd,i+1,fVal[i]);
		}
}

// 恢复地面飞行状态
void  DtCom2ModelVar_SetFlySts(uint16_t iCmd)
{
		// 各组件均工作为非校准状态
		g_bICM42688Calibing = g_bICM20602Calibing = g_bDPS310Calibing = g_bIst8310Calibing = false;  //
    bICM42688= bICM20602=bDPS310=bIST8310=bGPS=bRemoter=false;
	
   // 清除测试组件输出
	  vCloseTestCalibUsedFlg();
		g_iCurRunMode = 0;  // 表示进入待飞行状态		  
}

#define SysFunCmdNum 6
void (*ExeDtCom2ModelVarSysCmd[SysFunCmdNum])(uint16_t iCmd);

void DtCom2ModelVar_SysCmdGet(uint8_t idxFrameNo)
{
//	  uint8_t iFrameTypeCode[SysFunCmdNum] = {0xF1,0xF2,0xF3,0xF4,0xF5,0xFA};
	  uint8_t iFrameTypeCode[SysFunCmdNum] = {GetUserModelInfCmd,GetCompTestCmd,GetCompCalibCmd,GetCalibResCmd,GetSetupParaCmd,GetSetFlyStsCmd};
		ExeDtCom2ModelVarSysCmd[0] = ModelVar2DtCom_ModelVer;
		ExeDtCom2ModelVarSysCmd[1] = ModelVar2DtCom_CompTest;
		ExeDtCom2ModelVarSysCmd[2] = ModelVar2DtCom_CompCalib;
		ExeDtCom2ModelVarSysCmd[3] = DtCom2ModelVar_CalibRes;
		ExeDtCom2ModelVarSysCmd[4] = DtCom2ModelVar_SetupPara;
		ExeDtCom2ModelVarSysCmd[5] = DtCom2ModelVar_SetFlySts;
		
		uint16_t iGetSysCmd = (uint16_t) readDtCom(iFrameTypeCode[idxFrameNo] ,1);
		if(iGetSysCmd >= 100){
			 clrDtComInVal(iFrameTypeCode[idxFrameNo] ,1);
//			 bICM42688= bICM20602=bDPS310=bIST8310=bGPS=bRemoter=false;
			 ExeDtCom2ModelVarSysCmd[idxFrameNo](iGetSysCmd);
		}
}

// 处理地面站注入到飞控组件的指令帧
void ProGs2FcsCmdFrame(void)
{
	 // 处理地面站注入指令，指令包括：0xF1--0xF5、0xFA
  // 检查相应指令帧是否有效，并进行响应处理
	 for(uint8_t i = 0; i < SysFunCmdNum; i++){
		  DtCom2ModelVar_SysCmdGet(i);
	 }
}

// 为地面站系统级参数获取指令帧，做好初始化工作
void initGs2FcsCmdFrame(void)
{
	// 初始化用户模型文件版本管理获取指令帧, 帧类型码0xF1
	  initDtComIn(1,GetUserModelInfCmd,0);    // 请求获取用户模型信息指令码，只需一个输入数
//	  initUserModelInfDtComOut();   // 用户模型信息帧内容动态创建
	
	 // 初始化组件测试指令帧
	  initDtComIn(1,GetCompTestCmd,0);
	 // 组件测试结果输出的初始化处理。
	
	// 初始化组件校准指令帧
	  initDtComIn(1,GetCompCalibCmd,0);
	// 初始化请求返回组件校准结果指令帧
	  initDtComIn(1,GetCalibResCmd,0);
	
  // 	计算无人机配置参数的等效浮点数数量
	  uint8_t numFloats = iCalFloatNums(sizeof(SUavSetupPara)); // 向上取整

	// 初始化请求返回无人机配置参数指令帧
	  initDtComIn(numFloats + 1,GetSetupParaCmd,0);   // DtComIn数量 = 指令码 + 等效浮点数个数
	
	// 初始化恢复无人机飞行状态指令帧
	  initDtComIn(1,GetSetFlyStsCmd,0);
}	
/************************ (C) COPYRIGHT ACG co. *****END OF FILE****/
