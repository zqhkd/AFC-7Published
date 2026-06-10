/******************** (C) COPYRIGHT 2021 AFC Tech Co.*************************
 * 作    者  ： 曾庆华
 * 文 件 名  ：AFCSimuCom.c
 * 版    本  ：
 *        AFC-5V5.02.230920: 根据仲栋ConnectPC.c进行全面升级更改 
 *             
 * 描    述  ：数传模块串口接口函数
 * 
 * 官    网  ：www.acecreator.com
 * 淘    宝  ：acecreator.taobao.com
 * 公 众 号  ：飞行控制与仿真
 *
*****************************************************************************/
#include "AFCSimuCom.h"
#include "ACGCommonAPI.h"
#include "AFCGlobalVar.h"

#define SimuComChkMode       SUM_CHECK
//#define BaseInFrameType    0x70
//#define BaseOutFrameType   0xAA      // 

//#define MaxResponseNum2InSimuFrame  3   // 对于有串口中断输入时的回复帧次数

#define MaxSimuInFrameTypeNum   10
#define MaxSimuOutFrameTypeNum  20
#define MaxSignalNumOfSimuFrame 50         // 假设串口波特率为115200bps, 单帧1ms约传输11byte, 则10ms约传输110byte, 约27个浮点数，因此需要注意传输数据数量与波特率的匹配性


// 外部可使用全局变量
uint8_t  g_sSimuComTxBuf[SimuCom_MAX_Tx_SIZE]; 		// 单帧DMA发送缓冲,  最大SimuCom_MAX_Tx_SIZE字节
uint8_t  g_sSimuComRxBuf[SimuCom_MAX_Rx_SIZE]; 		// 单帧串口中断接收缓冲,  最大SimuCom_MAX_Rx_SIZE字节
int8_t  g_curSimuComRcvLen;

TRingBuffer *lpSimuRingBuff;

uint8_t g_iMaxSimuInFrameNum = 0;
uint8_t g_iResSimuFrameNum[MaxSimuInFrameTypeNum];
uint8_t g_iSimuInFrameType[MaxSimuInFrameTypeNum];   // 输出帧的帧类型码或帧序列码
int8_t g_iSimuInSignalNum[MaxSimuInFrameTypeNum];
bool g_bSimuInFrameFlg[MaxSimuInFrameTypeNum];
float g_fSimuInData[MaxSimuInFrameTypeNum][MaxSignalNumOfSimuFrame];     // 模型串口输入变量

uint8_t g_iMaxSimuOutFrameNum = 0;
uint8_t g_iSimuOutFrameType[MaxSimuOutFrameTypeNum];   // 输出帧的帧类型码或帧序列码
int8_t g_iSimuOutSignalNum[MaxSimuOutFrameTypeNum];
float g_fSimuOutData[MaxSimuOutFrameTypeNum][MaxSignalNumOfSimuFrame];    // 通过SimuCom串口输出的参数

uint8_t send_same_simupack_times = 0;
uint8_t resSimuInFrameType = 0;  //是否发送相同数据包标志位

// 进行SimuCom多帧多周期处理, 最多允许一个仿真周期20个帧排队发送
#define MAX_SIMUQUEUE_SIZE  20
bool g_bUsedOfSimuCom = false;
TRingBuffer *lpSimuOutQueue;

void ClrSimuComSignalNum(void)
{
	uint8_t i;
	for(i = 0; i < MaxSimuInFrameTypeNum; i++)  g_iSimuInSignalNum[i]  = -1;
	for(i = 0; i < MaxSimuOutFrameTypeNum; i++) g_iSimuOutSignalNum[i] = -1;
}

// 初始化模型输出串口
void initSimuComOut(uint8_t iSigNum,uint8_t iFrameType)
{
	  // 先检查当前帧iFrameType是否在输出帧队列中
	  uint8_t iCurFrameNo = iChkFrameTypeIdx(iFrameType, g_iMaxSimuOutFrameNum,g_iSimuOutFrameType);
	  if(iCurFrameNo == g_iMaxSimuOutFrameNum) g_iMaxSimuOutFrameNum++;   // 输出帧数量, 如果是在队列尾部添加帧类型码，则增加帧数量
	
	  if(iSigNum > MaxSignalNumOfSimuFrame) iSigNum = MaxSignalNumOfSimuFrame;
	  g_iSimuOutSignalNum[iCurFrameNo] = iSigNum;
	  g_iSimuOutFrameType[iCurFrameNo] = iFrameType;

	  for(uint8_t i = 0; i < iSigNum; i++) g_fSimuOutData[iCurFrameNo][i] = 0.f;
	  
	  g_bUsedOfSimuCom = g_bSimuComOutSucessful = true;
	  if(lpSimuOutQueue==NULL) lpSimuOutQueue = InitRingBuffer(lpSimuOutQueue,MAX_SIMUQUEUE_SIZE);
}

