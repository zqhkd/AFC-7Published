# TLD257HWFun (米尔 MYD-LD25X 硬件综合测试控制台)

## 📌 项目简介

本项目是针对米尔 MYD-LD25X (STM32MP257) 开发板 A35 核心的 Linux 硬件接口综合测试工程。
工程采用**“顶层交互菜单 + 底层独立模块”**架构，函数与文件命名遵循 `驼峰命名法`，专有名词（如 TLD257）全大写，测试接口统一以 `T` 引导。

## 📂 目录结构与命名规范

```text
TLD257HWFun/
├── Makefile                # 自动化编译脚本
├── main.c                  # 顶层交互菜单 (调试控制台)
├── TLD257Led.c / .h        # [模块 1] 单 LED 闪烁测试
├── TLD257KeyLed.c / .h     # [模块 2] 按键与双 LED 联动测试
└── README.md               # 项目说明文档
```

## 🧠 顶层架构：调试控制台 (main.c)

`main.c` 作为整个测试控制台的“大管家”，不包含任何底层硬件操作代码。它只负责：

1. **打印交互式 UI 菜单** 。
2. **捕获用户键盘输入** 。
3. **路由分发** ：根据用户选择，调用对应 `.h` 头文件中暴露的测试子函数（如 `TLD257Led()`）。
4. **状态闭环** ：子测试执行完毕后返回 `main`，系统重新拉起主菜单，实现无缝连续测试。

调试台程序是整个测试工程的“灵魂”，它负责路由调度。把上一轮我写给你的 `TLD257KeyLed.c` 和 `TLD257KeyLed.h` 也放在同级目录下，它就能完美联动了。
**C**

```
#include <stdio.h>
#include <stdlib.h>

// 引入底层测试模块的头文件
#include "TLD257Led.h"
#include "TLD257KeyLed.h"

int main() {
    int choice = -1;

    // 无限循环，保持测试控制台常驻
    while (1) {
        printf("======================================\n");
        printf("  TLD257 硬件综合测试控制台 v1.0      \n");
        printf("======================================\n");
        printf("  [1] 运行 模块 1：LED 基础闪烁测试\n");
        printf("  [2] 运行 模块 2：按键与双 LED 联动测试\n");
        printf("  [0] 退出测试程序\n");
        printf("======================================\n");
        printf("请输入测试项编号: ");
  
        // 读取用户键盘输入
        if (scanf("%d", &choice) != 1) {
            // 如果用户乱敲字母，清除缓存防止死循环
            while(getchar() != '\n'); 
            continue;
        }

        // 路由分发
        switch (choice) {
            case 1:
                TLD257Led();
                break;
            case 2:
                TLD257KeyLed();
                break;
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
```

## 🛠️ 模块开发与测试指南

## 模块 1：LED 指示灯测试

Linux 系统的核心思想是“一切皆文件”。在 Linux 下控制 LED，本质上是通过读写 `/sys/class/leds/` 目录下的设备节点文件来实现的。

### 第一步：创建模块 1 的头文件 (`TLD257Led.h`)

这是对外暴露的接口，代码极简。

**C**

```
#ifndef TLD257LED_H
#define TLD257LED_H

// 暴露给主控台的 LED 测试接口
int TLD257Led(void);

#endif
```

---

### 第二步：创建模块 1 的源码文件 (`TLD257Led.c`)

这里封装了带“夺权”功能的标准 LED 闪烁逻辑。

**C**

```
include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "TLD257Led.h"

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
```

## 🚀 编译与调试运行 (VS Code 环境)

本项目已深度集成 VS Code 交叉编译与 GDB 远程调试链路。

