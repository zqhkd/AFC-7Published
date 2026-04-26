#include <stdio.h>
#include <unistd.h>
#include "ZFA35Lib.h"       // 引用 Include 目录下的库头文件
#include "AfcControlLogic.h" // 引用 Slx2Rtw 下的算法头文件

int main() {
    printf("========================================\n");
    printf("   AFC-7 A35 Intelligent Flight Control \n");
    printf("========================================\n");

    // 1. 初始化底层硬件 (调用 ZfA35DevLib)
    ZfA35_Actuator_Init(400); 

    // 2. 任务调度循环 (以 100Hz 模拟)
    int tick = 0;
    while (tick < 50) {
        // 执行算法计算
        AfcControlLogic_Step();
        float out = AfcControlLogic_GetOutput();

        // 执行硬件输出
        ZfA35_Actuator_Output(out);

        usleep(10000); // 延时 10ms
        tick++;
    }

    printf("[System] Mission Completed. Safe Shutdown. \n");
    return 0;
}