// 初始化DMA模式的SimuCom(huart5)。最关键问题是打开空闲中断
void InitSimuComDMA(void)
{
	  lpSimuRingBuff = InitRingBuffer(lpSimuRingBuff,SimuCom_MAX_Rx_SIZE*2);
	
	  InitComDMA(&SimuCom,g_sSimuComRxBuf,SimuCom_MAX_Rx_SIZE);
}

// 以下两个函数initReadSimuCom、getSimuComVal是和readSimuComAPI.c及.tlc程序配合使用的函数
void initSimuComIn(uint8_t iSigNum,uint8_t iFrameType,uint8_t iResFrameNum)
{
	  // 先检查当前帧iFrameType是否在输入帧队列中，返回索引。没有就返回输入帧数量值
	  uint8_t iCurFrameNo = iChkFrameTypeIdx(iFrameType, g_iMaxSimuInFrameNum,g_iSimuInFrameType);
	  if(iCurFrameNo==g_iMaxSimuInFrameNum) g_iMaxSimuInFrameNum++;   // 输入帧数量, 如果是在队列尾部添加帧类型码，则增加帧数量
	  if(iSigNum > MaxSignalNumOfSimuFrame) iSigNum = MaxSignalNumOfSimuFrame;
	  g_iSimuInSignalNum[iCurFrameNo] = iSigNum;
	  g_iSimuInFrameType[iCurFrameNo] = iFrameType;
	  g_bSimuInFrameFlg[iCurFrameNo] = false;
	  g_iResSimuFrameNum[iCurFrameNo] = iResFrameNum;
	
	// 初始化数传串口接收DMA
	  if(!bInitSimuComIn){
	    if(lpSimuRingBuff==NULL) InitSimuComDMA();    // V4.02.220422：将该初始化程序从main函数中移至此处，确保当SimuCom打开时才使用。
		  
	  // V5.03.240226添加对SimuComIn数据的初始化赋0操作
			for(uint8_t i = 0; i < MaxSimuInFrameTypeNum; i++)	   
				for(uint8_t j =0; j < MaxSignalNumOfSimuFrame; j++)
			       g_fSimuInData[i][j] = 0.0f;
    }
			
		bInitSimuComIn = true;
}

int iFindSimuOutFrameIdx(uint8_t iFrameType)
{
	  return iFindIdBuffIdx(iFrameType,g_iMaxSimuOutFrameNum,g_iSimuOutFrameType);
}

int iFindSimuInFrameIdx(uint8_t iFrameType)
{
		return iFindIdBuffIdx(iFrameType,g_iMaxSimuInFrameNum,g_iSimuInFrameType);
}

void dma_sendbuff_simcom(uint16_t len)
{
 //等待上一次的数据发送完毕
	uint16_t i = 0;
  while(HAL_DMA_GetState(SimuCom.hdmatx) == HAL_DMA_STATE_BUSY){
	   if(i++ > 1000) break;   // 超时处理
	};  // 这种处理方式容易导致控制周期超时，甚至死机情况
	
	if(i<1000){
	/* 2020.10.24 防止出现因状态未被复位，导致无法发送的情况(过去由HAL_UART_DMAStop()关闭，现在。。。) */
	  if(SimuCom.gState != HAL_UART_STATE_READY)	HAL_UART_DMAStop(&SimuCom);
	
    /* 关闭DMA */
 //   __HAL_DMA_DISABLE(SimuCom.hdmatx);
		
    //开始发送数据
    HAL_UART_Transmit_DMA(&SimuCom,g_sSimuComTxBuf,len);
		
		g_bSimuComOutSucessful = false;
  }
}
	
