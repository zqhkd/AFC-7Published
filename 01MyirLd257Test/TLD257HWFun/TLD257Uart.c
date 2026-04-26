/******************** (C) COPYRIGHT 2026 AFC Tech Co.*************************
 * 作    者  ： 曾庆华
 * 文 件 名  ： TLD257Uart.C
 * 版    本  ： V1.0.260329 
 * 描    述  ： 米尔开发板的串口（J15）通信测试函数
 * 官    网  ： www.acecreator.com
 * 淘    宝  ： acecreator.taobao.com
 * 公 众 号  ： 飞行控制与仿真
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include "TLD257HwFun.h"

#define UART_DEV_PATH "/dev/ttySTM0" 

int TLD257Uart(void) {
    printf("\n---> 进入 [模块 3：A35 调试串口 (J15) 双向通讯] 测试 <---\n");
    
    // 1. 打开串口设备
    int fd = open(UART_DEV_PATH, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0) {
        printf("[错误] 无法打开串口设备节点: %s\n", UART_DEV_PATH);
        return -1;
    }

    // 2. 工业级串口参数配置 (115200, 8N1, 绝对纯净模式)
    struct termios options;
    tcgetattr(fd, &options);
    
    cfsetispeed(&options, B115200);
    cfsetospeed(&options, B115200);
    
    // 【关键升级】彻底关闭所有特殊字符处理、回显和换行符转换
    // 这相当于标准库的 cfmakeraw() 功能，保证数据 100% 原样透传
    options.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    options.c_oflag &= ~OPOST;
    options.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    
    options.c_cflag &= ~(CSIZE | PARENB | CSTOPB);
    options.c_cflag |= (CS8 | CLOCAL | CREAD);

    options.c_cc[VTIME] = 5; // 超时 500ms
    options.c_cc[VMIN]  = 0;
    
    // 【关键升级】在应用配置前，强力清空输入和输出缓冲区里的历史残存垃圾数据！
    tcflush(fd, TCIOFLUSH);
    tcsetattr(fd, TCSANOW, &options);
    
    fcntl(fd, F_SETFL, 0);

    printf("[系统] 串口 %s 初始化成功 (115200, 8N1)\n", UART_DEV_PATH);
    printf("[提示] 请在电脑端串口助手中发送字符，发送 'Quit!!!' 退出测试。\n");

    // 3. 发送问候数据
    const char *msg = "\r\n[AFC-7] Hello from TLD257 A35 UART!\r\n";
    write(fd, msg, strlen(msg));

    // 4. 稳健的接收与回显循环
    unsigned char rx_buf[64];
    int running = 1;

    while (running) {
        // 【关键修复】预留 3 个字节空间防溢出
        int rx_len = read(fd, rx_buf, sizeof(rx_buf) - 3);
      
        if (rx_len > 0) {
            // 安全截断，变成合法字符串
            rx_buf[rx_len] = '\0';

            printf("[收到数据] 长度: %d, 内容: ", rx_len);
            fwrite(rx_buf, 1, rx_len, stdout); 
            printf(" 。");

            // 先检测是否包含退出指令 (此时 rx_buf 绝对安全且未被破坏)
            if (rx_len >= 7 && strstr((char *)rx_buf, "Quit!!!") != NULL) {
                const char *quit_msg = "\r\n[SYSTEM] Received instruction 'Quit!!!', quit Uart Test Item. \r\n";
                write(fd, quit_msg, strlen(quit_msg));
                running = 0;
                break; // 立刻跳出，不再回显
            }

            // 安全追加回车换行并回发
            rx_buf[rx_len]   = '\r';
            rx_buf[rx_len+1] = '\n';
            write(fd, rx_buf, rx_len + 2);

            printf("并已原样回复，请输入“Quit!!!”可退出串口测试！\n");
        }
    }

    close(fd);
    printf("---> 串口测试完毕，返回主菜单 <---\n\n");
    return 0;
}
/************************ (C) COPYRIGHT ACG co. *****END OF FILE****/