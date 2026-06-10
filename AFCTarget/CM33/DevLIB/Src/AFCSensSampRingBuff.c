/******************** (C) COPYRIGHT 2019 ACG Tech Co.*************************
 * 作    者： 曾庆华
 * 文 件 名： AFCSensSampRingBuff.c
 * 描    述： AFC-7 多速率传感器采样与本地循环缓冲区管理总线源文件实现
 * 版    本： V7.01.260606
 * 官    网： www.acecreator.com
 * 淘    宝： acecreator.taobao.com
 * 公 众 号： 无人飞行控制
*****************************************************************************/

#include "AFCSensSampRingBuff.h"
#include <string.h>
#include <stdio.h>

/* ================= 1. 外部大时基定义 ================= */
uint32_t g_iMinSamplePeriod       =  1000;  
uint32_t g_iSimulinkAlgorithmStep =  2000;  

/* ================= 2. 全量 static 强行局部化隐蔽 ================= */
static uint32_t s_iFactorICM42688  = 1;           
static uint32_t s_iFactorICM20602  = 1;           
static uint32_t s_iFactorDPS310    = 10;          
static uint32_t s_iFactorIST8310   = 20;          
static uint32_t s_iFactorGPS       = 100;         
static uint32_t s_iFactorAirspeed  = 50;          
static uint32_t s_iFactorRcSbus    = 20;          

typedef enum {
    LOCAL_IMU_ICM42688 = 0, LOCAL_IMU_ICM20602, LOCAL_BARO_DPS310, LOCAL_MAG_IST8310, LOCAL_GPS, LOCAL_AIRSPEED, LOCAL_RC_SBUS, LOCAL_SENSOR_COUNT       
} ELocalSensorID;

typedef struct {
    uint16_t    u16SensorId; const char* szName; size_t uDataSize; uint32_t* pPeriodFactor; void* pLatestShmAddr; RingBuffer* pLocalRingBuffer; size_t uCalculatedRbSize;  
} TLocalSensorConfig;

#define SHM_PHYSICAL_BASE_ADDR   (0x38000000) 
#define M33_LOCAL_POOL_SIZE     (16 * 1024)   
static uint8_t g_pM33LocalBufferPool[M33_LOCAL_POOL_SIZE] __attribute__((aligned(4)));

static TLocalSensorConfig s_astDynamicSensorTable[LOCAL_SENSOR_COUNT] = {
    {LOCAL_IMU_ICM42688, "ICM42688", 28,  &s_iFactorICM42688,  NULL, NULL, 0}, 
    {LOCAL_IMU_ICM20602, "ICM20602", 28,  &s_iFactorICM20602,  NULL, NULL, 0}, 
    {LOCAL_BARO_DPS310,  "DPS310",   20,  &s_iFactorDPS310,    NULL, NULL, 0}, 
    {LOCAL_MAG_IST8310,  "IST8310",  24,  &s_iFactorIST8310,   NULL, NULL, 0}, 
    {LOCAL_GPS,          "GPS",      52,  &s_iFactorGPS,       NULL, NULL, 0}, 
    {LOCAL_AIRSPEED,     "AIRSPEED", 20,  &s_iFactorAirspeed,  NULL, NULL, 0}, 
    {LOCAL_RC_SBUS,      "RC_SBUS",  44,  &s_iFactorRcSbus,    NULL, NULL, 0}  
};

/* 引入外部物理硬件底层非阻塞抓取接口 (老飞控移植函数接口桩) */
extern bool Driver_ICM42688_CheckAndRead(uint8_t* pBuf);
extern bool Driver_ICM20602_CheckAndRead(uint8_t* pBuf);
extern bool Driver_DPS310_CheckAndRead(uint8_t* pBuf);
extern bool Driver_IST8310_CheckAndRead(uint8_t* pBuf);

// 老飞控由中断维护的三个异步全局设备变量
extern TGpsData       g_stGpsGlobal;       
extern TAirspeedData  g_stAirspeedGlobal;  
extern TRcData        g_stRcSbusGlobal;    

volatile TA35ToM33CtrlCmd* gp_ShmCtrlCmd   = (volatile TA35ToM33CtrlCmd*)0x38002000;
volatile TA35ToM33PerceptionData* gp_ShmPerception = (volatile TA35ToM33PerceptionData*)0x38002100;

/* 内部小写驼峰私有配置指针函数 */
static inline TLocalSensorConfig* getSensorConfig(uint16_t u16Id) {
    if (u16Id >= LOCAL_SENSOR_COUNT) return NULL;
    return &s_astDynamicSensorTable[u16Id];
}

