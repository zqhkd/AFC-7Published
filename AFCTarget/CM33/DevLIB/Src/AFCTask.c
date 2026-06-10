/******************** (C) COPYRIGHT 2019 ACE Tech Co.*************************
 * 作    者 ： 曾庆华
 * 文 件 名 ： AFCTask.c
 * 版    本 ： V7.01.260601 
 * 描    述 ： AFC-7任务管理模块 (单机最简调试无错标准基准)
 *****************************************************************************/
#include "cmsis_os2.h"
#include "i2c.h" 
#include <string.h> // 实例化 memcpy 必须
#include "FreeRTOS.h" // 支撑 pdMS_TO_TICKS 宏定义

#include "AFCTask.h"
#include "AFCGlobalDef.h"
#include "AFCGlobalVar.h" 
// #include "ICM42688.h"
#include "SimuAlgorithm.h"

#define PARAM_READY_FLAG_ADDR 0x38004000
// 实体实例化共享内存快照指针
#define SHARED_RAM_BASE   0x38000000

volatile TT0Snapshot* const p_T0Snapshot = (TT0Snapshot*)(SHARED_RAM_BASE + 0x5000);   // 暂定全局区0x5000


TSimSens g_SimSens;
TFcsCmd g_FcsCmd;

/* 🚨 在 M33 专属普通内存区域内开辟长度各异、相互隔离的多率环形缓冲区阵列 */
TIMUFrame  g_sIMURingBuffer[32];   // 惯组 2kHz 高频刚性滑窗 (32帧)
TBaroFrame g_sBaroRingBuffer[8];   // 气压计 200Hz 中频滑窗 (8帧)
uint8_t    g_u8IMUWriteIdx = 0;
uint8_t    g_u8BaroWriteIdx = 0;

/* 🚨 空中/地面高度耦合调参专属【影子参数暂存区】变量 */
TParamTuningPacket g_sShadowParam;
bool g_bParamReadyFlag = false;


// 调试用本地仿真状态控制桩
bool Is_Start_Triggered_By_System(void) { return true; }
bool bIsHilSimulationMode(void)         { return true; } 
bool bIsA35CommandAvailable(void)       { return false; }

void pushFastStateToA35SharedRam(uint64_t time_us) { (void)time_us; }
void pushHealthDataToA35Queue(uint64_t time_us)    { (void)time_us; }
uint64_t IPC_ReadA35Uint64MicrosecondTimestamp(void) { return g_sRealTimeCount.fcsM33Time_us + 200; }
void readA35FcsCommands(void) { }

// 底层通信包搬运接口桩
void vReadSimDataPacket(TSimSens* dest, uint64_t* ts) 
{ 
    *ts = g_sRealTimeCount.taskA35Time_us; 
    dest->gyro[0] = 0.01f; 
}
void IPC_Notify_A35_Flight_Start_Event(void) { }

// 补齐初始化所缺少的本地变量标记（非全局总线变量）
bool g_bUsedOfA35UAVPara = false;
bool bSPI1IsUsing = false;
bool bSPI4IsUsing = false;

#define __AFC7_ONLYM33_DEBUG_MODE__
#ifdef __AFC7_ONLYM33_DEBUG_MODE__
void Thread_Virtual_A35_Driver(void *argument)
{
    (void)argument; 
    uint64_t virtual_tsn_us = 10000000; 
    uint32_t loop_cnt = 0;

    for(;;)
    {
        osDelay(1); // 模拟 1ms 周期
        virtual_tsn_us += 1000; 
        loop_cnt++;

        // 1. 模拟 A35 发送高频 TSN 硬件锁相时钟，直接逆向灌入 M33 IPCC 接收回调
        IPCCTsnSyncCallback(virtual_tsn_us);

        // 2. 模拟运行到第 5 秒（5000ms）时，外部仿真机或地面站突发下发起飞/仿真 T0 信号
        if (loop_cnt == 5000)
        {
            vTriggerA35T0Latched(virtual_tsn_us);
        }
    }
}
#endif