1. **一键发车** ：在 VS Code 中打开本工程，直接按下键盘 **`F5`** 。
2. **自动化流程** ：VS Code 会自动调用跨平台工具链编译代码 ➡️ 通过 SCP 推送至开发板 ➡️ 拉起 GDB Server 并在主函数入口截停。
3. **交互与单步** ：
   * 在 VS Code 的**调试控制台 (Debug Console)** 或**终端 (Terminal)** 观察菜单输出。
   * 遇到系统底层函数（如 `write`, `usleep`），请使用 **断点 + F5 (Continue)** 跨过，或使用 **F10 (Step Over)** ，切勿使用 F11 强行步入动态链接库。


## 模块 2：按键与 LED 联动测试 (`TLD257KeyLed`)

本模块验证 Linux `Input Subsystem` (输入子系统) 与多设备的非阻塞联动。

* **原理** ：按键触发 Linux 底层的 Input Event；程序通过非阻塞轮询读取键值，并通过状态机切换两颗 LED 的闪烁逻辑。
* **状态机** ：
  * **状态 1** ：蓝灯 1秒间隔闪烁 (500ms亮/500ms灭)
  * **状态 2** ：绿灯 0.5秒快闪 (250ms亮/250ms灭)
  * **状态 3** ：双灯交替闪烁
  * **状态 4** ：退出测试，返回主菜单

#### 第一步：侦察按键设备节点

在开发板终端执行以下命令，寻找含有 `gpio-keys` 字样的输入设备：

**Bash**

```
root@myd-ld25x:~# cat /proc/bus/input/devices
```

显示如下：
root@myd-ld25x:~# cat /proc/bus/input/devices
I: Bus=0019 Vendor=0001 Product=0001 Version=0100
N: Name="gpio-keys"
P: Phys=gpio-keys/input0
S: Sysfs=/devices/platform/gpio-keys/input/input0
U: Uniq=
H: Handlers=event0
B: PROP=0
B: EV=3
B: KEY=2 0 0 0 0
最关键的两行：
N: Name="gpio-keys"：这说明这个设备正是通过 GPIO 引脚接入的实体按键（通常也就是板子上的 USER 按键）。
H: Handlers=event0：这就是我们要找的“终极答案”！Linux 输入子系统把这个物理按键的动作，绑定到了 event0 这个虚拟事件节点上。
以上表明：按键在 Linux 系统里的绝对路径就是 /dev/input/event0。

#### 第二步：联动测试核心源码 (`TLD257KeyLed.c`)

**C**

```
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include "TLD257KeyLed.h"

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
  
                if (mode == 4) {
                    running = 0; // 触发退出条件
                } else {
                    // 切换模式时，先全灭，防止状态残留
                    SetLedState(BLUE_BRIGHT_PATH, 0);
                    SetLedState(GREEN_BRIGHT_PATH, 0);
                    tick_count = 0; 
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
```

### 模块 3：A35 调试串口 (J15) 双向通讯测试 (`TLD257Uart`)

本模块验证 STM32MP257 A35 核心通过 USART2 与上位机（PC）进行全双工串口通讯的能力。

#### 🔌 硬件连接准备

请准备一个 USB-TTL 串口转换模块（如 CH340 或 CP2102），连接开发板的 J15 接口与上位机电脑：

* **开发板 J15 引脚 1 (PA8_USART2_RX)** <----连线---->  **USB-TTL 的 TX (发送)**
* **开发板 J15 引脚 2 (PA4_USART2_TX)** <----连线---->  **USB-TTL 的 RX (接收)**
* **开发板 J15 引脚 3 (GND)** <----连线---->  **USB-TTL 的 GND (共地)**

> ⚠️ **注意**：TX 和 RX 必须交叉连接！不要连接 VCC，以免烧毁引脚。

#### 💻 上位机配置

打开电脑端的串口调试助手（如 SSCOM、XCOM 或 SecureCRT）：

* **波特率 (Baud Rate)**: `115200`
* **数据位 (Data Bits)**: `8`
* **停止位 (Stop Bits)**: `1`
* **校验位 (Parity)**: `无 (None)`

#### 🔍 确定串口设备节点