void InitAFCSensorRingBuff(ESystemRunMode eMode) {
    g_eCurrentRunMode = eMode;
    uintptr_t currentShmOffset = SHM_PHYSICAL_BASE_ADDR;
    uint32_t  currentLocalOffset = 0;

    for (int i = 0; i < LOCAL_SENSOR_COUNT; i++) {
        TLocalSensorConfig* pCfg = &s_astDynamicSensorTable[i];
        size_t alignedShmSize = (pCfg->uDataSize + 3) & ~3;
        pCfg->pLatestShmAddr = (void*)currentShmOffset;
        currentShmOffset += alignedShmSize; 
        
        uint32_t sensorPeriodUs = g_iMinSamplePeriod * (*pCfg->pPeriodFactor);
        uint32_t requiredFrames = g_iSimulinkAlgorithmStep / sensorPeriodUs;
        if (sensorPeriodUs > g_iSimulinkAlgorithmStep || requiredFrames < 2) { requiredFrames = 2; }
        
        uint32_t queueDepth = requiredFrames * 2; size_t rawDataSpace = queueDepth * pCfg->uDataSize;
        if ((currentLocalOffset + sizeof(RingBuffer) + rawDataSpace) > M33_LOCAL_POOL_SIZE) { while(1); }
        
        pCfg->pLocalRingBuffer = (RingBuffer*)&g_pM33LocalBufferPool[currentLocalOffset];
        currentLocalOffset += sizeof(RingBuffer); currentLocalOffset += rawDataSpace;
        RingBuffer_Init(pCfg->pLocalRingBuffer);
    }
    if (eMode == RUN_MODE_FLIGHT) { memset((void*)SHM_PHYSICAL_BASE_ADDR, 0, (currentShmOffset - SHM_PHYSICAL_BASE_ADDR)); }
}

/* ==========================================================================
 * 3. 🚨 曾博士定型：各组件专属外部 API 级底层设备驱动 ReadAndPush 函数群实现
 * ========================================================================== */

void ICM42688_ReadAndPush(uint32_t u32CurrentTick, uint64_t u64QueryUs) {
    TLocalSensorConfig* pCfg = getSensorConfig(LOCAL_IMU_ICM42688);
    if ((u32CurrentTick % (*pCfg->pPeriodFactor)) != 0) return;
    uint8_t buf[32];
    
    if (g_eCurrentRunMode == RUN_MODE_FLIGHT) {
        if (Driver_ICM42688_CheckAndRead(buf)) {
            *(uint64_t*)(buf + pCfg->uDataSize - sizeof(uint64_t)) = u64QueryUs;
            memcpy(pCfg->pLatestShmAddr, buf, pCfg->uDataSize); // 实飞：推入共享区供地面监测
            RingBuffer_Write(pCfg->pLocalRingBuffer, (const char*)buf, pCfg->uDataSize);
        }
    } else {
        memcpy(buf, pCfg->pLatestShmAddr, pCfg->uDataSize); // 仿真：直接从共享区拉数据
        *(uint64_t*)(buf + pCfg->uDataSize - sizeof(uint64_t)) = u64QueryUs;
        RingBuffer_Write(pCfg->pLocalRingBuffer, (const char*)buf, pCfg->uDataSize);
    }
}

void ICM20602_ReadAndPush(uint32_t u32CurrentTick, uint64_t u64QueryUs) {
    TLocalSensorConfig* pCfg = getSensorConfig(LOCAL_IMU_ICM20602);
    if ((u32CurrentTick % (*pCfg->pPeriodFactor)) != 0) return;
    uint8_t buf[32];
    if (g_eCurrentRunMode == RUN_MODE_FLIGHT) {
        if (Driver_ICM20602_CheckAndRead(buf)) {
            *(uint64_t*)(buf + pCfg->uDataSize - sizeof(uint64_t)) = u64QueryUs;
            memcpy(pCfg->pLatestShmAddr, buf, pCfg->uDataSize);
            RingBuffer_Write(pCfg->pLocalRingBuffer, (const char*)buf, pCfg->uDataSize);
        }
    } else {
        memcpy(buf, pCfg->pLatestShmAddr, pCfg->uDataSize);
        *(uint64_t*)(buf + pCfg->uDataSize - sizeof(uint64_t)) = u64QueryUs;
        RingBuffer_Write(pCfg->pLocalRingBuffer, (const char*)buf, pCfg->uDataSize);
    }
}