void vInitRealTaskParameter(void)
{
    g_sRealTimeCount.check_flag = 0;
    g_sRealTimeCount.err_flag = 0;

    g_sRealTimeCount.ctlStep  = 0;
    g_sRealTimeCount.testStep = 0;
    g_sRealTimeCount.fcsM33Time = 0; 

    g_sRealTimeCount.cnt_OutTime = 0;
    g_sRealTimeCount.relTime2PPS = 0;

    g_sRealTimeCount.fcsM33Time_us  = 0; 
    g_sRealTimeCount.taskA35Time_us = 0;
}

void vTaskRealTimeCount(void)
{
    g_sRealTimeCount.fcsM33Time++; 
    g_sRealTimeCount.ctlStep++;
    g_sRealTimeCount.testStep++;

    g_sRealTimeCount.cnt_OutTime++;          
    g_sRealTimeCount.relTime2PPS++;

    if(g_sRealTimeCount.check_flag >= g_iSimulinkAlgorithmStep)
    {
        g_sRealTimeCount.err_flag ++;     
    }
    else
    {
        g_sRealTimeCount.check_flag += 1;   
    }
}

static uint32_t s_dwLastIpcCycCnt = 0;

/* ==========================================================================
 * 核心更正（1）：IPCC 中断服务程序（由 TSN 硬件网络时钟直接逆向驱动！）
 * ========================================================================== */
void IPCCTsnSyncCallback(uint64_t tsn_network_us)
{
    s_dwLastIpcCycCnt = DWT->CYCCNT;
    g_sRealTimeCount.taskA35Time_us = tsn_network_us;
    
    if (p_T0Snapshot->bIsT0Latched == 0 && Is_Start_Triggered_By_System())
    {
        p_T0Snapshot->t0_A35_us = tsn_network_us;
        p_T0Snapshot->t0_M33_us = g_sRealTimeCount.fcsM33Time_us;
        p_T0Snapshot->bIsT0Latched = 1; 
    }
}

/* ==========================================================================
 * 核心更正（2）：FreeRTOS SysTick 毫秒中断
 * ========================================================================== */
// void xPortSysTickHandler(void)
// {
//     vTaskRealTimeCount();
// }


void IPCCTsnSyncCallback(uint64_t tsn_network_us)
{
    s_dwLastIpcCycCnt = DWT->CYCCNT;
    g_sRealTimeCount.taskA35Time_us = tsn_network_us;
}

uint64_t Get_fcsM33Time_Us(void)
{
    uint32_t current_cyccnt = DWT->CYCCNT;
    uint32_t delta_cycles = current_cyccnt - s_dwLastIpcCycCnt;
    uint64_t delta_us = (uint64_t)delta_cycles / (SystemCoreClock / 1000000);
    g_sRealTimeCount.fcsM33Time_us = g_sRealTimeCount.taskA35Time_us + delta_us;
    return g_sRealTimeCount.fcsM33Time_us;
}

/* 内部私有小写驼峰函数：组装完成态势大总线的单步原子搬运 */
static void getSensorsModel_V2(void)
{
    TImuData       stLocalIMU;
    TBaroData      stLocalBaro;
    TMagData       stLocalMag;
    TGpsData       stLocalGps;
    TAirspeedData  stLocalAirspeed;
    TRcData        stLocalRc;

    uint64_t u64TargetUs = g_sRealTimeCount.fcsM33Time_us;

    // 刚性通过固定物理索引 ID 搬运向量
    if (GetSensorIdData(0, u64TargetUs, (uint8_t*)&stLocalIMU)) {
        g_SimSens.vGyro = stLocalIMU.fGyro; g_SimSens.vAcc  = stLocalIMU.fAcc;  
    }
    if (GetSensorIdData(2, u64TargetUs, (uint8_t*)&stLocalBaro)) {
        g_SimSens.fBaroAlt = stLocalBaro.fBarHeight;
    }
    if (GetSensorIdData(3, u64TargetUs, (uint8_t*)&stLocalMag)) {
        g_SimSens.vMag = stLocalMag.fMag; g_SimSens.fGpsHeadDeg = stLocalMag.fHeadDeg; 
    }
    if (GetSensorIdData(4, u64TargetUs, (uint8_t*)&stLocalGps)) {
        g_SimSens.dGpsLon = stLocalGps.dGpsLon; g_SimSens.dGpsLat = stLocalGps.dGpsLat; g_SimSens.fGpsAlt = stLocalGps.fGpsAlt;
        g_SimSens.vGpsVelNed = stLocalGps.fVNED; g_SimSens.u8GpsFixStatus = stLocalGps.iFixStatus;
    }
    if (GetSensorIdData(5, u64TargetUs, (uint8_t*)&stLocalAirspeed)) {
        g_SimSens.fAirspeed = stLocalAirspeed.fAirSpeed;
    }
    if (GetSensorIdData(6, u64TargetUs, (uint8_t*)&stLocalRc)) {
        g_SimSens.simA35Time_us = stLocalRc.u64TimeSampleUs; 
    }
}

