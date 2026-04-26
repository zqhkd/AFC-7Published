/******************** (C) COPYRIGHT 2026 AFC Tech Co.*************************
 * 作    者  ： 曾庆华
 * 文 件 名  ： TLD257EthTsn.C
 * 版    本  ： V1.0.260329
 * 描    述  ： 米尔开发板的TSN时间戳功能测试函数
 * 官    网  ： www.acecreator.com
 * 淘    宝  ： acecreator.taobao.com
 * 公 众 号  ： 飞行控制与仿真
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#ifndef _LINUX_IF_H
#define _LINUX_IF_H  /* 阻止 linux/if.h 被包含，强制使用 net/if.h */
#endif
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* 再包含 Linux 内核特定的时间戳定义 */
#include <net/if.h>
#include <linux/net_tstamp.h> 
#include <linux/errqueue.h>
#include <linux/sockios.h> 

#include <time.h>

// 引用统一的模块头文件
#include "TLD257HwFun.h"

#define TARGET_IP   "192.168.0.2" 
#define TARGET_PORT 8080

int TLD257EthTsn(void) {
    printf("\n---> 进入 [模块 5：ENET1 TSN 硬件时间戳] 测试 <---\n");

    int sockfd;
    struct sockaddr_in addr;
    struct ifreq ifr;
    struct hwtstamp_config hwconfig;
    int timestamp_flags;

    // 1. 创建 UDP Socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("[错误] Socket 创建失败");
        return -1;
    }

    // 2. 核心：通过 ioctl 开启 end1 网卡的硬件打桩模式
    memset(&ifr, 0, sizeof(ifr));
    memset(&hwconfig, 0, sizeof(hwconfig));
    strncpy(ifr.ifr_name, "end1", IFNAMSIZ);

    hwconfig.tx_type = HWTSTAMP_TX_ON;          // 开启发送硬件打桩
    hwconfig.rx_filter = HWTSTAMP_FILTER_ALL;    // 捕获所有接收包的硬件时间
    ifr.ifr_data = (char *)&hwconfig;

    if (ioctl(sockfd, SIOCSHWTSTAMP, &ifr) < 0) {
        perror("[警告] SIOCSHWTSTAMP 失败 (请确认以 root 运行或驱动支持)");
        // 即使失败也尝试继续，部分驱动可能已默认开启
    }

    // 3. 开启 Socket 层级的时间戳功能
    timestamp_flags = SOF_TIMESTAMPING_TX_HARDWARE | 
                      SOF_TIMESTAMPING_RX_HARDWARE | 
                      SOF_TIMESTAMPING_RAW_HARDWARE;
    
    if (setsockopt(sockfd, SOL_SOCKET, SO_TIMESTAMPING, &timestamp_flags, sizeof(timestamp_flags)) < 0) {
        perror("[错误] setsockopt 失败");
        close(sockfd);
        return -1;
    }

    // 4. 配置目标地址
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(TARGET_PORT);
    inet_pton(AF_INET, TARGET_IP, &addr.sin_addr);

    printf("[系统] TSN 硬件打桩已就绪。正在每 1s 采集一组数据包离开 MAC 的精确时刻...\n");

    int count = 0;
    while (count < 10) {
        const char *msg = "AFC-7 TSN SYNC";
        if (sendto(sockfd, msg, strlen(msg), 0, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            perror("[错误] 发送失败");
        }

        // 5. 从错误队列抓取刚才发出去的包的【硬件出发时间】
        struct msghdr msgh;
        struct iovec iov;
        char control[256];
        char dummy_data[256];
        
        iov.iov_base = dummy_data;
        iov.iov_len = sizeof(dummy_data);
        msgh.msg_iov = &iov;
        msgh.msg_iovlen = 1;
        msgh.msg_control = control;
        msgh.msg_controllen = sizeof(control);
        msgh.msg_name = NULL;
        msgh.msg_namelen = 0;

        // 稍微延时等待硬件处理完毕
        usleep(10000); 

        if (recvmsg(sockfd, &msgh, MSG_ERRQUEUE | MSG_DONTWAIT) > 0) {
            struct cmsghdr *cmsg;
            for (cmsg = CMSG_FIRSTHDR(&msgh); cmsg != NULL; cmsg = CMSG_NXTHDR(&msgh, cmsg)) {
                if (cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SO_TIMESTAMPING) {
                    struct scm_timestamping *ts = (struct scm_timestamping *)CMSG_DATA(cmsg);
                    // ts->ts[2] 为 Raw Hardware Timestamp
                    printf("[%02d] 发包时刻(HW): %ld.%09ld s\n", count+1, ts->ts[2].tv_sec, ts->ts[2].tv_nsec);
                    count++;
                }
            }
        }
        sleep(1);
    }

    close(sockfd);
    printf("---> 已完成 TSN 测试 <---\n");
    WaitAnyKey("[提示] 请按 [回车键(Enter)] 返回主菜单...");
    return 0;
}
/************************ (C) COPYRIGHT ACG co. *****END OF FILE****/