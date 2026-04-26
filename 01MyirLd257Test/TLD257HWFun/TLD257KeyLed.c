/******************** (C) COPYRIGHT 2026 AFC Tech Co.*************************
 * 作    者  ： 曾庆华
 * 文 件 名  ： TLD257KeyLed.C
 * 版    本  ： V1.0.260329:  
 * 描    述  ： 米尔开发板的按键和LED联动测试函数
 * 官    网  ： www.acecreator.com
 * 淘    宝  ： acecreator.taobao.com
 * 公 众 号  ： 飞行控制与仿真
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include "TLD257HwFun.h"

// 请根据你在第一步查到的实际 event 编号修改此处！
#define KEY_INPUT_PATH      "/dev/input/event0"

#define BLUE_BRIGHT_PATH    "/sys/class/leds/blue:heartbeat/brightness"
#define BLUE_TRIG_PATH      "/sys/class/leds/blue:heartbeat/trigger"
#define GREEN_BRIGHT_PATH   "/sys/class/leds/green:heartbeat/brightness"
#define GREEN_TRIG_PATH     "/sys/class/leds/green:heartbeat/trigger"

// 辅助函数：夺权与恢复
static void SetLedTrigger(const char* trigPath, const char* mode) {
    int fd = open(trigPath, O_WRONLY);
    if (fd >= 0) { write(fd, mode, 4); close(fd); }
}

// 辅助函数：控制亮灭
static void SetLedState(const char* brightPath, int state) {
    int fd = open(brightPath, O_WRONLY);
    if (fd >= 0) { 
        if(state) write(fd, "1", 1); 
        else      write(fd, "0", 1); 
        close(fd); 
    }
}

int TLD257KeyLed(void) {
    printf("\n---> 进入 [按键 & LED 联动] 测试模块 <---\n");
    printf("[提示] 请按下开发板上的 USER 按键切换模式。连续按切至模式4可退出。\n");

    // 1. 夺取双灯控制权
    SetLedTrigger(BLUE_TRIG_PATH, "none");
    SetLedTrigger(GREEN_TRIG_PATH, "none");
    SetLedState(BLUE_BRIGHT_PATH, 0);
    SetLedState(GREEN_BRIGHT_PATH, 0);

    // 2. 以【非阻塞模式】打开按键设备
    int fd_key = open(KEY_INPUT_PATH, O_RDONLY | O_NONBLOCK);
    if (fd_key < 0) {
        printf("[错误] 无法打开按键节点: %s\n", KEY_INPUT_PATH);
        return -1;
    }

    int mode = 1;         // 初始状态 1
    int tick_count = 0;   // 滴答计数器 (每次 50ms)
    struct input_event ev;
    int running = 1;

    // 3. 异步状态机主循环
    while (running) {
        // --- 非阻塞读取按键事件 ---
        while (read(fd_key, &ev, sizeof(struct input_event)) > 0) {
            // EV_KEY=按键事件, value 1=按下
            if (ev.type == EV_KEY && ev.value == 1) {
                mode++;
                if (mode > 4) mode = 1; // 循环切换
                printf("\n[检测到按键] 切换至模式: %d\n", mode);
                
                // 切换模式时，先全灭，防止状态残留
                SetLedState(BLUE_BRIGHT_PATH, 0);
                SetLedState(GREEN_BRIGHT_PATH, 0);
                tick_count = 0; 

                if (mode == 4) {
                    running = 0; // 触发退出条件
                }
            }
        }

        if (!running) break;

        // --- 根据当前 mode 执行 LED 逻辑 (每 50ms 一次 tick) ---
        switch (mode) {
            case 1: // 蓝灯 1s 周期 (500ms 亮, 500ms 灭 = 10 ticks)
                if (tick_count % 20 < 10) SetLedState(BLUE_BRIGHT_PATH, 1);
                else                      SetLedState(BLUE_BRIGHT_PATH, 0);
                break;
            case 2: // 绿灯 0.5s 周期 (250ms 亮, 250ms 灭 = 5 ticks)
                if (tick_count % 10 < 5) SetLedState(GREEN_BRIGHT_PATH, 1);
                else                     SetLedState(GREEN_BRIGHT_PATH, 0);
                break;
            case 3: // 双灯交替 (蓝亮绿灭 500ms, 蓝灭绿亮 500ms)
                if (tick_count % 20 < 10) {
                    SetLedState(BLUE_BRIGHT_PATH, 1);
                    SetLedState(GREEN_BRIGHT_PATH, 0);
                } else {
                    SetLedState(BLUE_BRIGHT_PATH, 0);
                    SetLedState(GREEN_BRIGHT_PATH, 1);
                }
                break;
        }

        // --- 基准延时 50ms ---
        usleep(50000); 
        tick_count++;
    }

    // 4. 清理现场，交还控制权
    close(fd_key);
    SetLedTrigger(BLUE_TRIG_PATH, "heartbeat");
    SetLedTrigger(GREEN_TRIG_PATH, "heartbeat");
    
    printf("---> 联动测试完毕，返回主菜单 <---\n\n");
    return 0;
}
/************************ (C) COPYRIGHT ACG co. *****END OF FILE****/