在开发板终端中输入以下命令，确认 USART2 对应的设备节点（通常为 `ttySTM0` 或 `ttySTM1`）：

```bash
root@myd-ld25x:~# dmesg | grep ttySTM
```

如果 J15 是 Linux 的默认终端输出口，它极有可能是 /dev/ttySTM0。

🚀 测试操作流程
启动测试程序，选择菜单 [3]。

此时开发板会主动向上位机发送一条问候信息 "Hello from TLD257 A35! \r\n"。

在电脑的串口助手中发送任意英文字符，开发板收到后会在终端打印，并原样回发给电脑（Echo 模式）。

在电脑串口助手发送小写字母 q，即可退出当前串口测试模块，返回主菜单。

---

### 第二步：创建模块 3 头文件 (`TLD257Uart.h`)

新建文件 `TLD257Uart.h`，写入以下接口：

```c
    #ifndef TLD257UART_H
    #define TLD257UART_H
    // 暴露给主控台的 串口 测试接口
    int TLD257Uart(void);

    #endif
```

第三步：创建模块 3 源码文件 (TLD257Uart.c)
这是 Linux 串口编程的“标准模板”。新建文件 TLD257Uart.c，写入以下代码。
(注意：宏定义 UART_DEV_PATH 默认为 /dev/ttySTM0，请根据你在板子上侦察到的实际情况修改，可能是 ttySTM1)。

```C
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include "TLD257Uart.h"

// ========================================================
// 请根据 dmesg | grep ttySTM 的结果，确认 J15 对应的设备节点
// ========================================================
#define UART_DEV_PATH "/dev/ttySTM0" 

int TLD257Uart(void) {
    printf("\n---> 进入 [模块 3：A35 调试串口 (J15) 双向通讯] 测试 <---\n");
  
    // 1. 打开串口设备 (非阻塞模式打开)
    int fd = open(UART_DEV_PATH, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0) {
        printf("[错误] 无法打开串口设备节点: %s\n", UART_DEV_PATH);
        return -1;
    }

    // 2. 配置串口参数 (115200, 8N1)
    struct termios options;
    tcgetattr(fd, &options);              // 获取当前设置
    cfsetispeed(&options, B115200);       // 设置输入波特率
    cfsetospeed(&options, B115200);       // 设置输出波特率
  
    options.c_cflag |= (CLOCAL | CREAD);  // 忽略调制解调器控制线，开启接收
    options.c_cflag &= ~CSIZE;            // 掩码清零
    options.c_cflag |= CS8;               // 8位数据位
    options.c_cflag &= ~PARENB;           // 无校验位
    options.c_cflag &= ~CSTOPB;           // 1位停止位
  
    // 设置为原始模式 (Raw mode)，不进行特殊字符处理
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_oflag &= ~OPOST;
    options.c_iflag &= ~(ICRNL | IXON);

    // 设置读取超时 (VMIN=0, VTIME=5 表示 read 最多阻塞 0.5 秒)
    options.c_cc[VTIME] = 5; 
    options.c_cc[VMIN]  = 0;
  
    tcsetattr(fd, TCSANOW, &options);     // 应用配置
  
    // 清除原来的非阻塞标志，改由 VTIME 和 VMIN 控制超时
    fcntl(fd, F_SETFL, 0);

    printf("[系统] 串口 %s 初始化成功 (115200, 8N1)\n", UART_DEV_PATH);
    printf("[提示] 请在电脑端串口助手中发送字符，发送 'q' 退出测试。\n");

    // 3. 发送问候数据到上位机
    const char *msg = "\r\n[AFC-7] Hello from TLD257 A35 UART!\r\n";
    write(fd, msg, strlen(msg));

    // 4. 接收与回显循环
    unsigned char rx_buf[64];
    int running = 1;

    while (running) {
        // 读取串口数据
        int rx_len = read(fd, rx_buf, sizeof(rx_buf) - 1);
    
        if (rx_len > 0) {
            rx_buf[rx_len] = '\0'; // 添加字符串结束符
        
            // 打印到开发板终端 (VS Code 可见)
            printf("[收到数据] 长度: %d, 内容: %s\n", rx_len, rx_buf);
        
            // 将收到的数据原样回发给电脑串口助手 (Echo)
            write(fd, rx_buf, rx_len);
        
            // 检查退出条件
            for (int i = 0; i < rx_len; i++) {
                if (rx_buf[i] == 'q' || rx_buf[i] == 'Q') {
                    printf("[系统] 收到退出指令 'q'，结束串口测试。\n");
                    running = 0;
                    break;
                }
            }
        }
    }

    // 5. 关闭串口
    close(fd);
    printf("---> 串口测试完毕，返回主菜单 <---\n\n");
    return 0;
}
```