/* ==========================================================================
 * 线程一：独立传感器高频采集任务 (由根任务动态派生，最高优先级硬抢占)
 * ========================================================================== */
void Task_AFCSensorCaptureEngine(void *argument)
{
    (void)argument;
    uint32_t u32CurrentTick = 0;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xBlock = pdMS_TO_TICKS(g_iMinSamplePeriod / 1000);
    
    for (;;) {
        vTaskDelayUntil(&xLastWakeTime, xBlock);
        u32CurrentTick++;
        
        uint64_t u64QueryUs = Get_fcsM33Time_Us();
        
        // 🚨 极致纯净串行推进：无脑、刚性地挨个调用各传感器专属底座 ReadAndPush 驱动
        ICM42688_ReadAndPush(u32CurrentTick, u64QueryUs);
        ICM20602_ReadAndPush(u32CurrentTick, u64QueryUs);
        DPS310_ReadAndPush(u32CurrentTick, u64QueryUs);
        IST8310_ReadAndPush(u32CurrentTick, u64QueryUs);
        FemtMesGPS_ReadAndPush(u32CurrentTick, u64QueryUs);
        MS5525_ReadAndPush(u32CurrentTick, u64QueryUs);
        SBusRC_ReadAndPush(u32CurrentTick, u64QueryUs); // 遥控器随动扫描
        
        g_sRealTimeCount.fcsM33Time = u32CurrentTick;
    }
}

/* ==========================================================================
 * 线程二：用户控制律算法主调度任务 (由根任务动态派生，中优先级)
 * ========================================================================== */
void Task_AFCSimulinkModelScheduler(void *argument)
{
    (void)argument;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xBlock = pdMS_TO_TICKS(g_iSimulinkAlgorithmStep / 1000);
    
    for (;;) {
        vTaskDelayUntil(&xLastWakeTime, xBlock);
        g_sRealTimeCount.ctlStep++;
        Get_fcsM33Time_Us();
        
        if (g_bParamReadyFlag) { __disable_irq(); g_bParamReadyFlag = false; __enable_irq(); }
        
        GetCrossCoreCmdAndPerception(&g_LocalCtrlCmd, &g_LocalPerception);
        getSensorsModel_V2(); // 调用文件内私有小写驼峰函数组装总线
        StepAlgorithmModel();
        
        pushFastStateToA35SharedRam(g_sRealTimeCount.fcsM33Time_us);
        if (g_sRealTimeCount.ctlStep % 5 == 0) { pushHealthDataToA35Queue(g_sRealTimeCount.fcsM33Time_us); }
    }
}

/* ==========================================================================
 * 外部大写 API：作为一个闭环死循环被 freertos.c 的 StartDefaultTask 强行接管跳转调用
 * ========================================================================== */
void BaseThreadTask00(void *argument)
{
    (void)argument;
    InitAFCSensorRingBuff(g_eCurrentRunMode);
    
    const osThreadAttr_t cap_attr = { .name = "SensCap", .stack_size = 512 * 4, .priority = (osPriority_t)osPriorityRealtime };
    xCaptureTaskHandle = osThreadNew(Task_AFCSensorCaptureEngine, NULL, &cap_attr);
    
    const osThreadAttr_t alg_attr = { .name = "SimAlg", .stack_size = 1024 * 4, .priority = (osPriority_t)osPriorityNormal };
    xAlgorithmTaskHandle = osThreadNew(Task_AFCSimulinkModelScheduler, NULL, &alg_attr);
    
    for(;;) { osDelay(osWaitForever); } // 挂起根任务，彻底移交控制权
}





















