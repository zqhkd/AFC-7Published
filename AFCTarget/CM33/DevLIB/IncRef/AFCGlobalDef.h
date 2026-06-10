/******************** (C) COPYRIGHT 2019 ACG Tech Co.*************************
 * 作    者： 曾庆华
 * 文 件 名：AFCGlobalVar.h
 * 描    述：AFC-5快速原型组件程序常数、结构类型等的定义文件
 * 版    本：
 * 官    网：www.acecreator.com
 * 淘    宝：acecreator.taobao.com
 * 公 众 号：无人飞行控制
*****************************************************************************/
#ifndef __AFC_GlobalDef_H__
#define __AFC_GlobalDef_H__
#include "stdint.h"
#include "stdbool.h"

//#define AFC_SYSTEM_MAIN_VER   5.05
//#define AFC_SYSTEM_DATE_VER   240913

// AFC-5V5.02.231107: AFC-5A仅捕获高电平持续脉冲数，tim2的预分频为3-1
#define PWM_IN_PERIOD_400HZ  106666.6667    // 128*10e6/(400*3)
#define PWM_IN_PERIOD_50HZ   853333.3333    // 128*10e6/(50*3)

#define CUR_PWM_PERIOD_COUNT    PWM_IN_PERIOD_400HZ

// 定义串口数据类型
#define CioCom   huart1       // simulink仿真环境调试串口
#define SimuCom  huart1      //  SimuCom与CioCom复用usart1

#define DtCom    huart2       // 数传串口
#define SBusCom  huart3       // SBus串口
#define GpuCom   huart4       // 智能模块GPU串口
#define GpsCom   huart6        // GPS串口, 兼容M10和NANO GNSS
#define TofCom   huart8       // 激光测距TofCom，标准RS232

#define IdxExtUart4  0      // Uart4的索引值为0  
#define IdxExtUart5  1      // Uart5的索引值为1  
#define IdxExtUart6  2      // Uart6的索引值为2  
#define IdxExtUart7  3      // Uart7的索引值为3  
#define IdxExtUart8  4      // Uart8的索引值为4  

#define NumOfSBusChannel  16  // 遥控器的总通道数
#define SENSOR_NUM        10

#define MAX_SLIDE_WIN_LEN 20   // 滑动滤波器最大采样数据点数，AFCToolbox中已设置最大值，0表示不开启滑动滤波

// 自定义串口帧帧头信息
#define FRAME_HEADER_ID          0xdaa5    // 定义串口帧帧头标识码
#define FRAME_HEADER_LENGTH      11        // 帧头长度为11

// 帧数据位置定义
#define FrameHeaderPos   0                    // 帧头
#define FrameLengthPos   2                    // 帧长度
#define FrameTickPos     4                     // 帧序号
#define FrameSenderPos   8                    // 发送方节点号
#define FrameReceverPos  9                    // 接收方节点号
#define FrameTypePos    10                    // 帧类型
#define FrameParaPos    FRAME_HEADER_LENGTH  // 帧指令类型

#define FrameParaNumPos   12   // 帧参数个数所在位置序号

typedef enum{
	 SUM_CHECK     = 0,          // 累加和校验
	 CRC_CHECK     = 1           // CRC校验
}TFrameCheckMode;

// 串口节点地址标识码
#define ID_Of_FCS            0x10
#define ID_OF_GPS            0x11
#define ID_OF_SIM            0x30
#define ID_OF_FPGA           0x88

#define ID_OF_TestComPort    0x11
#define ID_OF_SimuComPort    0x12
#define ID_OF_GCS            0x20
#define ID_OF_VideoComPort   0x21

// 遥控器定义
#define WFLY_ETS6S    1       // 天地飞遥控器ETS6S     
#define JUMPER_T20    2       // 开源遥控器T20

// 无人机机架定义
#define F550_UAV     1       // F550大旋翼机机架     
#define K80A_UAV     2       // K80A小旋翼无人机机架
#define K80B_UAV     3       // K80B小旋翼无人机机架(K80机架+自研机壳)
#define HS620_UAV    4       // HS620昊舜620旋翼机机架     

