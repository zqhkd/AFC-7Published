/******************** (C) COPYRIGHT 2026 AFC Tech Co.*************************
 * 作    者  ： 曾庆华
 * 文 件 名  ： TLD257Led.C
 * 版    本  ： V1.0.260329 
 * 描    述  ： 米尔开发板的LED指示灯测试函数
 * 官    网  ： www.acecreator.com
 * 淘    宝  ： acecreator.taobao.com
 * 公 众 号  ： 飞行控制与仿真
*****************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "TLD257HwFun.h"

// 蓝灯节点路径
#define LED_BRIGHTNESS_PATH "/sys/class/leds/blue:heartbeat/brightness"
#define LED_TRIGGER_PATH    "/sys/class/leds/blue:heartbeat/trigger"

int TLD257Led(void) {
    printf("\n---> 进入 [模块 1：LED 基础闪烁] 测试 <---\n");
    int fd_trigger, fd_bright;

    // 1. 夺取控制权：解除系统 heartbeat
    fd_trigger = open(LED_TRIGGER_PATH, O_WRONLY);
    if (fd_trigger >= 0) {
        write(fd_trigger, "none", 4);
        close(fd_trigger);
    } else {
        printf("[警告] 无法打开 trigger 节点，可能权限不足。\n");
    }

    // 2. 闪烁测试循环
    fd_bright = open(LED_BRIGHTNESS_PATH, O_WRONLY);
    if (fd_bright < 0) {
        printf("[错误] 找不到 LED 设备节点: %s\n", LED_BRIGHTNESS_PATH);
        return -1;
    }

    printf("[执行] 蓝灯将以 500ms 频率闪烁 10 次...\n");
    for (int tick = 0; tick < 10; tick++) {
        write(fd_bright, "1", 1);
        printf("  -> LED ON\n");
        usleep(500000); // 500ms
        
        write(fd_bright, "0", 1);
        printf("  -> LED OFF\n");
        usleep(500000); // 500ms
    }
    close(fd_bright);

    // 3. 释放资源，恢复系统心跳
    fd_trigger = open(LED_TRIGGER_PATH, O_WRONLY);
    if (fd_trigger >= 0) {
        write(fd_trigger, "heartbeat", 9);
        close(fd_trigger);
    }
    
    printf("---> LED 测试完毕，返回主菜单 <---\n\n");
    return 0;
}
/************************ (C) COPYRIGHT ACG co. *****END OF FILE****/