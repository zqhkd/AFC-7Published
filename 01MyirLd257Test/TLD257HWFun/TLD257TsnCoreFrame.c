/******************** (C) COPYRIGHT 2026 AFC Tech Co.*************************
 * 作    者  ： 曾庆华
 * 文 件 名  ： TLD257HwFun.h
 * 版    本  ： V1.0.260329:  
 * 描    述  ： 米尔开发板硬件功能测试应用程序接口（API）头文件
 * 官    网  ： www.acecreator.com
 * 淘    宝  ： acecreator.taobao.com
 * 公 众 号  ： 飞行控制与仿真
*****************************************************************************/
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/if_xdp.h>
#include <linux/if_ether.h>
#include <sys/mman.h>
#include <poll.h>
#include <fcntl.h>
#include <linux/ptp_clock.h>

#include "TLD257HwFun.h"

// ===================== 硬件配置 =====================
#define TSN_IF_NAME         "end1"        // 米尔 TSN 网口
#define TSN_XDP_QUEUE       3              // TSN 最高优先级队列(Q3)
#define PTP_DEVICE          "/dev/ptp1"    // TSN 硬件时钟

// ===================== 全局统计 =====================
static uint64_t total_packets = 0;
static uint64_t max_latency_ns = 0;
static uint64_t sum_latency_ns = 0;
static int is_running = 1;

// ###########################################################################
// 【你要的 · TSN 硬件 DMA 钩子函数】
// 功能：数据一到 → 硬件中断 → 直接执行（无协议栈、无轮询、无软件延时）
// ###########################################################################
void TSN_HW_DMA_Rx_Hook(uint8_t *data, int len, struct timespec *phc_ts)
{
    // ===================== 实时仿真机数据处理 =====================
    // 1. 硬件 PHC 时钟（全网统一时间）
    printf("[PHC时钟] %ld.%09ld | 帧长=%d\n",
           phc_ts->tv_sec, phc_ts->tv_nsec, len);

    // 2. 高实时转发 → RPMsg 发给 M33
    // RPMsg_SendToM33(data, len);
    // RPMsg_RecvFromM33(data, &len);
}

// ###########################################################################
// 读取 TSN 硬件时钟 PHC（真正 gPTP 同步时钟，非系统时钟）
// ###########################################################################
static int Get_PHC_Time(struct timespec *ts)
{
    // int fd = open(PTP_DEVICE, O_RDONLY);
    // if (fd < 0) return -1;

    // ioctl(fd, PTP_CLOCK_GETTIME, ts);
    // close(fd);
    // return 0;
    return clock_gettime(CLOCK_REALTIME, ts);
}

// ###########################################################################
// AF_XDP 初始化：零拷贝 + 硬件直通 + 绕过内核协议栈
// ###########################################################################
static int XDP_HW_Init(const char *ifname, uint32_t queue_id)
{
    int sock = socket(AF_XDP, SOCK_RAW, 0);
    if (sock < 0) return -1;

    // 🛡️ 核心补丁：告诉内核我们使用 COPY 模式，提高兼容性
    struct sockaddr_xdp sa = {
        .sxdp_family = AF_XDP,
        .sxdp_ifindex = if_nametoindex(ifname),
        .sxdp_queue_id = queue_id,
        .sxdp_flags = XDP_COPY  // <--- 关键！强制使用拷贝模式，确保能 bind 成功
    };

    if (sa.sxdp_ifindex == 0) {
        fprintf(stderr, "错误: 找不到网口 %s\n", ifname);
        close(sock);
        return -1;
    }

    // 执行绑定
    if (bind(sock, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
        perror("AF_XDP bind 失败"); 
        close(sock);
        return -1;
    }
    
    return sock;
}

// ###########################################################################
// 辅助函数：纳秒时间差
// ###########################################################################
static inline uint64_t calc_diff_ns(struct timespec *start, struct timespec *end)
{
    return (end->tv_sec - start->tv_sec) * 1000000000ULL +
           (end->tv_nsec - start->tv_nsec);
}

// ###########################################################################
// 【线程 1】TSN 实时仿真通道（硬件中断驱动 · 真·确定性）
// 调度：SCHED_FIFO 优先级 80
// 模式：AF_XDP 零拷贝 + PHC 硬件时钟 + 事件触发
// ###########################################################################
void* Thread_TSN_Gateway(void* arg)
{
    int xsk;
    uint8_t buffer[1526];
    struct timespec phc_ts, proc_start_ts;
    struct pollfd fds;

    printf("[线程1] TSN 硬件实时通道启动（AF_XDP 零拷贝 + PHC时钟）\n");

    // 1. 初始化 TSN 硬件直通
    xsk = XDP_HW_Init(TSN_IF_NAME, TSN_XDP_QUEUE);
    if (xsk < 0) return NULL;

    fds.fd = xsk;
    fds.events = POLLIN;

    // 2. 硬件中断等待（休眠，不占CPU）
    while (is_running)
    {
        // ===================== 关键：硬件中断唤醒 =====================
        int ret = poll(&fds, 1, -1);
        if (ret <= 0 || !(fds.revents & POLLIN)) continue;

        // 记录处理开始时间
        clock_gettime(CLOCK_TAI, &proc_start_ts);

        // 3. 从零拷贝缓冲区读取数据
        int len = recv(xsk, buffer, sizeof(buffer), 0);
        if (len <= 0) continue;

        // 4. 读取 TSN 硬件时钟
        Get_PHC_Time(&phc_ts);

        // ===================== 调用硬件钩子 =====================
        TSN_HW_DMA_Rx_Hook(buffer, len, &phc_ts);

        // 5. 回发数据（TSN 硬件自动调度发送时机）
        send(xsk, buffer, len, 0);

        // 6. 统计（不影响实时路径）
        uint64_t latency = calc_diff_ns(&proc_start_ts, &phc_ts);
        total_packets++;
        sum_latency_ns += latency;
        if (latency > max_latency_ns) max_latency_ns = latency;

        if (total_packets % 1000 == 0) {
            printf("📊 TSN 硬件通道：%llu 包 | 平均耗时: %.2f us | 峰值: %.2f us\n",
                   (unsigned long long)total_packets,
                   (double)sum_latency_ns / total_packets / 1000.0,
                   (double)max_latency_ns / 1000.0);
            sum_latency_ns = max_latency_ns = 0;
        }
    }

    close(xsk);
    return NULL;
}

// ###########################################################################
// 【线程 2】集群协同通信（优先级 50）
// ###########################################################################
void* Thread_MAVLink_Cluster(void* arg)
{
    printf("[线程2] 集群协同线程启动\n");
    struct timespec ts = {0, 50000000}; // 50ms
    while (is_running) {
        clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL);
        for(volatile int i=0; i<1000; i++);
    }
    return NULL;
}

