/******************** (C) COPYRIGHT 2026 AFC Tech Co.*************************
 * 作    者  ： 曾庆华
 * 文 件 名  ： main.C
 * 版    本  ： V1.0.260330 
 * 描    述  ： 米尔开发板功能测试主函数
 * 官    网  ： www.acecreator.com
 * 淘    宝  ： acecreator.taobao.com
 * 公 众 号  ： 飞行控制与仿真
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>

// 引入底层测试模块的头文件
#include "TLD257HwFun.h"
 
int main() {
    // 关闭 stdout 的所有内部缓冲, 确保调试过程中正常显示！ _IONBF 意思是 No Buffering (无缓冲)
    setvbuf(stdout, NULL, _IONBF, 0);
    int choice = -1;

    // 无限循环，保持测试控制台常驻
    while (1) {
        printf("======================================\n");
        printf("  TLD257 硬件综合测试控制台 v1.0      \n");
        printf("======================================\n");
        printf("  [1] 运行 模块 1：LED 基础闪烁测试\n");
        printf("  [2] 运行 模块 2：按键与双 LED 联动测试\n");
        printf("  [3] 运行 模块 3：A35 调试串口 (J15) 双向通讯测试\n");
        printf("  [4] 运行 模块 4：ENET2 普通千兆网 UDP 测试\n");
        printf("  [5] 运行 模块 5：ENET1 TSN 硬件时间戳 测试\n");
        printf("  [6] 运行 模块 6：高精度1ms实时任务调度线程 测试\n");
        printf("  [7] 运行 模块 7：TSN高实时双核顶层框架程序 测试\n");
        printf("  [0] 退出测试程序\n");
        printf("======================================\n");
        printf("请输入测试项编号: ");
        // 【关键新增】强制将缓冲区的内容推送到屏幕上！
        fflush(stdout);

        //  读取用户键盘输入
        if (scanf("%d", &choice) != 1) {
            // 如果用户乱敲字母，清除缓存防止死循环
            while(getchar() != '\n'); 
            continue;
        }
        // choice = 4;
        // 路由分发
        switch (choice) {
            case 1: 
                TLD257Led();         break;
            case 2:
                TLD257KeyLed();      break;
            case 3: 
                TLD257Uart();        break; // 路由到串口测试
            case 4: 
                TLD257EthBasic();    break; // 路由到网口2基本功能测试
            case 5: 
                TLD257EthTsn();      break; // 路由到网口1的TSN硬件时间戳测试
            case 6: 
                TLD257RtTask();      break; // 路由到网口1的任务线程实时调度测试
            case 7: 
                TLD257TsnCoreFrame(); break; // 路由到网口1的ENET1 TSN高实时双核架构顶层框架程序
            case 0:
                printf("\n[退出] 感谢使用 TLD257 测试控制台！\n");
                return 0;
            default:
                printf("\n[错误] 无效的编号，请重新输入。\n\n");
                break;
        }
    }
    return 0;
}