### 模块 4：ENET2 普通千兆网 UDP 测试 (TLD257EthBasic)
本模块验证 AFC-7 飞控系统的 ENET2 接口在 Linux 环境下的基础通讯能力。该网口在 AFC-7 规划中定位为“通用设备接口”与“算力级联接口”，要求具备极高的通讯鲁棒性。

#### 📡 实验环境与物理链路
物理连接：将开发板的 ENET2 网口（注意区分 ENET1）通过网线直连至 PC。

网络配置：
    - 开发板 (AFC-7)：需手动分配 IP 192.168.1.10（命令：ifconfig end2 192.168.1.10 up）。
    - 上位机 (PC)：设置静态 IP 为 192.168.1.2，子网掩码 255.255.255.0。
    - 调试工具：在 Windows 侧运行 scomm.exe 或其他网络调试助手。

#### 💡 核心设计逻辑
异步非阻塞 (O_NONBLOCK)：利用 fcntl 开启非阻塞模式，确保在等待接收网络指令时，飞控的心跳广播逻辑和主菜单响应不会卡死。
本地端口绑定 (Bind)：显式绑定本地端口 9999。在无人机集群或多网口应用中，明确监听端口是确保指令准确送达特定进程（如 AFC-7 导航进程）的基础。

双向通讯流：
    - 上行 (Telemetry)：每隔 1 秒（20 个 50ms ticks）自动向地面站推送心跳包。
    - 下行 (Command)：实时监听地面站指令。收到数据后执行“回显 (Echo)”并打印，输入 q 即可退出模块。

#### 🛠️ 模块关键源码结构
TLD257EthBasic.h：定义模块入口函数 int TLD257EthBasic(void)。
TLD257EthBasic.c：
    - 使用 socket(AF_INET, SOCK_DGRAM, 0) 创建 UDP 套接字。
    - 执行 bind() 绑定 LOCAL_BIND_PORT (9999)。
    - 在 while(running) 循环中通过 recvfrom() 进行非阻塞数据捕获。

#### 🚀 测试验证
**文件归位**：将 .c 和 .h 文件存入 TLD257HWFun 目录。
**路由配置**：在 main.c 的交互菜单中新增 [4] 选项，并链接至 TLD257EthBasic()。
**一键编译**：在 WSL2 或 VS Code 终端输入 `make`，确认没有 `undefined reference` 报错。    
**PC 端配置**：    
    - 打开 **网络调试助手**。        
    - 协议类型：**UDP**。        
    - 本地端口：**8888**（需与代码中的 `TARGET_PORT` 一致）。        
**运行测试**：在开发板运行 `./TLD257HWFun`，选择 `[4]`。    
    - 你应该能立刻在 PC 端的调试助手中看到 **`[AFC-7] Heartbeat: ENET2 is Active!`**。        
    - 在 PC 端发送任何字符给开发板，开发板应能在终端打印并在 PC 端回显。

