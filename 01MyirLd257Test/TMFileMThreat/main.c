/******************** (C) COPYRIGHT 2019 ACE Tech Co.*************************
 * 作  者 ： 曾庆华
 * 文件名 ： imu.c
 * 版  本 ： V1.01.260320                                         
 * 描  述 ： AFC-7问候世界的多文件测试模块
 * 官  网 ： www.acecreator.com
 * 淘  宝 ： acecreator.taobao.com
 * 公众号 ： 无人飞行控制
*****************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include "imu.h"  // 测试 Makefile 是否能自动找到子目录下的头文件

// 模拟电机控制线程
void* motor_thread(void* arg) {
    printf("[Motor Thread] 线程已启动，正在监听控制指令...\n");
    fflush(stdout);
    
    int count = 0;
    while(count < 5) {
        printf("[Motor Thread] 模拟输出 PWM 占空比: %d%%\n", 10 + count * 5);
        fflush(stdout);
        sleep(1);
        count++;
    }
    printf("[Motor Thread] 测试结束，退出线程。\n");
    return NULL;
}

int main() {
    printf("==================================================\n");
    printf("    卓飞智控 AFC-7 A35 多文件/多线程 集成测试       \n");
    printf("==================================================\n");
    fflush(stdout);

    // 1. 测试子文件夹函数调用
    float gz = get_accel_z();
    printf("[Main] 从 sensor 文件夹读取到 IMU 数据: %.3f m/s^2\n", gz);
    fflush(stdout);

    // 2. 测试多线程功能 (-pthread 链接参数测试)
    pthread_t tid;
    if (pthread_create(&tid, NULL, motor_thread, NULL) != 0) {
        printf("[Main] 错误：无法创建多线程，请检查 Makefile 是否包含 -pthread\n");
        return -1;
    }

    // 3. 测试变量单步调试 (a = 7 验证)
    int a = 2;
    a = a + 5; 
    printf("[Main] 变量 a 调试验证值: %d (预期应为 7)\n", a);
    fflush(stdout);

    // 等待子线程结束
    pthread_join(tid, NULL);

    printf("==================================================\n");
    printf("    所有功能测试完成，AFC-7 开发环境准备就绪！     \n");
    printf("==================================================\n");
    fflush(stdout);

    return 0;
}