/******************** (C) COPYRIGHT 2026 AFC Tech Co.*************************
 * 作    者  ： 曾庆华
 * 文 件 名  ： A35CommonFun.C
 * 版    本  ： AFC-7.A35.V1.0.260329:  
 * 描    述  ：OpenSTLinux系统共用函数
 * 官    网  ：www.acecreator.com
 * 淘    宝  ：acecreator.taobao.com
 * 公 众 号  ：飞行控制与仿真
*****************************************************************************/
#define _GNU_SOURCE
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "TLD257HwFun.h"

void WaitAnyKey(char *tmps) {
    // 使用传入的字符串作为提示
    if (tmps != NULL) {
        printf("\n%s", tmps);
    } else {
        printf("\n[提示] 请按 [回车键] 继续...");
    }
    fflush(stdout);

    // 1. 强力清空缓冲区 (吃掉之前的残留)
    int c;
    while ((c = getchar()) != '\n' && c != EOF);

    // 2. 阻塞等待用户敲击回车
    getchar();

    // printf("[系统] 指令已确认，正在处理...\n");
}

/**
 * @brief 创建并启动绑定到特定 CPU 的实时任务
 * @param cpuId 绑定的核心编号 (对于 MP257 A35 核，推荐使用 1)
 */
int CreateAndStartTask(void *(*taskFunc)(void *), const char *taskName, 
                       int stackSize, int priority, int schedpolicy, 
                       int cpuId, void *arg) {
    pthread_t thread;
    pthread_attr_t attr;
    struct sched_param param;
    cpu_set_t cpuset;

    // 1. 初始化线程属性
    if (pthread_attr_init(&attr)) {
        perror("初始化属性失败");
        return -1;
    }

    // 2. 核心绑定逻辑 (新增)
    if (cpuId >= 0) {
        CPU_ZERO(&cpuset);
        CPU_SET(cpuId, &cpuset);
        if (pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpuset)) {
            perror("设置核心绑定失败");
            pthread_attr_destroy(&attr);
            return -1;
        }
    }

    // 3. 设置堆栈大小
    if (stackSize > 0) {
        if (stackSize < PTHREAD_STACK_MIN) stackSize = PTHREAD_STACK_MIN;
        pthread_attr_setstacksize(&attr, stackSize);
    }

    // 4. 设置实时调度与优先级 (显式调度)
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&attr, schedpolicy);
    param.sched_priority = priority;
    pthread_attr_setschedparam(&attr, &param);

    // 5. 创建线程
    if (pthread_create(&thread, &attr, taskFunc, arg)) {
        perror("创建任务线程失败");
        pthread_attr_destroy(&attr);
        return -1;
    }

    // 6. 分离线程以便自动回收资源
    pthread_detach(thread);
    pthread_attr_destroy(&attr);

    printf("任务 '%s' 成功绑定到 CPU %d, 优先级 %d\n", taskName, cpuId, priority);
    return 0;
}
/************************ (C) COPYRIGHT ACG co. *****END OF FILE****/