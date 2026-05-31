#!/bin/bash
# ==========================================================================
# 🚨 AFC-7 纯 WSL2 侧 ST-LINK 自动穿透与审计脚本 autoAttachSTLink.sh
# ==========================================================================

echo -e "\n======================= 🔍 [ AFC-7 调试前置硬件审计 ] ======================="

# --------------------------------------------------------------------------
# 步骤 1: 环境变量合规性断言 (防盲调看板)
# --------------------------------------------------------------------------
echo "[1/3] 正在读取 AFC-7 平台硬件环境变量..."
echo "      👉 .afc7_env中配置的ST-LINK编程器特征ID: [ $ST_LINK_VID_PID ]"

if [ -z "$ST_LINK_VID_PID" ]; then
    echo -e "\033[31m❌ [错误] 未检测到 \$ST_LINK_VID_PID 环境变量，请检测.afc7_env中的相关配置！\033[0m"
    echo "=========================================================================\n"
    exit 1
fi

# 检查本地 WSL 提权密码是否成功加载
if [ -z "$AFC7_WSL_KEY" ]; then
    echo -e "\033[31m❌ [错误] 未检测到 \$AFC7_WSL_KEY 环境变量，请检测.afc7_env中的相关配置！\033[0m"
    echo "=========================================================================\n"
    exit 1
else
    echo "      👉 本地 WSL 审计通道授权凭证已成功加载。"
fi

# --------------------------------------------------------------------------
# 步骤 2: 精准检测 WSL2 底层 USB 总线 (lsusb 研判)
# --------------------------------------------------------------------------
echo "[2/3] 正在检索本地 USB 总线状态..."

# 精准过滤环境变量中定义的 VID_PID
STLINK_LSUSB_INFO=$(lsusb | grep -i "$ST_LINK_VID_PID")

if [ -n "$STLINK_LSUSB_INFO" ]; then
    echo "      👉 当前总线状态: [ $STLINK_LSUSB_INFO ]"
    echo -e "\033[32m⚡ [硬件状态已就绪] WSL已可访问ST-LINK编程器，正在强制刷新访问特权通道...\033[0m"
    
    # 解析当前已存在节点的总线号和设备号进行精准权限刷新
    BUS_NUM=$(echo "$STLINK_LSUSB_INFO" | awk '{print $2}')
    DEV_NUM=$(echo "$STLINK_LSUSB_INFO" | awk '{print $4}' | tr -d ':')
    DEV_PATH="/dev/bus/usb/$BUS_NUM/$DEV_NUM"
    
    echo "      👉 正在为访问节点 $DEV_PATH 重新赋予 666 运行特权..."
    echo "$AFC7_WSL_KEY" | sudo -S chmod 666 "$DEV_PATH" >/dev/null 2>&1

# 🚨 硬件时序对齐：强制在原地等待 2 秒，确保 A35(Linux-as-BIOS) 已经彻底把 M33 喂饱并钉在 bkpt 门口
    echo "      ⏳ 正在等待板载 A35 侧 Linux 异构固件加载时序对齐 (1s)..."
    sleep 1
    
    echo -e "\033[32m✅ [授权就绪] 访问授权通道刷新成功，可开始调试！\033[0m"
    echo "=========================================================================\n"
    exit 0
else
    echo "      ⚠️  [WSL无法访问硬件] lsusb 中未发现ST-LINK编程器 $ST_LINK_VID_PID，需要执行WSL的USB处理..."
fi

# --------------------------------------------------------------------------
# 步骤 3: 纯 WSL2 侧跨界拉起 usbipd 并赋权
# --------------------------------------------------------------------------
echo "[3/3] 正在通过本地命令行强制激活USB访问的跨平台处理流程..."

# 1. 动态抓取当前 ST-LINK 在 Win11 侧对应的 BUSID (剔除冒号影响，利用 VID/PID 匹配)
VID=$(echo "$ST_LINK_VID_PID" | cut -d':' -f1)
PID=$(echo "$ST_LINK_VID_PID" | cut -d':' -f2)

# 直接调用 Windows 侧的 usbipd.exe list 嗅探 BUSID
WIN_BUSID=$(usbipd.exe list 2>/dev/null | grep -E -i "($VID:$PID|$VID-$PID|ST-Link|STMicroelectronics)" | head -n 1 | awk '{print $1}')
echo "      👉 智能匹配到Win11侧ST-LINK编程器的USB的BUS ID: [ $WIN_BUSID ]"

if [ -z "$WIN_BUSID" ]; then
    echo -e "\033[31m❌ [连接失败] Win11 主机上未发现物理接入的 ST-LINK 编程器，请检查硬件连线！\033[0m"
    echo "=========================================================================\n"
    exit 1
fi

# 2. 跨界执行绑定与穿透 (直接调用 Windows 引擎，免除进程挂载)
echo "      🚀 正在向WSL申请设备跨平台访问能力绑定..."
usbipd.exe bind -b "$WIN_BUSID" >/dev/null 2>&1
usbipd.exe attach --wsl -b "$WIN_BUSID" >/dev/null 2>&1

# 3. 🔄 动态高速轮询
echo "      ⏳ 正在等待 USB 节点在 WSL2 侧动态建立..."
for i in {1..20}; do   # 最高等待 2 秒（20 * 0.1s）
    NEW_LSUSB_INFO=$(lsusb | grep -i "$ST_LINK_VID_PID")
    if [ -n "$NEW_LSUSB_INFO" ]; then
        break
    fi
    sleep 0.1
done

if [ -n "$NEW_LSUSB_INFO" ]; then
    echo "      👉 跨平台绑定成功，它在WSL终端的访问节点信息: [ $NEW_LSUSB_INFO ]"
    
    # 解析动态生成的总线号和设备号进行精准 chmod
    BUS_NUM=$(echo "$NEW_LSUSB_INFO" | awk '{print $2}')
    DEV_NUM=$(echo "$NEW_LSUSB_INFO" | awk '{print $4}' | tr -d ':')
    DEV_PATH="/dev/bus/usb/$BUS_NUM/$DEV_NUM"
    
    echo "      👉 正在为访问节点 $DEV_PATH 赋予 666 运行特权..."
    # 🚨 用密码环境变量 $AFC7_WSL_KEY 代替手动输入 sudo 密码（非硬编码自适应）
    echo "$AFC7_WSL_KEY" | sudo -S chmod 666 "$DEV_PATH" >/dev/null 2>&1

    # 🚨 硬件时序对齐：强制在原地等待 2 秒，确保 A35(Linux-as-BIOS) 已经彻底把 M33 喂饱并钉在 bkpt 门口
    echo "      ⏳ 正在等待板载 A35 侧 Linux 异构固件加载时序对齐 (5s)..."
    sleep 5
    
    echo -e "\033[32m✅ [成功连接] ST-LINK 已成功连接并完成WSL调试赋权，可开始调试！\033[0m"
    echo "=========================================================================\n"
    exit 0
else
    echo -e "\033[31m❌ [致命错误] usbipd命令已执行，但 WSL 无法捕获该USB节点！\033[0m"
    echo "=========================================================================\n"
    exit 1
fi