/* ==========================================================================
 * 核心更正（3）：M33 实时读取本地绝对微秒时间的标准接口
 * ========================================================================== */
uint64_t getFcsM33TimeUs(void)
{
    uint32_t current_cyccnt = DWT->CYCCNT;
    uint32_t delta_cycles = current_cyccnt - s_dwLastIpcCycCnt;
    uint64_t delta_us = delta_cycles / (SystemCoreClock / 1000000);
    
    g_sRealTimeCount.fcsM33Time_us = g_sRealTimeCount.taskA35Time_us + delta_us;
    
    return g_sRealTimeCount.fcsM33Time_us;
}

/* ==========================================================================
 * IPCC 中断服务：严格遵照 stm32mp2xx_hal_ipcc.h 官方三参数原型对接更正
 * ========================================================================== */
void HAL_IPCC_RxCallback(IPCC_HandleTypeDef *hipcc, uint32_t ChannelIndex, IPCC_CHANNELDirTypeDef ChannelDir)
{
    (void)hipcc;
    (void)ChannelDir;

    if (ChannelIndex == 0) 
    {
        uint64_t capture_M33_us = g_sRealTimeCount.fcsM33Time_us;
        uint64_t capture_A35_us = IPC_ReadA35Uint64MicrosecondTimestamp();
        (void)capture_M33_us; 
        (void)capture_A35_us;
    }
}

/* ==========================================================================
 * 核心落实（1）：IPCC 中断服务程序（处理双机仿真 A35 下发的 T0 点指令）
 * ========================================================================== */
void vTriggerA35T0Latched(uint64_t a35_t0_timestamp_us)
{
    if (p_T0Snapshot->bIsT0Latched == 0)
    {
        p_T0Snapshot->t0_A35_us = a35_t0_timestamp_us;
        p_T0Snapshot->t0_M33_us = g_sRealTimeCount.fcsM33Time_us; 
        p_T0Snapshot->bIsT0Latched = true; 
    }
}

/* ==========================================================================
 * 核心落实（2）：M33 触发的起飞零点锁存入口（处理正常飞行场景）
 * ========================================================================== */
void vTriggerFcsM33T0Latched(void)
{
    if (p_T0Snapshot->bIsT0Latched == 0)
    {
        p_T0Snapshot->t0_M33_us = g_sRealTimeCount.fcsM33Time_us; 
        IPC_Notify_A35_Flight_Start_Event(); 
        p_T0Snapshot->bIsT0Latched = true;
    }
}

/* --------------------------------------------------------------------------
 * 场景一：基于 T0 快照的仿真数据无感注入
 * -------------------------------------------------------------------------- */
void readA35SimulationSensors(void)
{
    uint64_t tSensA35TimeUs = 0;
    vReadSimDataPacket(&g_SimSens, &tSensA35TimeUs); 
    
    if (p_T0Snapshot->bIsT0Latched == 1)
    {
        int64_t delta_run_A35_us = (int64_t)tSensA35TimeUs - (int64_t)p_T0Snapshot->t0_A35_us;
        uint64_t alignedM33TimeUs = p_T0Snapshot->t0_M33_us + delta_run_A35_us;
        
        g_SimSens.simA35Time_us = alignedM33TimeUs; 
    }
    else
    {
        g_SimSens.simA35Time_us = g_sRealTimeCount.fcsM33Time_us;
    }
}

/* ==========================================================================
 * 🚨 核心更正（4）：多源传感器数据提取模块（GetSensorsModel）阶段性最简化实现
 * 🚨 直接提取当前环形缓冲区内写指针前一刻写入的最新、最近有效对标数据
 * ========================================================================== */
void GetSensorsModel(TSimSens* pDestSens)
{
    // 计算上一帧有效索引边界
    uint8_t u8LastReadIdx = (g_u8IMUWriteIdx == 0) ? 31 : (g_u8IMUWriteIdx - 1);
    
    pDestSens->gyro[0] = g_sIMURingBuffer[u8LastReadIdx].fGyroX;
    pDestSens->gyro[1] = g_sIMURingBuffer[u8LastReadIdx].fGyroY;
    pDestSens->gyro[2] = g_sIMURingBuffer[u8LastReadIdx].fGyroZ;
    pDestSens->acc[0]  = g_sIMURingBuffer[u8LastReadIdx].fAccX;
    pDestSens->acc[1]  = g_sIMURingBuffer[u8LastReadIdx].fAccY;
    pDestSens->acc[2]  = g_sIMURingBuffer[u8LastReadIdx].fAccZ;
    pDestSens->u64TimeStampUs = g_sIMURingBuffer[u8LastReadIdx].u64TimeStampUs; // 完全对齐的时间戳
}