### 模块 5：ENET1 TSN 硬件时间戳测试 (TLD257EthTsn)
本模块旨在验证 AFC-7 飞控系统 **ENET1** 接口的 **TSN (Time-Sensitive Networking)** 硬件特性。这是实现 **1ms 实时仿真步长** 以及无人机集群 **高精度时空对齐** 的底层核心技术。

#### 📡 实验环境与物理链路
* **物理连接**：将开发板的 **ENET1** 网口（支持 TSN 特性）连接至支持 IEEE 1588 (PTP) 的仿真机或另一台 AFC-7 飞控。
* **网口确认**：在 Linux 系统中确认 `end1` 接口处于 `UP` 状态。
* **软件需求**：内核需支持 `SO_TIMESTAMPING` 网络套接字选项。
* **硬件支持预检**：在编写代码前，请在开发板终端执行以下命令，确认 end1 接口是否支持硬件时间戳：
```
    ethtool -T end1
```
回复的关键指标解析：
    - hardware-transmit / hardware-receive：具备物理层硬件发包/收包打桩能力。
    - PTP Hardware Clock: 0：存在专门的 PTP 硬件时钟单元（PHC），这是实现 1ms 实时仿真 的绝对基准。
    - Hardware Transmit/Receive Modes: on / all：驱动层已完全解锁，可以捕获任何协议包（不仅限于 PTP 包）的硬件时间。

#### 💡 核心设计逻辑
* **硬件时间戳捕获 (Hardware Timestamping)**：不同于传统的软件时间戳（受内核调度抖动影响），硬件时间戳由以太网 MAC 控制器在数据包到达/离开物理层的瞬间生成。这是实现微秒级同步的先决条件。
* **异步辅助通道 (Control Messages)**：通过 `recvmsg()` 接口读取套接字的辅助数据（Control Messages），从中提取内核上报的 `struct scm_timestamping` 硬件结构体。
* **同步验证流**：
    * **发送端**：在发包后，通过错误队列（Error Queue）抓取该包离开物理层的精确“出发时刻”。
    * **接收端**：实时打印数据包到达的硬件时刻，计算链路延迟。

#### 🛠️ 模块关键源码结构
* **TLD257EthTsn.h**：定义模块入口函数 `int TLD257EthTsn(void)`。
* **TLD257EthTsn.c**：
    * 使用 `setsockopt()` 开启 `SOF_TIMESTAMPING_TX_HARDWARE` 与 `SOF_TIMESTAMPING_RX_HARDWARE` 标志。
    * 调用 `recvmsg()` 配合 `CMSG_FIRSTHDR` 宏解析硬件时间戳。
    * 对比系统时间与硬件时间，直观展示“确定性”差异。

#### 🚀 测试验证
* **环境初始化**：确保 `end1` 驱动已加载并支持 PTP（可通过 `ethtool -T end1` 预先验证）。
* **路由配置**：在 `main.c` 交互菜单中新增 **[5]** 选项，链接至 `TLD257EthTsn()`。
* **一键编译**：利用自动化框架完成交叉编译，确保链接了必要的实时库。
* **运行测试**：
    1.  选择菜单 **[5]** 进入 TSN 测试模式。
    2.  UDP 和硬件时间戳的具有盲发特性，它只管把数据包网网线里“扔”，这正是TSN的迷人之处，因此，上位机不需要做任何操作。当然，如果你把上位机的指定IP和端口号打开，可以观察到网线上的数据。
    2.  观察VSCode调试终端输出的 `HW Timestamp`。
    3.  **性能对比**：在高负载流量（如同时进行模块 4 的 UDP 压测）下，观察硬件时间戳的抖动是否依然保持在微秒（$\mu s$）量级。

### 模块 6：基于 Linux 线程管理的任务线程调度测试 (TLD257RtTask)
本模块旨在通过 Linux 的实时调度策略（SCHED_FIFO）验证 AFC-7 飞控系统在 A35 核心上的任务调度精度。虽然它不属于基于 TSN 的顶层硬件对齐框架，但它是评估 Linux 线程管理极限能力 的重要参考，也是飞控系统运行复杂算法线程的基础。