void DPS310_ReadAndPush(uint32_t u32CurrentTick, uint64_t u64QueryUs) {
    TLocalSensorConfig* pCfg = getSensorConfig(LOCAL_BARO_DPS310);
    if ((u32CurrentTick % (*pCfg->pPeriodFactor)) != 0) return;
    uint8_t buf[32];
    if (g_eCurrentRunMode == RUN_MODE_FLIGHT) {
        if (Driver_DPS310_CheckAndRead(buf)) {
            *(uint64_t*)(buf + pCfg->uDataSize - sizeof(uint64_t)) = u64QueryUs;
            memcpy(pCfg->pLatestShmAddr, buf, pCfg->uDataSize);
            RingBuffer_Write(pCfg->pLocalRingBuffer, (const char*)buf, pCfg->uDataSize);
        }
    } else {
        memcpy(buf, pCfg->pLatestShmAddr, pCfg->uDataSize);
        *(uint64_t*)(buf + pCfg->uDataSize - sizeof(uint64_t)) = u64QueryUs;
        RingBuffer_Write(pCfg->pLocalRingBuffer, (const char*)buf, pCfg->uDataSize);
    }
}

void IST8310_ReadAndPush(uint32_t u32CurrentTick, uint64_t u64QueryUs) {
    TLocalSensorConfig* pCfg = getSensorConfig(LOCAL_MAG_IST8310);
    if ((u32CurrentTick % (*pCfg->pPeriodFactor)) != 0) return;
    uint8_t buf[32];
    if (g_eCurrentRunMode == RUN_MODE_FLIGHT) {
        if (Driver_IST8310_CheckAndRead(buf)) {
            *(uint64_t*)(buf + pCfg->uDataSize - sizeof(uint64_t)) = u64QueryUs;
            memcpy(pCfg->pLatestShmAddr, buf, pCfg->uDataSize);
            RingBuffer_Write(pCfg->pLocalRingBuffer, (const char*)buf, pCfg->uDataSize);
        }
    } else {
        memcpy(buf, pCfg->pLatestShmAddr, pCfg->uDataSize);
        *(uint64_t*)(buf + pCfg->uDataSize - sizeof(uint64_t)) = u64QueryUs;
        RingBuffer_Write(pCfg->pLocalRingBuffer, (const char*)buf, pCfg->uDataSize);
    }
}

void FemtMesGPS_ReadAndPush(uint32_t u32CurrentTick, uint64_t u64QueryUs) {
    TLocalSensorConfig* pCfg = getSensorConfig(LOCAL_GPS);
    if ((u32CurrentTick % (*pCfg->pPeriodFactor)) != 0) return;
    uint8_t buf[64];
    if (g_eCurrentRunMode == RUN_MODE_FLIGHT) {
        __disable_irq(); memcpy(buf, (uint8_t*)&g_stGpsGlobal, pCfg->uDataSize); __enable_irq(); // 快照劫持
        *(uint64_t*)(buf + pCfg->uDataSize - sizeof(uint64_t)) = u64QueryUs;
        memcpy(pCfg->pLatestShmAddr, buf, pCfg->uDataSize); // 实飞：上抛共享区供地面监测
        RingBuffer_Write(pCfg->pLocalRingBuffer, (const char*)buf, pCfg->uDataSize);
    } else {
        memcpy(buf, pCfg->pLatestShmAddr, pCfg->uDataSize);
        *(uint64_t*)(buf + pCfg->uDataSize - sizeof(uint64_t)) = u64QueryUs;
        RingBuffer_Write(pCfg->pLocalRingBuffer, (const char*)buf, pCfg->uDataSize);
    }
}

void MS5525_ReadAndPush(uint32_t u32CurrentTick, uint64_t u64QueryUs) {
    TLocalSensorConfig* pCfg = getSensorConfig(LOCAL_AIRSPEED);
    if ((u32CurrentTick % (*pCfg->pPeriodFactor)) != 0) return;
    uint8_t buf[32];
    if (g_eCurrentRunMode == RUN_MODE_FLIGHT) {
        __disable_irq(); memcpy(buf, (uint8_t*)&g_stAirspeedGlobal, pCfg->uDataSize); __enable_irq();
        *(uint64_t*)(buf + pCfg->uDataSize - sizeof(uint64_t)) = u64QueryUs;
        memcpy(pCfg->pLatestShmAddr, buf, pCfg->uDataSize);
        RingBuffer_Write(pCfg->pLocalRingBuffer, (const char*)buf, pCfg->uDataSize);
    } else {
        memcpy(buf, pCfg->pLatestShmAddr, pCfg->uDataSize);
        *(uint64_t*)(buf + pCfg->uDataSize - sizeof(uint64_t)) = u64QueryUs;
        RingBuffer_Write(pCfg->pLocalRingBuffer, (const char*)buf, pCfg->uDataSize);
    }
}