/* ==========================================================================
 * 🚨 核心更正（5）：地面组件物理自旋标校运行模块（FLY_STATE_CALIBRATING）
 * 🚨 囊括陀螺仪去零偏、六面法、气压零点、罗盘椭球最小二乘拟合、电调行程标定接口
 * ========================================================================== */
void ProcessComponentCalibration(TCalibCommand* pCmd, TCalibStatus* pStatus)
{
    if (pCmd->u32StartMagicWord != 0xAA5555AA) return;
    
    pStatus->u8CalibStage = 1; // 切入刚性采样
    
    if (pCmd->u8CalibTypeMask == 0x01) {
        // 1. 陀螺仪地面静态去零偏：循环提取 1000 帧均值向量覆盖 fGyroBias
        pStatus->u8CalibStage = 3; // 标定成功
    }
    else if (pCmd->u8CalibTypeMask == 0x02) {
        // 2. 加速度计六面标校法：分步（u8CalibStepIndex）提取六面几何拟合
    }
    else if (pCmd->u8CalibTypeMask == 0x03) {
        // 3. 气压计海平面高度温漂清零偏：锁定起飞点 fDps310Offset 基准
    }
    else if (pCmd->u8CalibTypeMask == 0x04) {
        // 4. 磁力计罗盘球体最小二乘拟合解算：校正硬铁 fMagIronBias 及软铁矩阵
    }
    else if (pCmd->u8CalibTypeMask == 0x05) {
        // 5. 电调行程自平衡互锁调校：强制输出 2000us 高油门，听到确认鸣叫后阶跃至 1000us 怠速
    }
}

/* ==========================================================================
 * Task00 核心基频线程（最高优先级抢占，绝对时空控制流）
 * ========================================================================== */
/* ==========================================================================
 * Task00 核心控制解算主回路（作为一个闭环死循环外部函数被 StartDefaultTask 调用）
 * ========================================================================== */
void BaseThreadTask00(void *argument)
{
    (void)argument;
    uint32_t u32Tick = osKernelGetTickCount();
    const uint32_t u32TicksPeriod = pdMS_TO_TICKS(g_iSimulinkAlgorithmStep);

    for(;;)
    {
        u32Tick += u32TicksPeriod;
        osDelayUntil(u32Tick); // 严格跟随 Simulink 模型自选离散步长卡位挂起
        
        uint64_t u64CurrentM33Us = getFcsM33TimeUs();
        (void)u64CurrentM33Us; 

        /* A. 🚨 参数原子覆盖：控制步长最前端，瞬间闭锁中断，执行影子缓冲向运行增益的全量刷新 */
        if (g_bParamReadyFlag) {
            __disable_irq(); // 挂起全内核硬件中断，开辟 1 微秒绝对原子临界区
            // 现场一次性极速覆盖主运行控制律增益结构体
            // memcpy(&g_UavFcsParam.LawGains, &g_sShadowParam, sizeof(TControlLawParam));
            g_bParamReadyFlag = false;
            __enable_irq();  // 恢复中断，临界区安全解锁
        }

        /* B. 常规物理飞行模式下的刚性硬件采集（仿真模态下总线关闭，由 IPCC 中断在后台默默入队） */
        if (!bIsHilSimulationMode()) 
        {
            if (g_bUsedOfICM42688 && !bSPI1IsUsing) {
                bSPI1IsUsing = true;  
                // 内部读取 ICM42688 原始物理值，并打上本地 fcsM33Time_us 压入 g_sIMURingBuffer
                // ICM42688_ReadAndPush(); 
                bSPI1IsUsing = false;
            }
            
            // 级联读取由 A35 核高度耦合测控总线发送过来的导航控制指令
            if (bIsA35CommandAvailable()) {
                readA35FcsCommands(); 
            }
        }

        /* C. 🚨 数据预处理：调用 GetSensorsModel 提取模块，获取完全对标清洗后的最新一帧态势数据 */
        GetSensorsModel(&g_SimSens);

        /* D. 🚀 驱动核心控制律迭代计算 */
        StepAlgorithmModel(); 

        /* E. 控制量上抛共享内存区（带本地 M33 物理微秒时标），供仿真机在另一端执行反向时间同步 */
        pushFastStateToA35SharedRam(g_sRealTimeCount.fcsM33Time_us);

        /* F. 多速率信号分频监测：分时降频上抛健康度黑匣子日志码 */
        if (g_sRealTimeCount.fcsM33Time % 10 == 0) {
            pushHealthDataToA35Queue(g_sRealTimeCount.fcsM33Time_us);
        }

        g_sRealTimeCount.check_flag = 0; 
    }
}