📡 实验环境与系统硬化（实操步骤）
为了获得最佳的 1ms 调度精度，必须在按下 F5 启动调试前，确保开发板环境已执行以下“系统硬化”操作，以消除通用 Linux 内核带来的随机抖动。
1. 锁定 CPU 频率 (开发板侧)
防止 schedutil 模式下的动态调频产生微秒级延迟：
```
    # 执行环境：AFC-7 开发板终端 (root 权限)
    echo performance > /sys/devices/system/cpu/cpu1/cpufreq/scaling_governor
```
2. 强制中断重定向 (开发板侧)
将所有可移动的硬件中断从 CPU 1 赶往 CPU 0，腾空实时核心：
```
    # 执行环境：AFC-7 开发板终端 (root 权限)
    for i in /proc/irq/*/smp_affinity; do echo 1 > $i 2>/dev/null; done
```

💡 核心设计逻辑
实时优先级锁定：创建线程时指定 SCHED_FIFO 策略，并将优先级设为最高（99），确保任务在 CPU 1 上具有绝对抢占权。
算法补偿机制：利用 clock_nanosleep 提前唤醒 + do-while 忙等逻辑，精确捕捉 CLOCK_MONOTONIC 的 1.000ms 起跳时刻。
内存锁定：代码内部调用 mlockall(MCL_CURRENT | MCL_FUTURE)，防止虚拟内存换页停顿。

📊 输出界面与统计参数解读
在调试控制台中，模块 6 会输出详细的实时性报告。以下为关键参数解析：
实时采样数据：

```Plaintext
     序   |  累计周期 | 当前 Jitter (ns)| 峰值 Jitter (ns)
    ----------------------------------------------------
    [019] |    10003 |            9175 |         1514400
```

当前 Jitter (ns)：瞬时调度偏差。在未完全隔离内核的情况下，通常在 9μs 左右浮动。
峰值 Jitter (ns)：自启动以来的最大偏差记录。
终结统计报告：
```Plaintext
    📊 AFC-7 核心线程实时性统计报告:
    - 运行总次数:    10004 次，调度周期： 1.00 ms
    - 平均绝对偏差:  10818.63 ns (10.819 us)
    - 历史最大偏差:  1514400 ns (1514.400 us)
```
运行总次数：测试覆盖的执行周期总数。
平均绝对偏差 (Average Jitter)：反映系统长期的调度稳定性。AFC-7 在环境硬化后应稳定在 11μs 以内（软件极限）。
历史最大偏差 (Max Latency)：若数值超过 1ms (1,000,000 ns)，实锤了 Linux 内核存在不可抢占的“大锁”路径或后台 IO 干扰，这标志着必须进一步实施 isolcpus 物理隔离。

🚀 运行备注
一键操作：直接在 VSCode 中按 “ctrl+shift+B”连续运行或者F5单步调试 即可完成编译、部署与运行。
重要区分：本模块评估的是 Linux 线程管理 的软件实时性极限。后续 基于 TSN 的顶层调度框架 将采用硬件 PHC 时钟作为基准，请注意区分两者不同的测试维度。

### 模块 7：基于 TSN 硬件实时驱动的顶层核心调度框架 (TLD257TsnCore)
本模块是 AFC-7 飞控系统的核心架构基石。它摒弃了传统的“软件定时器”驱动模式，全面转向 “异构多核 + TSN 硬件实时” 架构。通过 AF_XDP 零拷贝 技术绕过内核协议栈，并利用 PHC 硬件时钟 实现全网时空对齐。A35 核心在此框架中仅作为“确定性网关”，由外部仿真机的 TSN 数据包通过硬件中断直接驱动，实现微秒级响应。