// 通过模型端口发送信息，DMA方式
void DataDMA2SimuCom(uint8_t idxCurFrame)
{
	  uint16_t len;
		THeaderFrame pHeaderFrame;
		__HAL_UART_DISABLE_IT(&SimuCom, UART_IT_IDLE);
	
		pHeaderFrame.iFrameTick = g_sRealTimeCount.fcsTime;
		pHeaderFrame.idSender = getCurUAVId();
//		pHeaderFrame.idReceiver = ID_OF_GCS;
	
	  if((resSimuInFrameType!=0)&&(send_same_simupack_times>0)) {    // 如果从串口接收到数据，且要求回令时，则暂停正常发送，而将刚刚收到数据发送指定次数
				pHeaderFrame.iFrameType = resSimuInFrameType;
			  uint8_t idxFrame = iFindSimuInFrameIdx(resSimuInFrameType);
				len = iPackData2Hex(pHeaderFrame,g_iSimuInSignalNum[idxFrame] + 1,g_fSimuInData[idxFrame],g_sSimuComTxBuf,SUM_CHECK);   // g_iSimuInSignalNum包括时标和信号数量，所以+1
			
				if(send_same_simupack_times > 1) send_same_simupack_times--;
		    else resSimuInFrameType = 0;
		}
		else{
			pHeaderFrame.iFrameType = g_iSimuOutFrameType[idxCurFrame];  // 发送给SimuCom的帧类型码
			
// 原来为虚函数，需用它才能将外部虚函数实化。
			len = iPackData2Hex(pHeaderFrame,g_iSimuOutSignalNum[idxCurFrame],g_fSimuOutData[idxCurFrame],g_sSimuComTxBuf,SUM_CHECK); 
	 }
		
	  dma_sendbuff_simcom(len);    // DMA发送缓冲区一定要使用全局变量，否则极易引起内存泄漏
		__HAL_UART_ENABLE_IT(&SimuCom, UART_IT_IDLE);
}

// 传递指定参数给模型串口
void writeSimuCom(uint8_t iFrameType,uint8_t iChannel,double fVal)
{
	 static int iSigNum = 0;
     g_bSimuComOutBufIsUsing = true;
	 uint8_t idxCurSimuFrame = iFindSimuOutFrameIdx(iFrameType);
	 if(g_iSimuOutSignalNum[idxCurSimuFrame] >=0){
		 if(iChannel < g_iSimuOutSignalNum[idxCurSimuFrame]){
				g_fSimuOutData[idxCurSimuFrame][iChannel] = fVal;
				iSigNum++;
				if(iSigNum >= g_iSimuOutSignalNum[idxCurSimuFrame]){
					// 当前帧的最后一个变量，则将当前帧数据打包输出，且将帧序号清0
					RingBufferPut(lpSimuOutQueue,&idxCurSimuFrame,1);
					iSigNum = 0;
				    g_bSimuComOutBufIsUsing = false;
				}
		 }
	 }
}