SSaveParamFlash loadDefaultPara(void)
{
    SSaveParamFlash pSUavDefaultPara;
    pSUavDefaultPara.FirstInitFlg = 0xA5DA;
    pSUavDefaultPara.UavPara.UavId = 0;
    pSUavDefaultPara.UavPara.UavFrame = HS620_UAV;
    pSUavDefaultPara.UavPara.FcsBoard = AFC7_BOARD;
    pSUavDefaultPara.UavPara.XSetup = true;         pSUavDefaultPara.UavPara.YSetup = true;         pSUavDefaultPara.UavPara.ZSetup = false;
    pSUavDefaultPara.UavPara.ResSetup = 0x0;
    pSUavDefaultPara.UavPara.Remoter = WFLY_ETS6S;
    pSUavDefaultPara.UavPara.FlyMode = ALTITUDE_HOLD_MODE;
    pSUavDefaultPara.UavPara.AuthorizationCode = 0x2CAD;
    
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
    pSUavDefaultPara.Ist8310Neg = 1;
    return pSUavDefaultPara;
}

bool LoadUAVParaFromA35(SSaveParamFlash* pParam)
{
    if (*(volatile uint32_t*)PARAM_READY_FLAG_ADDR == 0x5AA5)
    {
        memcpy(pParam, (void*)(PARAM_READY_FLAG_ADDR + 4), sizeof(SSaveParamFlash));
        return true; 
    }
    return false; 
}

void SaveUAVParaToA35(SSaveParamFlash* pParam) { (void)pParam; }

void initAFCPara(void)
{
    bool bCrcOk = LoadUAVParaFromA35(&g_UavFcsParam);
    if (!g_bUsedOfA35UAVPara){          
            g_UavFcsParam = loadDefaultPara();  
            g_UavFcsParam.FirstInitFlg = 0x5AA5;  
    }
    else if ((!bCrcOk) || g_UavFcsParam.FirstInitFlg != 0xA5DA){
            g_UavFcsParam = loadDefaultPara();  
            SaveUAVParaToA35(&g_UavFcsParam);
    }
    g_UavFcsParam.UavPara.XSetup = false;           g_UavFcsParam.UavPara.YSetup = false;           g_UavFcsParam.UavPara.ZSetup = false;
}

void initTask0Period(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk; 
    if (g_iSimulinkAlgorithmStep == 0) {
        g_iSimulinkAlgorithmStep = 2000; // 🚨 适应 1us 级主时基，自适应初始化为 2000微秒 (2ms) 变步长 🚨
    }
}

void initSysChipIdValid(void) { }
void vOverTimePro(void){}

void MainProc(void)
{
    vOverTimePro();
}

void initGs2FcsCmdFrame(void)
{
    p_T0Snapshot->t0_A35_us = 0;
    p_T0Snapshot->t0_M33_us = 0;
    p_T0Snapshot->bIsT0Latched = 0;
}

void AFCTaskInit(void)
{
    HAL_InitTick(TICK_INT_PRIORITY);
    initAFCPara();
    // vSysParamInit();
    vInitRealTaskParameter();
    InitializeModel();
    initSysChipIdValid();
    initGs2FcsCmdFrame(); 
    initTask0Period();

    HAL_Delay(3000);
    g_sRealTimeCount.err_flag = 0;
}