#define SURFERX8_FIX    50      // 冲浪者固定翼
#define HALO190_FIX     51      // 光环固定翼

#define USER_UAV     99         // 用户自定义机架，缺省采用F550_UAV参数，用户可进行更改

// 控制器板选择：AFC5A_BOARD、AFC5B_BOARD
#define AFC5A_BOARD  1     // 
#define AFC5B_BOARD  2
#define AFC6_BOARD   3

#define ATTITDE_STAB_MODE    0x01   // 无人机姿态稳定模式
#define ALTITUDE_HOLD_MODE   0x02   // 无人机定高飞行模式
#define POS_NAVI_MODE        0x03   // 无人机位置导航模式

#define RAD_TO_DEG 57.295779513082320876798154814105f

typedef struct
{
	float x,y,z;
} Vector3f;

typedef struct
{
	int32_t x,y,z;
} Vector3i;

typedef struct
{
	Vector3f a,b,c;
} Matrix3f;

#pragma pack(1)
// 帧格式控制信息结构定义，共11字节
typedef struct{
	  uint16_t     iFrameID;  // 帧头标识码
	  uint16_t     iFrameLen;   // 帧长度。从帧头到最后的校验码的全部长度
	  uint32_t     iFrameTick;   // 帧时标
	  uint16_t     idSender;     // 发送方  V5.05.241003更改为发送方
//	  uint8_t      idSender;     // 发送方
//	  uint8_t      idReceiver;    // 接收方
	  uint8_t      iFrameType;         // 帧类型码
}THeaderFrame;
#pragma pack()

typedef union {
	uint8_t sChar[4];
	float   fVal;
}TChar2FloatStruct;

typedef union {
	uint8_t    sChar[2];
	uint16_t   fVal;
}TChar2UInt16Struct;

// 定义产品基本配置信息（包含你的产品ID、产品规格型号、授权码U码、产品密钥等）
typedef struct {
  uint32_t ProductId;       // 产品ID
	char     ProductName[16];     // 产品名称，如：AFC-6、AFC-5A、AFC-5B、EEC-3、PTSCanner-1等，字符长度不超过16个字符   
	uint16_t BoardType;    // 所使用电路板索引号（公司内部规定索引号），如：AFC5A_BOARD（1）、AFC5B_BOARD（2）、AFC6_BOARD（3）。2字节（补2字节对齐）
	
  uint32_t ProductAuthCode;      // 产品授权码P码
	uint32_t iRes1;                // 8字节的备用信息
	
  uint16_t UserAuthCode;        // 用户授权码U码。2字节（补2字节对齐）
	uint32_t iRes2;               // 8字节的备用信息
	uint16_t KeyCode;             // 用户Key码。2字节（补2字节对齐）

	uint8_t iRes3[32];            // 32字节的备用信息
	
  uint16_t ProductCrc;          // 校验位（防数据错乱）。2字节（补2字节对齐）
	uint8_t  padding[20];        // 填充到96字节（3*32）
} TProductConfig;