// 从环形缓冲区中提取帧内容，解析SimuCom的传送数据
void vRcvSimuComInfTask(void)
{
    static uint8_t iState = 1;                // 状态机初始状态
    static uint8_t pBuff[SimuCom_MAX_Rx_SIZE]; // 帧数据缓存
    static uint16_t s_iFrameLen = 0;          // 缓存已解析的帧长度
    static bool bExitFlg = false;              // 退出循环标志
	  uint16_t needed;

    while (!bExitFlg && !RingBufferEmpty(lpSimuRingBuff)) 
    {
        switch (iState) 
        {
            /*---------------------------------------
             * 状态1：帧头同步
             * 功能：寻找完整帧头（0xAA 0x55）
             *---------------------------------------*/
            case 1: 
                // 至少需要2字节才能检查帧头
                if (GetRingBufferLen(lpSimuRingBuff) >= 2) {
                    uint8_t header[2];
                    RingBufferPeek(lpSimuRingBuff, header, 2); // 查看但不取出数据

                    // 检查帧头是否匹配
                    if ((header[0] == (FRAME_HEADER_ID & 0xFF)) && 
                        (header[1] == ((FRAME_HEADER_ID >> 8) & 0xFF))) {
                        // 帧头匹配，取出并保存到pBuff
                        RingBufferGet(lpSimuRingBuff, pBuff, 2);
                        iState = 3; // 跳转到帧长度处理
                    } 
                    else {
                        // 帧头不匹配，丢弃第一个字节继续寻找
                        RingBufferGet(lpSimuRingBuff, NULL, 1);
                    }
                }
                break;
            /*---------------------------------------
             * 状态3：帧长度处理（可重入）
             * 功能：读取帧长度，并等待足够负载数据
             *---------------------------------------*/
            case 3: 
                // 首次进入需读取帧长度
                if (s_iFrameLen == 0) {
                    if (GetRingBufferLen(lpSimuRingBuff) >= 2) {
                        // 读取帧长度（2字节）
                        RingBufferGet(lpSimuRingBuff, pBuff + 2, 2);
                        s_iFrameLen = pBuff[2] | (pBuff[3] << 8);

                        // 帧长度合法性检查
                        if (s_iFrameLen < 4 || s_iFrameLen > SimuCom_MAX_Rx_SIZE) {
                            // 非法长度，重置状态机
                            s_iFrameLen = 0;
                            iState = 1;
                            break;
                        }
                    } 
                    else {
                        // 长度数据不足，保持状态3等待
                        break;
                    }
                }

                // 计算剩余需要的数据量
                needed = s_iFrameLen - 4;
                if (GetRingBufferLen(lpSimuRingBuff) >= needed) {
                    // 数据足够，读取剩余负载
                    RingBufferGet(lpSimuRingBuff, pBuff + 4, needed);
                    iState = 4; // 跳转到完整帧处理
                } 
                else {
                    // 数据不足，退出循环等待下次调用
                    bExitFlg = true;
                }
								break;
            /*---------------------------------------
             * 状态4：完整帧处理
             * 功能：校验并处理完整数据帧
             *---------------------------------------*/
            case 4: 
                // 校验帧完整性（CRC或其他校验）
                if (bChkFrameValid(pBuff, SimuComChkMode, s_iFrameLen)) {
                    // 查找帧类型索引
                    int idxFrame = iFindSimuInFrameIdx(pBuff[FrameTypePos]);
                    if (idxFrame >= 0) {
                        // 解析数据负载
                        uint8_t iVar = iUnPackHex2Data(pBuff, s_iFrameLen, g_fSimuInData[idxFrame], SimuComChkMode);
                        
                        // 更新信号数量和帧标志
                        g_iSimuInSignalNum[idxFrame] = iVar;
                        g_bSimuInFrameFlg[idxFrame] = true;

                        // 触发响应逻辑（如回复指定次数）
                        send_same_simupack_times = g_iResSimuFrameNum[idxFrame];
                        resSimuInFrameType = pBuff[FrameTypePos];
                    }
                }

                // 强制重置状态机（无论校验是否通过）
                s_iFrameLen = 0;
                iState = 1;
                bExitFlg = true; // 退出循环，防止重复处理
                break;
            default:
                iState = 1;
                break;
        }
    }

    // 复位退出标志，为下一次任务调用做准备
    bExitFlg = false;
}

// 帧解析处理函数
void ProcessSimuFrame(uint8_t *frame, int iCurFrameLen)
{
    // 实际业务处理逻辑
    int idxFrame = iFindSimuInFrameIdx(frame[FrameTypePos]);
    if (idxFrame >= 0) {
        uint8_t iVar = iUnPackHex2Data(frame, iCurFrameLen, g_fSimuInData[idxFrame], SimuComChkMode);
				// 更新信号数量和帧标志
				g_iSimuInSignalNum[idxFrame] = iVar;
				g_bSimuInFrameFlg[idxFrame] = true;

			// 触发响应逻辑（如回复指定次数）
				send_same_simupack_times = g_iResSimuFrameNum[idxFrame];
				resSimuInFrameType = frame[FrameTypePos];
    }
}

bool bChkSimuInFrameLenValid(int iCurFrameLen)
{
	  bool bFlg = false;
	  uint8_t iVarNum = iCalFloatNums(iCurFrameLen - (sizeof(THeaderFrame) + SimuComChkMode + 1 ));    // 获取传输变量个数 m = (N - 12)/4, 取上边界
	
	  for(int i = 0; i < MaxSimuInFrameTypeNum;i++){
			if(g_iSimuInSignalNum[i] == iVarNum){
				bFlg = true;
				break;
			}
		}
		return bFlg;
}

