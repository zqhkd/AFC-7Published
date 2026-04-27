# AFC-7 异构多核开发平台工程架构

**所属机构**：卓飞智控科技有限公司 (AFC Tech Co.)  
**平台架构**：AFC-7新一代智能飞控
**文档版本**：V1.01.260427 
**开源平台**：[zqhkd/AFC-7Published: AFC-7智能飞控软件开发平台](https://github.com/zqhkd/AFC-7Published)
**日   期**: 2026-04-27
**作   者**: 曾庆华

## 📖 平台摘要

本项目为 AFC-7 智能飞行控制与仿真平台的底层工程架构基座。针对 STM32MP257 异构多核处理器，本工程实现了 Cortex-A35 (OpenSTLinux) 与 Cortex-M33 (FreeRTOS) 的跨核协同开发模板。平台攻克了底层内存防火墙 (TZC/ETZPC) 隔离与 IPCC 核间通信时序竞争的难题，构建了基于 Windows WSL2 的全栈交叉编译与无痕在线调试 (Attach) 工作流。

> **⚠️ 声明** > 本开源版本仅包含系统级底层框架与基础驱动验证例程，它是卓飞智控核心无人机飞行控制与仿真软件开发平台代码。 

---
## 🛠️ 1. 开发环境安装与部署 
开发环境采用 Windows 宿主机与 WSL2 (Ubuntu) 虚拟编译机相融合的跨平台拓扑。
### 1.1 Windows 端核心工具链安装
请在 Windows 宿主机安装以下软件（建议保持默认路径）：
1. **Visual Studio Code**：前往官网安装最新版，作为全栈统一 IDE。    
2. **STM32CubeMX (≥6.17.0)**：用于底层引脚分配、生成设备树片段 (A35) 及初始化代码 (M33)。 
3. **STM32CubeProgrammer**：用于向米尔开发板 eMMC 烧录完整出厂系统镜像。请在 CubeMX 内下载 STM32MP2 系列支持包。    
4. **MobaXterm** (或 Xshell)：建立 Serial 串口会话（波特率 `115200`），用于直连开发板底层终端。   
5. **usbipd-win [关键底层依赖]**：前往 GitHub Releases 下载并安装。它用于打破虚拟机边界，将 Windows 端的物理 ST-Link 强行穿透至 WSL2 内部。

### 1.2 WSL2 (Ubuntu 20.04) 极速部署
_注：米尔官方手册推荐使用 Ubuntu 20.04 64bit。当前米尔开发板载 OpenSTLinux 版本为 6.1.82。_

1. **开启 WSL2**：在 Win11 PowerShell (管理员) 运行 `wsl --install -d Ubuntu-20.04`，重启电脑。    
2. **账号初始化**：进入 Ubuntu 设置 UNIX 账号。（例：用户名 `zqhkd`，密码 `3.1415926`）。    
3. **安装 A35 核心依赖包**：在 Ubuntu 终端执行换源（如清华源）并完整运行以下依赖库安装指令：
```Bash
# 更新源并安装米尔指定的“全家桶”核心依赖
sudo apt-get update
sudo apt-get install -y gawk wget git diffstat unzip texinfo gcc-multilib build-essential chrpath socat libsdl1.2-dev xterm sed cvs subversion coreutils texi2html docbook-utils python-pysqlite2 help2man make gcc g++ desktop-file-utils libgl1-mesa-dev libglu1-mesa-dev mercurial autoconf automake groff curl lzop asciidoc u-boot-tools cpio sudo locales bc libncurses5-dev screen flex bison vim-tiny device-tree-compiler xvfb libgtk2.0-dev libssl-dev net-tools libyaml-dev rsync liblz4-tool zstd python3-pip git-lfs iputils-ping jq iperf3

# 🚨 严禁漏装：这是 VS Code 能够进行 ARM 交叉调试的物理工具链基础
sudo apt install gdb-multiarch -y
```
4. **配置语言环境与 Python 依赖**：
```Bash
sudo ln -sf /usr/bin/python3 /usr/bin/python
sudo sed -i -e 's/# en_US.UTF-8 UTF-8/en_US.UTF-8 UTF-8/' /etc/locale.gen
echo 'LANG="en_US.UTF-8"' | sudo tee /etc/default/locale > /dev/null
sudo dpkg-reconfigure --frontend=noninteractive locales
sudo update-locale LANG=en_US.UTF-8

sudo pip3 install pyusb usb crypto ecdsa crcmod tqdm pycryptodome pycryptodomex pyelftools
```
5. **配置个人 Git 标识**：
```Bash
git config --global user.name "Your Name"
git config --global user.email "your.email@example.com"
```
### 1.3 A35 交叉编译工具链 (SDK) 安装
1. **载入安装脚本**：在 WSL 终端输入 `explorer.exe .` 打开资源管理器。将米尔资料包内的 SDK 安装脚本（如 `myir-image-full-openstlinux...sh`）拖入弹出的 Ubuntu 文件夹中。    
2. **赋予权限并执行**：
```Bash
chmod +x myir-image-full-openstlinux-weston-myd-ld25x.rootfs-x86_64-toolchain-5.0.3-snapshot.sh
./myir-image-full-openstlinux-weston-myd-ld25x.rootfs-x86_64-toolchain-5.0.3-snapshot.sh
```
> **🚨 架构红线警告：指定部署路径** 当安装程序询问 `Enter target directory for SDK (default: /opt/st/...)` 时，**切勿使用默认路径**！ 请务必手动输入并回车：`~/st_mp2_sdk` (或绝对路径 `/home/zqhkd/st_mp2_sdk`)。 若用户自定义了其他路径，后续必须在 `~/.afc7_env` 文件中同步更新 `ST_MP2_SDK_PATH`。

3. **确证地基**：安装完成后，执行以下指令。若未报错且文件存在，则 SDK 释放成功： `ls ~/st_mp2_sdk/environment-setup-cortexa35-ostl-linux`
### 1.4 环境配置标准化与自动化链路预设
1. **分离式环境变量部署 (SSoT)**： 用户须将项目仓库中的环境模板拷贝至个人家目录：   
```Bash
    cp ~/AFC-7/.afc7_env.setup ~/.afc7_env
```
   _执行 `nano ~/.afc7_env` 手动校核 `ST_MP2_SDK_PATH`、`AFC7_BOARD_IP` 和用户名，确保符合个人实际。_
2. **注入系统自启配置 (`.profile`)**： 此动作确保 VS Code 远端服务器接入时，环境变量自动生效
```Bash
    echo 'if [ -f ~/.afc7_env ]; then source ~/.afc7_env; fi' >> ~/.profile
```
  3. **自动化调试链路预设（屏蔽 IP 指纹冲突警告）**： 为避免重刷固件后 SSH 报错 (`Remote Host Identification Has Changed`) 阻断自动下发流水线，强制信任网段：    
```Bash
    mkdir -p ~/.ssh
    echo -e "Host 192.168.*.*\n    StrictHostKeyChecking no\n    UserKnownHostsFile /dev/null" >> ~/.ssh/config
    chmod 600 ~/.ssh/config
```
  4. **重启生效与“起飞”确证**： 在 Windows PowerShell 执行 `wsl --shutdown` 重启 Linux 内核。重新打开 Ubuntu 终端：    
```Bash
    echo $ST_MP2_SDK_PATH
    echo $AFC7_BOARD_IP
```
   若成功打印出正确的 SDK 路径与开发板 IP，底层架构正式打通！该环境变量模板文件示例如下：
```Bash
# --- SDK 与项目根目录 (波浪线自适应) ---
# ST MP2 官方交叉编译 SDK 绝对路径
export ST_MP2_SDK_PATH="$HOME/st_mp2_sdk"
# AFC-7开发平台 根目录
export AFC7_SYSTEM_ROOT="$HOME/AFC-7" 

# --- 目标板卡硬件配置 ---
# AFC-7 智能飞控板IP地址，用于SSH调试
export AFC7_BOARD_IP="192.168.0.10"
# AFC-7 智能飞控板SSH登录用户名
export AFC7_BOARD_USER="root"
# AFC-7 智能飞控板SSH登录用户密码
export AFC7_BOARD_KEY="1234"
# AFC-7 智能飞控板网络端口号
export AFC7_BOARD_PORT="22" 

# --- 硬件调试与工具链参数 ---
# M33核的ST-Link编程器VID和PID号
export ST_LINK_VID_PID="0483:3748"
# M33核的GDB调试器名称
export AFC7_GDB="gdb-multiarch"
# AFC-7远程访问路径
export AFC7_REMOTE_PATH="/home/root" 

# --- OpenAMP 异构路径修补配置 (相对于项目工程根目录（如NakedAFC7）) --
# 若未来 CubeMX 升级修复或更改了路径结构，仅需在此处修改，工程 CMake 自动生效
export AFC7_OAMP_SYS_INC="Middlewares/Third_Party/OpenAMP/libmetal/lib/metal/system/generic"
export AFC7_OAMP_PROC_INC="Middlewares/Third_Party/OpenAMP/libmetal/lib/metal/processor/arm"
```
### 1.5 M33 纯净沙盒依赖初始化
为保证 M33 裸核开发环境不被庞大的 Linux A35 SDK 污染，且兼容 CubeMX 对高版本 CMake (3.22+) 的强制要求，必须构建独立的裸机交叉编译沙盒。
在 WSL2 终端执行以下指令（引入 CMake 现代版官方源并部署 ARM 裸机神装）：
```Bash
# 1. 引入 Kitware 官方源以获取最新版 CMake
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null
echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ focal main' | sudo tee /etc/apt/sources.list.d/kitware.list >/dev/null

# 2. 更新源并一次性安装 M33 核心工具链
sudo apt update
sudo apt install cmake gcc-arm-none-eabi build-essential openocd binutils-multiarch -y
```
> **🚨 工具链隔离纪律：** 严禁在 M33 工程的编译终端中执行 `source` 加载 A35 的 SDK 环境脚本，必须保持 M33 沙盒的绝对纯净！
## 1.6 硬件调试器静默挂载 (VBS 开机自动化)
默认状态下 WSL2 无法访问物理 USB。利用 `usbipd` 配合 VBS 脚本，可实现 ST-Link 开机后台无感自动穿透，实现“插板即调试”。
1. **打开启动目录**：Windows 侧按 `Win + R`，输入 `shell:startup` 回车。    
2. **创建静默脚本**：新建文本文档，重命名为 `AFC7_USB_AutoAttach.vbs` (确保扩展名为 .vbs)。    
3. **注入核心指令**：右键编辑，贴入以下 2 行代码并保存：    
```VBScript
    Set ws = CreateObject("Wscript.Shell")
    ws.Run "powershell.exe -WindowStyle Hidden -Command ""usbipd attach --wsl --hardware-id 0483:3748 --auto-attach""", 0, False
```
  _注：`0483:3748` 为 ST-Link 硬件识别码。配置完成后，无论何时插入开发板，底层均自动完成 WSL 挂载，绝无黑框弹窗打扰。_
### 1.7 VS Code 必备平台开发插件
请在 VS Code 扩展市场安装以下插件，并确保它们在 `Enabled in WSL: Ubuntu-20.04` 状态：
1. **WSL** (Microsoft) - 必备底层容器支撑。    
2. **C/C++ Extension Pack** (Microsoft) - 提供 IntelliSense 代码感知。    
3. **Cortex-Debug** (marus25) - M33 裸核底层通信与硬件断点核心引擎。    
4. **STM32CubeIDE Extension for VS Code** (STMicroelectronics) - `.ioc` 工程解析支撑。
### 1.8 旧版环境无缝迁移指南
针对早期使用 `myir_sdk` 命名规范的开发者，请按以下 4 步无损迁移至新版 `.profile` 架构：
1. **物理目录更名**： `cd ~ && mv myir_sdk st_mp2_sdk`    
2. **剥离旧版耦合**： 使用 `nano ~/.bashrc` 删除文件末尾所有涉及 `myir_sdk` 的 `source` 或 `export` 指令。    
3. **拥抱新架构**： 按照本指南 **0.4 节**，重新拷贝 `~/.afc7_env`，并向 `~/.profile` 写入启动项。    
4. **清理编译缓存残留**： 在 VS Code 全局搜索替换旧路径后，**必须清空**由于硬编码路径导致的缓存： `rm -rf ~/AFC-7/03MyLd25XM33/NakedAFC7/CM33/build/*` 最后执行 `wsl --shutdown` 重启系统完成环境刷新。
## 🔗 2. 硬件网络链路与总线物理透传
(1) 以太网链路：宿主机网口直连目标板 ETH1 (J9)。宿主机静态 IP 配置为 192.168.0.2，目标板默认 IP 为 192.168.0.10。 
(2) 调试总线透传 (ST-Link)：每次主机重启或插拔调试器后，必须在 Windows PowerShell (管理员权限) 重新确权透传： 
```PowerShell
usbipd list                  # 获取 ST-Link 的 BUSID (例如 2-1)
usbipd bind -b 2-1           # 绑定物理设备
usbipd attach --wsl -b 2-1   # 强行透传至 WSL2 实例
```
(3) WSL2 设备节点提权：
```Bash
sudo chmod -R 777 /dev/bus/usb/ # OpenOCD 访问控制授权
```
## 🛡️ 3. 异构系统调试隔离架构
针对多核安全机制，必须严格隔离 A35 与 M33 的 GDB 调试端口，防止调试器全局复位引发系统内核态异常。工程包含 scripts/debug/ 配置源：
(1) 多核调试掩码配置 (m33_debug.cfg)
```Tcl
stm32mp25x.a35_0 configure -gdb-port disabled
stm32mp25x.a35_1 configure -gdb-port disabled
stm32mp25x.m0p configure -gdb-port disabled
stm32mp25x.m33 configure -gdb-port 50000
reset_config none
```
(2) 调试环境封装脚本 (.afc7_openocd.sh)
```Bash
#!/bin/bash
source ${ST_MP2_SDK_PATH}/environment-setup-cortexa35-ostl-linux
exec openocd "$@"
```
## 🚀 4. A35 核 (Linux) 应用层验证

### 4.1 测试工程初始化

WSL 挂载：点击 VS Code 左下角远端指示器，选择 Connect to WSL，确权当前上下文为 WSL: Ubuntu-20.04。
工程构件同步：
在终端执行 cd $AFC7_SYSTEM_ROOT && explorer.exe .。将外部验证例程文件夹（TAFC7HelloWorld 等，含 .vscode 目录）移入该路径。
### 4.2 验证例程 1：全链路构建与自动部署测试
目标：验证 Makefile 管线与自动下发链路。
操作：打开 TAFC7HelloWorld 文件夹，执行 Ctrl + Shift + B，选中 Build, Deploy & Run。
确证事实：终端输出编译 Log 并完成 scp 上传，目标板实时回传 [AFC-7] HelloWorld Test OK! 字符串。 
### 4.3 验证例程 2：多线程与源码级在线调试测试
目标：验证 GDB 远端挂载与符号表映射。
操作：在 TMFileMThreat 工程中打开 main.c，在目标逻辑行注入断点。
调试序列：键入 F5 启动调试。
确证事实：程序计数器 (PC) 精准停驻于断点处。通过 F10 步进观测 VARIABLES 窗口，变量内存值应实时更新。
### 4.4 验证例程 3：TLD257 硬件功能综合测试
基于构建的AFC-7 **“异构多核跨平台集成”** 开发架构，本测试项目针对米尔 MYD-LD25X (STM32MP257) 开发板 A35 核心的 Linux 硬件接口综合测试工程。工程采用**“顶层交互菜单 + 底层独立模块”**架构，函数与文件命名遵循 `驼峰命名法`，专有名词（如 TLD257）全大写，测试接口统一以 `T` 引导。**

详情可参见**TLD257HWFUN项目中的readme.md文件**。

## ⚙️ 5. M33 核 (FreeRTOS) 裸核极限调试
M33 协处理器的生命周期依托 A35 的 remoteproc 框架。传统复位调试会导致 IPCC 状态机崩溃，本平台采用 无痕挂载 (Attach) 与软件自旋阻塞 (Spin-Wait Trap) 机制。
### 5.1 构建软件自旋临界区
需在 main.c 核心初始化阶段建立拦截路障。规范要求：自旋锁必须位于全局中断隔离机制之内，防止历史幽灵中断风暴。 

```C
  /* [验证任务 3] 异构核间生命周期同步与确权 */
  if (!IS_DEVELOPER_BOOT_MODE())
  {
    __disable_irq();    /* 屏蔽中断响应，防止遗留标志位引发风暴 */
    MX_IPCC1_Init();
    CoproSync_Init();   /* 完成核间协议握手及底层状态机清理 */
    __enable_irq();     /* 退出临界区，安全开启中断 */
  }
  /* 注入自旋锁阻塞逻辑：拦截程序执行流，等待 GDB Attach 状态同步 */
  volatile int dbg_wait = 1;
  while(dbg_wait);
  /* 启动系统调度器 */
  osKernelStart();
```
### 5.2 调试操作序列
(1) 启动调试任务 (F5)。系统将自动完成编译、remoteproc 唤醒及 OpenOCD 监听。
(2) 待调试界面弹出后，点击 暂停 (Pause) ⏸️ 强制获取当前程序计数器 (PC)，此时指令必停驻于 while(dbg_wait)。
(3) 在 VS Code VARIABLES 窗口将 dbg_wait 值由 1 修改为 0。
(4) 点击 继续 (Continue)，程序脱离阻塞态，精确触发后续业务断点。 

## 🔄 6. 工程迭代与开源合规规范
(1) 模块继承化：新建独立逻辑层工程时，必须直接克隆平台基础工程内的 .vscode/ 配置文件与 Makefile 脚本，保障跨核编译时序逻辑的绝对一致性。
(2) 发布流水线：更新开源仓库时，禁止手动提交。请采用第7节git方式提交。

## 💡 7. 软件版本管控(https://github.com/zqhkd/AFC-7Published/releases/tag/V1.0.0)
### 🚨7.1 每日基础流水线（全员适用）
**拉取远程更新**（防止和同事冲突）：
```Bash
  git pull origin master  # 普通成员用 master，核心成员用 develop_core
```
**查看当前状态**（看看改了哪些文件）：
```Bash
  git status              # 查看哪些文件变了
  git diff --stat        # 查看每个文件的改动行数（摘要）
  git diff TLD257RtTask.c # 查看核心调度逻辑的具体改动（如果怕忘代码细节）
```
**日常开发（修改代码后）**（看看改了哪些文件）：
```Bash
git add .
git commit -m "Fix: [A35] 描述您的具体改动"
git push origin main
```

**关键节点归档**（如完成某个算法模块）：
```Bash
# 1. 打标签（建议版本号递增）
git tag -a V1.1-Algo_Alpha -m "完成了XX飞控算法初步部署"
# 2. 连同标签一起推送
git push origin main --tags
```

- **换台电脑同步环境** （新入职博士生）**
```Bash
# 1. 克隆代码
git clone git@github.com:zqhkd/AFC-7Published.git
```
### 🧹7.2  容错与规范戒律
- **写乱了，想放弃本地修改，回退到远程版本**：
```
    git checkout -- main.c  # 放弃 main.c 的本地修改
    # 或者全推倒重来
    git reset --hard origin/master
```    
- **查历史记录（谁在什么时候改了什么）**：
```
    git log --oneline --graph --all
```    
- **删错了文件想找回**：
```
    git checkout [Commit_ID] -- [文件名]

**⚠️  版 权 声 明 © 2026 曾庆华/ACG团队 保留所有权利 ⚠️**
禁止未经授权转载、复制、修改、商用及传播
