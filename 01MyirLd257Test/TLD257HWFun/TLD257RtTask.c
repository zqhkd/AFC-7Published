/******************** (C) COPYRIGHT 2026 AFC Tech Co.*************************
 * 作    者  ： 曾庆华
 * 文 件 名  ： TLD257RtTask.C
 * 版    本  ： V1.0.260330 
 * 描    述  ： 米尔开发板的实时任务线程调度函数
 * 官    网  ： www.acecreator.com
 * 淘    宝  ： acecreator.taobao.com
 * 公 众 号  ： 飞行控制与仿真
*****************************************************************************/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <time.h>
#include <unistd.h>
#include <sched.h>
#include <stddef.h>
#include <sys/mman.h>
#include "A35CommonFun.h"
#include "TLD257HwFun.h"

/* --- 你的业务函数 --- */
void RealSimCommunicationTask(void) { }
void UAVsInfExchangeTask(void)    { }
void RPMsgeM33(void)             { }

/* --- 你的观测变量 --- */
static atomic_int      g_TaskRun = 0;
static atomic_long     g_LastJitterNs = 0;
static atomic_long     g_MaxJitterNs = 0;
static atomic_ullong   g_TotalJitterNs = 0;
static atomic_ullong   g_CycleCount = 0;

/* --- 你的初始化 --- */
void InitRealTaskHardeningEnv(void){
    mlockall(MCL_CURRENT | MCL_FUTURE);
    system("echo performance > /sys/devices/system/cpu/cpu1/cpufreq/scaling_governor");
    system("echo 0 > /proc/sys/vm/swappiness");
}

/* --- 你的核心线程（完全不变！） --- */
void *A35RtTaskFunc(void *arg) {
    struct timespec solveStep, now;
    long period = 1000000L; /* 1ms = 10e6 ns */

    atomic_store(&g_TaskRun, 1);
    clock_gettime(CLOCK_MONOTONIC, &solveStep);

    while (atomic_load(&g_TaskRun)) {
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &solveStep, NULL);

        RealSimCommunicationTask();
        UAVsInfExchangeTask();
        RPMsgeM33();

        clock_gettime(CLOCK_MONOTONIC, &now);

        // ============= 我只修复这里 =============
        int64_t j = (int64_t)(now.tv_sec - solveStep.tv_sec) * 1000000000LL
                  + (int64_t)(now.tv_nsec - solveStep.tv_nsec);
        long abs_j = llabs(j);

        atomic_store(&g_LastJitterNs, abs_j);
        atomic_fetch_add(&g_CycleCount, 1);
        atomic_fetch_add(&g_TotalJitterNs, abs_j);

        if (abs_j > atomic_load(&g_MaxJitterNs)) {
            atomic_store(&g_MaxJitterNs, abs_j);
        }

        // 下一周期
        solveStep.tv_nsec += period;
        if (solveStep.tv_nsec >= 1000000000L) {
            solveStep.tv_nsec -= 1000000000L;
            solveStep.tv_sec += 1;
        }
    }
    return NULL;
}

/* --- 你的观测主函数（完全不变） --- */
int TLD257RtTask(void) {
    atomic_store(&g_CycleCount, 0);
    atomic_store(&g_MaxJitterNs, 0);
    atomic_store(&g_TotalJitterNs, 0);
    atomic_store(&g_TaskRun, 0);

    InitRealTaskHardeningEnv();

    int ret = CreateAndStartTask(A35RtTaskFunc, "AFC7_CORE", 0, 99, SCHED_FIFO, 1, NULL);
    if (ret != 0) return -1;

    printf("\n---> AFC-7 实时性观测启动 (采样周期: 500ms) <---\n");
    printf(" 序   | 累计周期 | 当前 Jitter (ns) | 峰值 Jitter (ns)\n");
    printf("----------------------------------------------------\n");

    for (int i = 0; i < 20; i++) {  // 采样次数20次，每个周期500ms, 共计10s
        usleep(500000);            // 采样周期为500ms。注意1ms = 10e3 us = 10e6 ns
        long cur_j = atomic_load(&g_LastJitterNs);
        long max_j = atomic_load(&g_MaxJitterNs);
        unsigned long long count = atomic_load(&g_CycleCount);

        printf(" [%03d] | %8llu | %15ld | %15ld\n", i, count, cur_j, max_j);
    }

    atomic_store(&g_TaskRun, 0);
    usleep(100000);

    unsigned long long total_count = atomic_load(&g_CycleCount);
    if (total_count > 0) {
        long final_max = atomic_load(&g_MaxJitterNs);
        double avg_jitter = (double)atomic_load(&g_TotalJitterNs) / total_count;

        printf("----------------------------------------------------\n");
        printf("📊 AFC-7 核心线程实时性统计报告:\n");
        printf("  - 运行总次数:    %llu 次，调度周期： %.2f ms\n", total_count, 20.0*500/total_count);
        printf("  - 平均绝对偏差:  %.2f ns (%.3f us)\n", avg_jitter, avg_jitter / 1000.0);
        printf("  - 历史最大偏差:  %ld ns (%.3f us)\n", final_max, final_max / 1000.0);
        printf("----------------------------------------------------\n");
    }

    WaitAnyKey("按 [回车] 返回菜单...");
    return 0;
}
/************************ (C) COPYRIGHT ACG co. *****END OF FILE****/