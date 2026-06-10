/******************** (C) COPYRIGHT 2019 ACG Tech Co.*************************
 * 作    者： 曾庆华
 * 文 件 名： AFCSensSampRingBuff.h
 * 描    述： AFC-7 多速率传感器采样与本地循环缓冲区管理总线头文件
 *           本模块全量统一采用 4 字节硬对齐规范，通过绝对微秒时标实现多率异步解耦
 * 版    本： V7.01.260606
 * 官    网： www.acecreator.com
 * 淘    宝： acecreator.taobao.com
 * 公 众 号： 无人飞行控制
*****************************************************************************/
#pragma once

#ifndef ZFZK_AFC_SensSamp_RingBuff_H
#define ZFZK_AFC_SensSamp_RingBuff_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "RingBuff.h"        // 引入项目原有的无锁单写单读环形缓冲区库

#include "AFCGlobalDef.h"

/* ================= 1. 基础时序控制与周期因子变量 ================= */
extern uint32_t g_iMinSamplePeriod;         // 最小传感器采集基准周期 (单位: us)
extern uint32_t g_iSimulinkAlgorithmStep;    // 用户 Simulink 算法模型解算步长 (单位: us)

// 各传感器相对于 g_iMinSamplePeriod 的采集周期整数倍率因子变量
extern uint32_t g_iFactorICM42688;
extern uint32_t g_iFactorICM20602;
extern uint32_t g_iFactorDPS310;
extern uint32_t g_iFactorIST8310;
extern uint32_t g_iFactorGPS;
extern uint32_t g_iFactorAirspeed;

/* ================= 2. 传感器 ID 与系统运行模态枚举 ================= */
typedef enum {
    SENSOR_IMU_ICM42688 = 0,
    SENSOR_IMU_ICM20602,
    SENSOR_BARO_DPS310,
    SENSOR_MAG_IST8310,
    SENSOR_GPS,
    SENSOR_AIRSPEED,
    SENSOR_TYPE_COUNT       // 动态跟踪注册的传感器总数
} ESensorID;

typedef enum {
    RUN_MODE_FLIGHT = 0,    // 真实物理自主飞行模态
    RUN_MODE_HIL_SIM        // 半实物在环仿真模态
} ESystemRunMode;

extern volatile ESystemRunMode g_eCurrentRunMode; // 全局运行模态字

/* ================= 3. 航空级规范化传感器数据帧定义 ================= */
#pragma pack(push, 4)
/* 1. 惯性测量单元 (IMU) 数据结构体 */
typedef struct{
	Vector3f  fGyro;      // 陀螺仪测量或仿真模拟的机体轴角速度值（rad/s）
	Vector3f  fAcc;       // 加速度计测量或仿真模拟的机体轴加速度值（m/s^2）
	float     fTemp;        //  惯组器件内部温度值（°C）
	uint64_t  u64TimeSampleUs;  // 采集时刻点的本地 fcsM33Time_us 绝对微秒时标
}TImuData;

/* 2. 气压高度计数据结构体 */
typedef struct {
    float    fPress;           // 气压计测量或仿真模拟气压值 (Pa)
	float    fBarHeight;       // 气压计测量或仿真模拟气压高度值（m）
	float    fTemp;           //  气压计器件内部温度值（°C）
    uint64_t u64TimeSampleUs;    // 对标对齐后的本地 fcsM33Time_us 绝对微秒时标
} TBaroData;

/* 3. 磁力计数据结构体 */
typedef struct{
	Vector3f  fMag;         //  磁力计测量或仿真模拟的机体轴角速度值（rad/s）
	float     fHeadDeg;      //  气压计测量结算的航向角（°）
	float     fTemp;        //  气压计器件内部温度值（°C）
	uint64_t  u64TimeSampleUs;  // 采集时刻点的本地 fcsM33Time_us 绝对微秒时标
}TMagData;

/* 4. 低频卫星导航 (GPS)数据结构体  */
typedef struct {
    double   dGpsLon;           // 经度：采用双精度浮点型以保障厘米级大地坐标分辨率 (deg)
    double   dGpsLat;           // 纬度：采用双精度浮点型 (deg)
    float    fGpsAlt;           // 高度：采用单精度浮点型符合航空控制规范 (m)
    Vector3f fVNED;            // 北东地 (NED) 坐标系下的绝对地速三轴分量 (m/s)
    float    fHeadDeg;         // 卫导卫星航向角 (°)
    uint8_t  iFixStatus;       // 定位状态字: 3-3D_Fix, 4-RTK_Float, 5-RTK_Fixed
    uint8_t  u8Reserved[3];    // 4字节对齐填充
	float    fTemp;          //  GPS器件内部温度值（°C）
    uint64_t u64TimeSampleUs;  // 采集时刻点的本地 fcsM33Time_us 绝对微秒时标
} TGpsData;