// 无人机配置信息
#pragma pack(4)    // 由于要将其转换为4字节的浮点数，因此，此处按4字节对齐
typedef struct{
	 uint32_t UavId;       // 无人机ID号。且低16位为XXX集群系统中的唯一标识符，即目前设计为一个集群系统最大65536架无人机
	 uint16_t UavFrame;    // 无人机机架，包括：F550_UAV、K80A_UAV、K80B_UAV
	 uint8_t  FcsBoard;    // 飞控板，包括：AFC5A_BOARD、AFC5B_BOARD
	
	// 飞控组件安装矩阵,如：正装 [1，1，1], 反装 [1，-1，-1],……
	 uint8_t  XSetup:1;   //其中: D0对应X轴，为0表示前(+1)，1表示后(-1)；
	 uint8_t  YSetup:1;   //      D1对应Y轴，为0表示右(+1)，1表示左(-1)；
	 uint8_t  ZSetup:1;   //      D2对应Z轴，为0表示下(+1)，1表示上(-1)。
	 uint8_t  ResSetup:4;
	 uint8_t  bAuthorizedFlg:1;      // D7 授权状态信息，20250218 by zqhkd
	
	 uint8_t  Remoter;    // 遥控器, 包括：天地飞遥控器WFLY_ETS6S，开源遥控器JUMPER_T20
	 uint8_t  FlyMode;    // 包括：姿态模式、定高模式、导航模式
	
	 uint16_t AuthorizationCode;   // 
	 uint16_t CRCCode;             // CRC校验码，包括:芯片ID + UavId + CRCCode
	
}SUavSetupPara;

typedef struct{
	 Vector3f AccScale,AccOffset;
	 Vector3f GyroScale,GyroOffset;
}SImuCalibPara;

#define FIRST_INIT_FLAG   0xA5DA
typedef struct{
	 uint16_t FirstInitFlg;  // 2字节，初始化
	
	// 无人机初始配置参数
	 SUavSetupPara UavPara;
	 
	// IMU校准参数
	 SImuCalibPara Icm42688;
	 SImuCalibPara Icm20602;
	 
	// 气压高度计气压零偏值
	 float Dps310Offset;    
   	
  // 磁力计IST8310零偏值
   Vector3f Ist8310Bias;    // 偏置
	 Vector3f Ist8310Radius;    // 椭球半径a,b,c
	
	 float  Ist8310Neg;     // V5.05.240817： 磁力计校准时，不仅按椭球修正公式修正，
	                         //    还需要对比磁力计输出值和IMU解算的航向角值，如果基本一致就没问题；
	                         //    如果出现反向，则补偿模型中需要乘以-1
	 uint16_t ParaSaveCRC;    // 参数保存用CRC校验码
}SSaveParamFlash;
#pragma pack()

// 飞行控制实时任务调度相关的时钟计数器
typedef struct{
	uint8_t  check_flag;
	uint16_t err_flag;
	
	uint16_t  ctlStep;     // simulink模型中的控制步长，一般在5ms~10ms之间
	uint8_t  testStep;    // TestCom端口输出步长，一般在5ms~10ms之间
	
	uint16_t cnt_OutTime;     // 超时检测用时标
	uint16_t relTime2PPS;     // 以1PPS为基准的相对时间
	
	uint32_t  sampStep;
	uint32_t relTime;      // 相对飞行器启动零点时间，发射起飞前始终为0
	uint32_t fcsTime;      // 飞控对准时标
}TRealTimeCnt;

#define SYSTEM_INIT_STS_D0   0
#define LAND_COMPLETE_D1     1
#define ARMED_ENABLE_D2      2
#define DCM_FAST_GROUND_D3   3
typedef struct{
	 uint32_t bSystemInitSts:1;       // D0
	 uint32_t bLandComplete:1; 			  // D1
	 uint32_t bArmedEnable:1;				  // D2
	 uint32_t bDcmFastGroundGains:1;  // D3
	 uint32_t bLimitRollPitch:1;		  // D4
	 uint32_t bLimitYaw:1;					  // D5
	 uint32_t bLimitThrottleLower:1;  // D6
	 uint32_t bLimitThrottleUpper:1;  // D7
	
	 uint32_t bMotorSlowStart:1;     // D8
	 uint32_t Reserved:3;				     // D9--D11
		 
	 uint32_t iSbusErrNum:4;				// D12--D15, 遥控器错误
	 uint32_t iSdCardWriteErr:8;		// D16--D23
	 uint32_t iTaskOverTimeNo:8;    // D24--D31
}AFCSTATUS;

#define MOTORS_CHANEL_NUM    4

#endif

/************************ (C) COPYRIGHT ACE Co. about ANSGlobalDef *****END OF FILE****/