void ParseSimuComFrame(void)
{
    static uint8_t lastValidFrame[SimuCom_MAX_Rx_SIZE];
    uint8_t tempBuffer[SimuCom_MAX_Rx_SIZE];
    int availableLen,iCurFrameLen;

    // 持续处理直到缓冲区空或找到有效帧
    while ((availableLen = GetRingBufferLen(lpSimuRingBuff)) >= 4) 
    {
        // 1. 预读帧头
        uint8_t header[4];
        RingBufferPeek(lpSimuRingBuff, header, 4);

        // 2. 帧头不匹配则丢弃1字节继续查找
            // 检查帧头是否匹配
				if (!(header[0] == (FRAME_HEADER_ID & 0xFF) && 
						header[1] == ((FRAME_HEADER_ID >> 8) & 0xFF))) {
            RingBufferGet(lpSimuRingBuff, NULL, 1);
            continue;
        }
				
        // 3. 检查是否为有效帧长度，不是则丢弃当前帧头继续查找
				iCurFrameLen = header[2] + (header[3] << 8);
				if(!bChkSimuInFrameLenValid(iCurFrameLen)){
           RingBufferGet(lpSimuRingBuff, NULL, 2);  // 表示当前的帧头不是真正帧头
					 continue;
				}

        // 4. 检查完整帧可用性
        if (availableLen < iCurFrameLen) {
            break; // 数据不足等待下次
        }

        // 5. 原子化读取帧数据
        RingBufferGet(lpSimuRingBuff, tempBuffer, iCurFrameLen);

        // 6. 校验帧有效性
        if (bChkFrameValid(tempBuffer, SimuComChkMode, iCurFrameLen)) 
        {
            memcpy(lastValidFrame, tempBuffer, iCurFrameLen);
            ProcessSimuFrame(lastValidFrame,iCurFrameLen);
            return; // 成功处理一帧后立即退出
        }
        else 
        {
            // 校验失败，丢弃帧头重新同步
            RingBufferGet(lpSimuRingBuff, NULL, 1); 
            continue;
        }
    }
}

// 处理SimuCom的空闲中断DMA：接收SimuCom字符串
void ProSimuComRcvIRQ(void)
{  
	  uint32_t isrflags   = READ_REG(SimuCom.Instance->ISR);
    uint32_t cr1its     = READ_REG(SimuCom.Instance->CR1);
    bool bClearOverFlag = true;
	
    if(((isrflags & USART_ISR_IDLE) != RESET) && ((cr1its & USART_CR1_IDLEIE) != RESET) && ((isrflags & USART_ISR_ORE) != USART_ISR_ORE)){
	    __HAL_UART_CLEAR_IDLEFLAG(&SimuCom);
	    uint32_t _len_dmarev = SimuCom_MAX_Rx_SIZE - __HAL_DMA_GET_COUNTER(SimuCom.hdmarx);
	    if(_len_dmarev){
//            HAL_UART_DMAStopRx(&SimuCom);
			/* 这里以前使用HAL_UART_DMAStop(huart)，但这个函数会导致TX的DMA被关闭，小概率DMA发送会丢数据 */
			/* 2021.10.24 修改为HAL_UART_AbortReceive(huart)，对应DMA发送也需要做一些处理 */
			HAL_UART_AbortReceive(&SimuCom);
			
			if(bInitSimuComIn){
				// 向环形缓冲区写数据
				RingBufferPut(lpSimuRingBuff,g_sSimuComRxBuf,_len_dmarev);

				HAL_UART_Receive_DMA(&SimuCom, g_sSimuComRxBuf, SimuCom_MAX_Rx_SIZE); 
			}
				__HAL_UART_DISABLE_IT(&SimuCom, UART_IT_ERR);
				__HAL_UART_DISABLE_IT(&SimuCom, UART_IT_PE);
			
        // 不用清除溢出标志，其它情况都需要清除			
		    bClearOverFlag = false;
	    }
    }
	if(bClearOverFlag){
	   READ_REG(SimuCom.Instance->ISR);
	   READ_REG(SimuCom.Instance->RDR);
	   __HAL_UART_CLEAR_OREFLAG(&SimuCom);
//	   __HAL_UART_CLEAR_IDLEFLAG(&SimuCom);
	}
}