/* 5. 空速计数据结构 */
typedef struct {
    float    fDifP;           // 测量或仿真模拟的上下表面差压值 (Pa)
    float    fAirSpeed;      // 测量或仿真模拟的指示空速值（m/s）
	float    fTemp;            //  空速计器件内部温度值（°C）
	uint64_t u64TimeSampleUs;  // 采集时刻点的本地 fcsM33Time_us 绝对微秒时标
} TAirspeedData;

/* 6. RC 遥控器通道透传数据结构体 */
typedef struct {
    // ---- 第一部分：原始设备通道信息（完全透明，由上层用户自定功能） ----
    uint16_t iChannel[16];   // 标准航空规范：直接透传 16 个物理通道的原始数值 (us)
                               // 用户在 Simulink 中可自由决定 ch[0]~ch[15] 的物理映射

    // ---- 第二部分：物理链路链路状态与健康度（供底座和算法层双重拦截保护） ----
    uint8_t  u8Rssi;           // 接收机测量的实时信号强度指示 (0 ~ 100%)
    uint8_t  u8FailsafeStatus; // 链路状态字：0-正常连接；1-断联失控 (FailSafe)
    uint16_t u16LostFrames;    // 物理链路接收过程中累计的丢帧计数
    
    // ---- 第三部分：绝对控制时轴戳 ----
    uint64_t u64TimeSampleUs;  // 劫持该接收机全局变量快照时刻的本地绝对微秒时标
} TRcData;


/* ================= 适航级刚性指令控制模式选通字定义 ================= */
#define AFC7_CTRL_MODE_MANUAL_STABILIZE   0x01  // 纯姿态稳定控制回路（遥控器手动拦截自稳）
#define AFC7_CTRL_MODE_TRACK_POSITION     0x02  // 位置自主跟踪模式
#define AFC7_CTRL_MODE_TRACK_TRAJECTORY   0x03  // 位置/速度/加速度高阶复合轨迹跟踪模式
#define AFC7_CTRL_MODE_LOITER_HOVER       0x04  // 定点飞行 / 定高悬停模式
#define AFC7_CTRL_MODE_LOITER_CIRCLE      0x05  // 🚨 水平定点圆周盘旋模式
#define AFC7_CTRL_MODE_AUTO_LANDING       0x06  // 自主引导滑跑降落模式
#define AFC7_CTRL_MODE_HEADING_CLIMB      0x07  // 🚨 纯航向角/爬升率控制模式

/* ================= 最终适航定型版：AFC-7 共享内存指令端口结构体 (0x38002000) =================
 * 规范说明：遵照 ARINC 429/653 端口总线规范，全物理空间拉直对齐，无动态分支，边界编译期静态死锁。
 * 通过统一的九维空间矩阵流与伴随状态变量，完美支持 Simulink 算法二次开发的极致灵活性。
 */
typedef struct {
    // ---- [分区一：控制模态与生命期追踪 (ARINC 标准控制头)] ----
    uint8_t     u8FlyCtrlMode;    // 飞行控制模式指令选通字（取上方定义的核心控制模式宏）
    uint8_t     u8Reserved[3];    // 4字节寻址对齐填充
    uint32_t    u32CmdSeq;        // 🚨 曾博士刚性要求保留：指令自增流水号，跨核心跳与突发有效性判定锁
    uint64_t    u64TimeSampleUs;  // A35 写入瞬间经 T0 快照对齐后的 M33 本地绝对微秒时标 (us)

    // ---- [分区二：全平直三维导航与控制指令矩阵矩阵] ----
    Vector3f    vTargetPosNed;    // 目标点 / 轨迹点 / 盘旋中心点 的空间三轴绝对位置 (m)
    Vector3f    vTargetVelNed;    // 目标空间三轴速度前馈 / 切线速度控制量 (m/s)
    Vector3f    vTargetAccNed;    // 目标空间三轴前馈加速度指令 (m/s^2)

    // ---- [分区三：航向与高级多模态伴随参数流] ----
    float       fTargetYawRad;    // 🚨 跟踪目标航向角指令（航向选通或各轨迹断面目标偏航角）(rad)
    float       fCircleRadius;    // 🚨 盘旋模式专属：水平盘旋半径指令 (m)
    float       fGlideSlopeDeg;   // 降落模式专属：下滑道几何斜率夹角 (°)
    float       fFlareHeight;     // 降落模式专属：触地前拉平解锁的安全高度边界 (m)
    float       fTargetAirspeed;  // 前向期望目标空速指令 (m/s)
} TA35ToM33CtrlCmd;