void SBusRC_ReadAndPush(uint32_t u32CurrentTick, uint64_t u64QueryUs) {
    TLocalSensorConfig* pCfg = getSensorConfig(LOCAL_RC_SBUS);
    if ((u32CurrentTick % (*pCfg->pPeriodFactor)) != 0) return;
    uint8_t buf[64];
    // 🚨 遵照铁律：遥控器硬核物理一视同仁，全生命周期无条件走真机 DMA 中断全局变量提取
    __disable_irq(); memcpy(buf, (uint8_t*)&g_stRcSbusGlobal, pCfg->uDataSize); __enable_irq();
    *(uint64_t*)(buf + pCfg->uDataSize - sizeof(uint64_t)) = u64QueryUs;
    memcpy(pCfg->pLatestShmAddr, buf, pCfg->uDataSize); // 刚性写入共享区，保障地面站实时监测
    RingBuffer_Write(pCfg->pLocalRingBuffer, (const char*)buf, pCfg->uDataSize);
}

bool GetSensorIdData(uint16_t eSensorId, uint64_t u64TargetTimeUs, uint8_t* pOutData) {
    if (eSensorId >= LOCAL_SENSOR_COUNT || !pOutData) return false;
    TLocalSensorConfig* pCfg = &s_astDynamicSensorTable[eSensorId];
    uint8_t tempFrame[64]; bool matchFound = false;
    
    __disable_irq(); 
    while (RingBuffer_IsFrameAvailable(pCfg->pLocalRingBuffer, pCfg->uDataSize)) {
        RingBuffer_Peek(pCfg->pLocalRingBuffer, (char*)tempFrame, pCfg->uDataSize);
        uint64_t frameTime = *(uint64_t*)(tempFrame + pCfg->uDataSize - sizeof(uint64_t));
        if (frameTime >= u64TargetTimeUs) { memcpy(pOutData, tempFrame, pCfg->uDataSize); matchFound = true; break; }
        RingBuffer_Read(pCfg->pLocalRingBuffer, (char*)tempFrame, pCfg->uDataSize);
    }
    __enable_irq(); return matchFound;
}

void GetCrossCoreCmdAndPerception(TA35ToM33CtrlCmd* pOutCmd, TA35ToM33PerceptionData* pOutPercept) {
    if (!gp_ShmCtrlCmd || !gp_ShmPerception) return;
    __disable_irq();
    if (pOutCmd)     memcpy((uint8_t*)pOutCmd, (const uint8_t*)gp_ShmCtrlCmd, sizeof(TA35ToM33CtrlCmd));
    if (pOutPercept) memcpy((uint8_t*)pOutPercept, (const uint8_t*)gp_ShmPerception, sizeof(TA35ToM33PerceptionData));
    __enable_irq();
}







