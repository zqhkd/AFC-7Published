/******************** (C) COPYRIGHT 2026 AFC Tech Co.*************************
 * 作    者  ： 曾庆华
 * 文 件 名  ： TLD257EthBasic.C
 * 版    本  ： V1.0.260329:  
 * 描    述  ： 米尔开发板的网络基本功能测试函数
 * 官    网  ： www.acecreator.com
 * 淘    宝  ： acecreator.taobao.com
 * 公 众 号  ： 飞行控制与仿真
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "TLD257HwFun.h"

// ---------------------------------------------------------
// 预设参数
// ---------------------------------------------------------
#define PC_TARGET_IP   "192.168.1.2" 
#define PC_TARGET_PORT 8080
#define LOCAL_BIND_PORT 9090    // 必须与 SSCOM 远程主机端口一致

int TLD257EthBasic(void) {
    printf("\n---> 进入 [模块 4：ENET2 普通千兆网 UDP] 测试 <---\n");

    int sockfd;
    struct sockaddr_in servaddr, myaddr; // 增加 myaddr 用于绑定
    char rx_buf[1024];
    const char *heartbeat_msg = "[AFC-7] Heartbeat: ENET2 Link Active\n";

    // 1. 创建 UDP Socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("[错误] Socket 创建失败");
        return -1;
    }

    // =========================================================
    // 【新增：明确绑定本地端口】
    // 只有绑定了 LOCAL_BIND_PORT (9090)，开发板才能收到发往该端口的包
    // =========================================================
    memset(&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY); // 监听所有网口地址
    myaddr.sin_port = htons(LOCAL_BIND_PORT);

    if (bind(sockfd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
        perror("[错误] Bind 端口失败");
        close(sockfd);
        return -1;
    }
    printf("[系统] 成功绑定本地端口: %d\n", LOCAL_BIND_PORT);
    // =========================================================

    // 2. 关键：设置非阻塞模式
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

    // 3. 配置目标地址 (PC 地面站)
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PC_TARGET_PORT);
    inet_pton(AF_INET, PC_TARGET_IP, &servaddr.sin_addr);

    printf("[系统] UDP 模块初始化完成。目标 PC: %s:%d\n", PC_TARGET_IP, PC_TARGET_PORT);
    printf("[提示] 请在 PC 端发送数据至 192.168.1.10:%d。发送 'q' 退出。\n", LOCAL_BIND_PORT);

    int running = 1;
    int tick_50ms = 0;

    // 4. 异步通讯主循环
    while (running) {
        // --- 模拟飞控心跳 ---
        if (tick_50ms % 20 == 0) {
            sendto(sockfd, heartbeat_msg, strlen(heartbeat_msg), 0, 
                   (const struct sockaddr *)&servaddr, sizeof(servaddr));
            printf("[发送] 心跳包已推送至地面站...\n");
        }

        // --- 尝试异步接收地面站数据 ---
        struct sockaddr_in from_addr;
        socklen_t addr_len = sizeof(from_addr);
        int n = recvfrom(sockfd, rx_buf, sizeof(rx_buf) - 1, 0, 
                         (struct sockaddr *)&from_addr, &addr_len);

        if (n > 0) {
            rx_buf[n] = '\0';
            printf("[接收] 长度: %d, 内容: %s (来自: %s)\n", n, rx_buf, inet_ntoa(from_addr.sin_addr));

            // 回显逻辑
            sendto(sockfd, rx_buf, n, 0, (const struct sockaddr *)&from_addr, addr_len);

            // 退出判定
            if (rx_buf[0] == 'q' || rx_buf[0] == 'Q') {
                printf("[系统] 检测到退出指令，准备返回...\n");
                running = 0;
            }
        }

        usleep(50000); 
        tick_50ms++;
    }

    close(sockfd);
    printf("---> ENET2 测试结束，返回主菜单 <---\n\n");
    return 0;
}
/************************ (C) COPYRIGHT ACG co. *****END OF FILE****/