typedef struct {
    Vector3f    vPosition;        
    Vector3f    vVelocity;        
    float       fQuaternion[4];   
    float       fCovariance[6];   
    uint8_t     u8DataTypeFlag;   
    uint8_t     u8Reserved[3];    
    uint64_t    u64TimeSampleUs;  
} TA35ToM33PerceptionData;


// 仿真注入参数集结构体定义
typedef struct {
    Vector3f    vGyro;            // 惯组角速度 wx, wy, wz (rad/s)
    Vector3f    vAcc;             // 惯组加速度 ax, ay, az (m/s^2)
    Vector3f    vMag;             // 地磁场强度三轴分量 (Gauss)
    float       fBaroAlt;         // 气压高度计解算高度 (m)
    float       fAirspeed;        // 微压计空速值 (m/s)
    double      dGpsLon;          // 经度 (deg)
    double      dGpsLat;          // 纬度 (deg)
    float       fGpsAlt;          // 大地高 (m)
    Vector3f    vGpsVelNed;       // 绝对速度三轴向量 (m/s)
    float       fGpsHeadDeg;      // 卫导卫星航向角 (°)
    uint8_t     u8GpsFixStatus;   
    uint8_t     u8Padding[7];     
    uint64_t    simA35Time_us;    // 同步绝对微秒时标
} TSimSens;

/* ================= 4. 底座管理核心抽象配置结构体 ================= */
typedef struct {
    ESensorID   eSensorId;         // 传感器唯一硬核 ID
    const char* szName;            // 调试与日志用全名字符串
    size_t      uDataSize;         // 对应数据结构体的真实字节大小 (sizeof)
    uint32_t*   pPeriodFactor;     // 指向该传感器周期倍数变量的指针

    void*       pLatestShmAddr;    // 自动求解：轻量化共享内存最新单帧中转地址
    RingBuffer* pLocalRingBuffer;  // 自动求解：M33 本地高速度 SRAM 循环缓冲头指针
    size_t       uCalculatedRbSize; // 自动求解：本地环形缓冲区占用的实际总字节数
} TAutoSensorConfig;

#pragma pack(pop)

// 声明全局动态配置表，允许算法层或黑匣子安全查询元数据属性
extern const TAutoSensorConfig g_astDynamicSensorTable[SENSOR_TYPE_COUNT];

/* ================= 5. 外部公共调用 API 声明 ================= */
/**
 * @brief 核心初始化引擎
 * @details 依据当前的周期控制变量与各结构体真实 size，自动动态求解并绑定 SHM 物理地址与本地环形缓冲区深度
 * @param eMode 初始确立的系统硬实时运行模态 (实飞 / HIL仿真)
 */
void InitAFCSensorRingBuff(ESystemRunMode eMode);

/**
 * @brief 独立传感器采集任务线程专用的单步扫描查询函数
 * @details 由 AFCTask 采集线程在每个 g_iMinSamplePeriod 的刚性卡点上单次敲击。
 * 内部自动执行多速率周期判定、双模态软件非阻塞一次性高速抓取、绝对时标固化和入队压栈。
 * @param u32CurrentTick 系统自上电或引导开始的自增基础软件 Tick 计数
 */
void AFCTaskOfSampleSensors(uint32_t u32CurrentTick);

/**
 * @brief 用户 Simulink 算法模型层（GetSensorsModel 组件）专用的异步时标对齐提取接口
 * @details 算法解算步长到来时，按需从本地历史离散数据集中向后回溯，异步提取最贴合目标时刻点的一帧标准态势快照
 * @param eId 目标检索的传感器 ID
 * @param u64TargetTimeUs 当前解算步长 g_iSimulinkAlgorithmStep 对应的期望目标时刻点时间戳 (us)
 * @param pOutData 导出的对齐数据接收缓冲区指针 (大小必须等同于该传感器的真实数据帧大小)
 * @return true 成功检索并提出满足时间窗的帧；false 缓冲区内暂时没有符合时标要求的数据
 */
bool GetSensorIdData(ESensorID eId, uint64_t u64TargetTimeUs, uint8_t* pOutData);

#endif