// 从模型串口SimuCom读取变量到模型端口(ReadMC模块)，供用户模型Simulink程序使用。
double readSimuCom(uint8_t idFrameType,uint8_t iChanel)
{
	  double fVal = 0x00;
	  static int iSigNum = 0;
	  static bool bFirstUpdateDataFlag = true;
	
	// 首次调用进入，需要读取循环缓冲区帧内容并进行解析，以便后续使用
	  if(bFirstUpdateDataFlag){
			ParseSimuComFrame();
			bFirstUpdateDataFlag = false; 
		}
	  
		int idxFrame = iFindSimuInFrameIdx(idFrameType);
//	  if(g_bSimuInFrameFlg[idxFrame]){    // 用该方法的话，读取一次后数据会回0
		if(idxFrame >= 0 ){    
				fVal = g_fSimuInData[idxFrame][iChanel];
				iSigNum++;
				if(iSigNum >= g_iSimuInSignalNum[idxFrame]+1){          // 
					 g_bSimuInFrameFlg[idxFrame] = false;
					 iSigNum = 0;
					
			// 下次调用时确保进行循环缓冲数据读取与解析
					 bFirstUpdateDataFlag = true;
				}
		}
	  return fVal;
}

// 从环形缓冲区中提取帧内容，解析SimuCom的传送数据。
//   以下代码为V5.05.250314版本以前的SimuCom串口接收解析程序
void vRcvSimuComInfTask250327(void)
{
	static uint8_t iState = 1;
	static uint8_t pBuff[SimuCom_MAX_Rx_SIZE];
	static bool bExitFlg = false;
	uint16_t iFrameLen,iBuffLen;

	while(!(bExitFlg || RingBufferEmpty(lpSimuRingBuff)))		// 判断环形缓冲区是否为空,或者是退出标志？
	{
	    // 非空，则开始处理数据
		switch (iState)
    {
			// 开始判断帧头第1字节
      case 1:
				RingBufferGet(lpSimuRingBuff,pBuff,1);
				if(pBuff[0] == (FRAME_HEADER_ID&0xff)){
					iState = 2;
				}
      	break;
			// 开始判断帧头第2字节
      case 2:
				RingBufferGet(lpSimuRingBuff,pBuff + 1,1);
				if(pBuff[1] == ((FRAME_HEADER_ID&0xff00)>>8)){
					iState = 3;
				}
				else iState = 1;   // 如果第2字节不是帧头，则返回至状态0，重新开始寻找帧头
      	break;
			case 3:
			// 读取帧长度
				if(GetRingBufferLen(lpSimuRingBuff) >= 2){   // 不到2个字节时，则需等待第二次进入
					RingBufferGet(lpSimuRingBuff,pBuff+2,2);
					iFrameLen = pBuff[FrameLengthPos] + (pBuff[FrameLengthPos + 1] << 8);
					iBuffLen = GetRingBufferLen(lpSimuRingBuff) + 4;           //  AFC-5V5.03.240522更改帧长度异常注入的问题
					if((iFrameLen < SimuCom_MAX_Rx_SIZE) && (iFrameLen <= iBuffLen)){        //  AFC-5V5.03.240522更改帧长度异常注入的问题，添加iFrameLen <= iBuffLen条件
						iState = 4;
					}
					else{
					   iState = 1;  // 如果帧长度不对，抛弃该帧
					   bExitFlg = true;
				    }
				}
				break;
			case 4:
			// 确保环形缓冲区有iFrameLen - 4个数据，表示一个完整数据帧
				if(GetRingBufferLen(lpSimuRingBuff) >= iFrameLen - 4){
					RingBufferGet(lpSimuRingBuff, pBuff + 4, iFrameLen - 4);
		
			  int idxFrame = iFindSimuInFrameIdx(pBuff[FrameTypePos]);
					if((bChkFrameValid(pBuff,SimuComChkMode,iFrameLen))&&(idxFrame >= 0)){
						 uint8_t iVar = iUnPackHex2Data(pBuff,iFrameLen,g_fSimuInData[idxFrame],SimuComChkMode);
						
						// V5.05.240830: 动态调整帧队列中信号个数
				         g_iSimuInSignalNum[idxFrame] = iVar;
						 
						 g_bSimuInFrameFlg[idxFrame] = true;
						 
						 // 收到数据后，回复装订指定次数的帧
						 send_same_simupack_times = g_iResSimuFrameNum[idxFrame];
						 resSimuInFrameType = pBuff[FrameTypePos];
					}
					iState = 1;				  bExitFlg = true;
				}
				break;
      default:
     		break;
     }
	}
	bExitFlg = false;
}
/************************ (C) COPYRIGHT ACG co. *****END OF FILE****/