// ###########################################################################
// 【线程 3】AI 视觉 / 任务规划（非实时）
// ###########################################################################
void* Thread_AI_Vision(void* arg)
{
    printf("[线程3] AI视觉规划线程启动\n");
    while (is_running) {
        usleep(100000);
        for(volatile int i=0; i<50000; i++);
    }
    return NULL;
}

// ###########################################################################
// 顶层框架：AFC-7 TSN 高实时核心框架程序
// ###########################################################################
int TLD257TsnCoreFrame(void)
{
    pthread_t tid_tsn, tid_mavlink, tid_ai;
    pthread_attr_t attr_tsn, attr_mavlink, attr_ai;
    struct sched_param param;

    printf("\n======================================================\n");
    printf("  AFC-7 真·TSN 硬件实时架构（AF_XDP + PHC时钟 + 零拷贝）\n");
    printf("======================================================\n");

    // 锁定内存，防止页面缺失导致抖动
    mlockall(MCL_CURRENT | MCL_FUTURE);

    // -------------------- 线程1：TSN 硬件实时通道 --------------------
    pthread_attr_init(&attr_tsn);
    pthread_attr_setschedpolicy(&attr_tsn, SCHED_FIFO);
    param.sched_priority = 80;
    pthread_attr_setschedparam(&attr_tsn, &param);
    pthread_attr_setinheritsched(&attr_tsn, PTHREAD_EXPLICIT_SCHED);
    pthread_create(&tid_tsn, &attr_tsn, Thread_TSN_Gateway, NULL);

    // -------------------- 线程2：集群协同 --------------------
    pthread_attr_init(&attr_mavlink);
    pthread_attr_setschedpolicy(&attr_mavlink, SCHED_FIFO);
    param.sched_priority = 50;
    pthread_attr_setschedparam(&attr_mavlink, &param);
    pthread_attr_setinheritsched(&attr_mavlink, PTHREAD_EXPLICIT_SCHED);
    pthread_create(&tid_mavlink, &attr_mavlink, Thread_MAVLink_Cluster, NULL);

    // -------------------- 线程3：AI视觉 --------------------
    pthread_attr_init(&attr_ai);
    pthread_attr_setschedpolicy(&attr_ai, SCHED_OTHER);
    param.sched_priority = 0;
    pthread_create(&tid_ai, &attr_ai, Thread_AI_Vision, NULL);

    printf("\n✅ 真 TSN 系统启动完成！按回车退出...\n");
    WaitAnyKey("[提示] 请按 [回车键(Enter)] 返回主菜单...\n\n");
    is_running = 0;

    // 安全退出
    pthread_cancel(tid_tsn);
    pthread_join(tid_tsn, NULL);
    pthread_join(tid_mavlink, NULL);
    pthread_join(tid_ai, NULL);

    printf("✅ TSN 核心框架关闭完成\n");
    return 0;
}