#### 📡 实验环境与系统硬化（实操步骤）
为了验证 TSN 硬件直通的极致性能，必须在启动测试前执行以下“环境硬化”与“中断绑定”操作，确保 CPU 1 成为纯净的 TSN 处理核心。

1. 锁定 CPU 频率与内存 (开发板侧)
消除调频延迟与虚拟内存换页停顿：
```
    # 执行环境：AFC-7 开发板终端 (root 权限)
    echo performance > /sys/devices/system/cpu/cpu1/cpufreq/scaling_governor
```
2. 精确中断亲和性绑定 (开发板侧)
必须将 TSN 网口的中断源强制绑定到处理线程所在的 CPU 1，避免跨核唤醒延迟：
```
    # 执行环境：AFC-7 开发板终端 (root 权限)
    # 假设 end1 中断号为 124 (通过 grep end1 /proc/interrupts 确认)
    echo 2 > /proc/irq/124/smp_affinity
```
3. 启动并绑定核心
```
    # 使用 taskset 强制程序运行在 CPU 1
    taskset -c 1 ./build/TLD257HWFun
```

#### 💡 核心设计逻辑
AF_XDP 硬件直通钩子：利用 AF_XDP 零拷贝技术，使数据包直接从网卡 DMA 缓冲区进入用户态。通过 poll() 监听硬件中断，数据一到即刻执行 TSN_HW_DMA_Rx_Hook，彻底绕过 Linux 内核协议栈。
PHC 硬件时钟对齐：通过 ioctl(PTP_CLOCK_GETTIME) 直接读取 /dev/ptp1 硬件时钟。这是基于 IEEE 802.1AS (gPTP) 同步后的全网统一时间，而非不稳定的系统软件时钟。
异构多核优先级隔离：
线程 1 (优先级 80)：TSN 实时仿真通道，负责 1ms 高动态数据搬运。
线程 2 (优先级 50)：集群协同 MAVLink 通讯，周期 5ms~50ms。
线程 3 (非实时)：AI 视觉与任务规划，准实时 20ms~100ms。

#### 📊 输出界面与统计参数解读
模块 7 侧重于评估 “网关转发延迟”，即数据包从物理层到达硬件，到被 A35 线程响应处理的物理耗时。
实时统计数据解析：
```
Code snippet
   收包序号  |  A35 转发耗时 (us) | 峰值耗时 (us)
  ----------------------------------------------------
   [1000]   |              85.50 |          142.30
```
A35 转发耗时 (us)：指从 PHC 硬件收包时刻 到 用户态 Hook 函数启动 的物理时延。在 AF_XDP 驱动下，该值通常应在 50μs ~ 150μs 之间。
峰值耗时 (us)：测试过程中的最大延迟，反映了系统在极端干扰下的确定性边界。

统计报告：
```
Code snippet
    📊 AFC-7 真·TSN 硬件实时框架统计报告:
    - 运行总次数:     10000 次，仿真步长：1.00 ms (TSN 驱动)
    - 平均转发时延:   112.45 us (0.112 ms)
    - 历史最大偏差:   285.60 us (0.285 ms)
```
运行总次数：由外部实时仿真机驱动的 1000Hz 脉冲总数。
平均转发时延：反映了 A35 作为“透明转发通道”的效率。满足 1ms 仿真要求的判据为：平均时延 < 500μs。
历史最大偏差：若该值稳定在 1ms 以内，则证明 A35 转发不会造成 M33 解算跨周期，系统具备真正的“硬实时”数据交换能力。

#### 🚀 运行备注
一键操作：直接在 VSCode 中按 “ctrl+shift+B” 即可完成编译、部署与运行。
前置条件：确保 Windows 侧已启动 PTPd4Windows 进行时钟同步，否则 PHC 时间戳读取将无意义。
重要区分：本模块是 AFC-7 的 终极实时方案。它不再受限于 Linux 软件调度的波动，而是完全依赖 TSN 硬件时钟与异构多核的物理响应。