void AFCTaskOfSampleSensors(uint32_t u32CurrentTick) {
    uint64_t u64QueryUs = Get_fcsM33Time_Us();
    uint8_t stLocalFrameBuffer[sizeof(TGpsData) > 256 ? sizeof(TGpsData) : 256]; 

    for (int i = 0; i < SENSOR_TYPE_COUNT; i++) {
        TAutoSensorConfig* pCfg = (TAutoSensorConfig*)&g_astDynamicSensorTable[i];
        
        if ((u32CurrentTick % (*pCfg->pPeriodFactor)) == 0) {
            
            // RC 遥控器全模态纯物理硬核透传，不离线
            if (pCfg->eSensorId == SENSOR_RC_SBUS) {
                __disable_irq();
                memcpy(stLocalFrameBuffer, (uint8_t*)&g_stRcSbusGlobal, pCfg->uDataSize);
                __enable_irq();
                
                *(uint64_t*)(stLocalFrameBuffer + pCfg->uDataSize - sizeof(uint64_t)) = u64QueryUs;
                memcpy(pCfg->pLatestShmAddr, stLocalFrameBuffer, pCfg->uDataSize);
                RingBuffer_Write(pCfg->pLocalRingBuffer, (const char*)stLocalFrameBuffer, pCfg->uDataSize);
                continue; 
            }
            
            if (g_eCurrentRunMode == RUN_MODE_FLIGHT) {
                bool bDataReady = false;
                switch (pCfg->eSensorId) {
                    case SENSOR_IMU_ICM42688: bDataReady = Driver_ICM42688_CheckAndRead(stLocalFrameBuffer); break;
                    case SENSOR_IMU_ICM20602: bDataReady = Driver_ICM20602_CheckAndRead(stLocalFrameBuffer); break;
                    case SENSOR_BARO_DPS310:  bDataReady = Driver_DPS310_CheckAndRead(stLocalFrameBuffer);  break;
                    case SENSOR_MAG_IST8310:  bDataReady = Driver_IST8310_CheckAndRead(stLocalFrameBuffer);  break;
                    case SENSOR_GPS:
                        __disable_irq(); memcpy(stLocalFrameBuffer, (uint8_t*)&g_stGpsGlobal, pCfg->uDataSize); __enable_irq();
                        bDataReady = true; break;
                    case SENSOR_AIRSPEED:
                        __disable_irq(); memcpy(stLocalFrameBuffer, (uint8_t*)&g_stAirspeedGlobal, pCfg->uDataSize); __enable_irq();
                        bDataReady = true; break;
                    default: break;
                }
                
                if (bDataReady) {
                    *(uint64_t*)(stLocalFrameBuffer + pCfg->uDataSize - sizeof(uint64_t)) = u64QueryUs;
                    memcpy(pCfg->pLatestShmAddr, stLocalFrameBuffer, pCfg->uDataSize);
                    RingBuffer_Write(pCfg->pLocalRingBuffer, (const char*)stLocalFrameBuffer, pCfg->uDataSize);
                }
            } 
            else {
                /* 半实物仿真模态：硬件闭锁，无感从共享区搬运模拟单帧 */
                memcpy(stLocalFrameBuffer, pCfg->pLatestShmAddr, pCfg->uDataSize);
                *(uint64_t*)(stLocalFrameBuffer + pCfg->uDataSize - sizeof(uint64_t)) = u64QueryUs;
                RingBuffer_Write(pCfg->pLocalRingBuffer, (const char*)stLocalFrameBuffer, pCfg->uDataSize);
            }
        }
    }
}

bool GetSensorIdData(ESensorID eId, uint64_t u64TargetTimeUs, uint8_t* pOutData) {
    TAutoSensorConfig* pCfg = sensor_GetConfig(eId);
    if (!pCfg || !pOutData) return false;
    
    uint8_t tempFrame[sizeof(TGpsData) > 256 ? sizeof(TGpsData) : 256]; 
    bool matchFound = false;
    
    __disable_irq(); 
    while (RingBuffer_IsFrameAvailable(pCfg->pLocalRingBuffer, pCfg->uDataSize)) {
        RingBuffer_Peek(pCfg->pLocalRingBuffer, (char*)tempFrame, pCfg->uDataSize);
        uint64_t frameTime = *(uint64_t*)(tempFrame + pCfg->uDataSize - sizeof(uint64_t));
        
        if (frameTime >= u64TargetTimeUs) {
            memcpy(pOutData, tempFrame, pCfg->uDataSize);
            matchFound = true;
            break; 
        }
        RingBuffer_Read(pCfg->pLocalRingBuffer, (char*)tempFrame, pCfg->uDataSize);
    }
    __enable_irq(); 
    return matchFound;
}

volatile TA35ToM33CtrlCmd* gp_ShmCtrlCmd   = (volatile TA35ToM33CtrlCmd*)0x38002000;
volatile TA35ToM33PerceptionData* gp_ShmPerception = (volatile TA35ToM33PerceptionData*)0x38002100;

void GetCrossCoreCmdAndPerception(TA35ToM33CtrlCmd* pOutCmd, TA35ToM33PerceptionData* pOutPercept) {
    if (!gp_ShmCtrlCmd || !gp_ShmPerception) return;
    __disable_irq();
    if (pOutCmd)     memcpy((uint8_t*)pOutCmd, (const uint8_t*)gp_ShmCtrlCmd, sizeof(TA35ToM33CtrlCmd));
    if (pOutPercept) memcpy((uint8_t*)pOutPercept, (const uint8_t*)gp_ShmPerception, sizeof(TA35ToM33PerceptionData));
    __enable_irq();
}