#!/bin/bash

# ==============================================================================
# 脚本名称: AFC7LinuxBSPSetUp.sh
# 功能描述: AFC-7 智能飞控板 - 固件与双启动环境一键部署工具 (团队/用户交付版)
# 注意事项：运行时必须要🔴切换到该文件所在目录⛔，因为它还要处理该目录下其它相关文件
# 运行环境: 宿主机 (WSL2 / Linux / macOS)
# 出品单位: 卓飞智控技术有限公司 (Zhuofei Zhikong Technology)
# ==============================================================================

# 定义目标开发板默认 IP 地址与通用账户
TARGET_IP="192.168.0.10"
TARGET_USER="root"

echo -e "\e[1;36m=================================================================\e[0m"
echo -e "\e[1;36m       🚀 欢迎使用 AFC-7 飞控板 BSP 一键自动化部署工具 v1.1\e[0m"
echo -e "\e[1;36m=================================================================\e[0m"
echo -e "正在初始化与目标开发板 (\e[1;35m${TARGET_USER}@${TARGET_IP}\e[0m) 的安全通道..."
echo ""

# ------------------------------------------------------------------------------
# 【第一阶段：前置固件与静态资源推送】
# ------------------------------------------------------------------------------

# 1. 推送底层设备树固件
echo -e "\e[1;33m[1/7] 正在推送设备树固件至 /boot/...\e[0m"
scp myb-afc7-2GB.dtb ${TARGET_USER}@${TARGET_IP}:/boot/
if [ $? -ne 0 ]; then echo -e "\e[1;31m[-] 步骤 1 失败，请检查网络连接或 dtb 文件是否存在！\e[0m"; exit 1; fi

# 2. 推送双启动引导配置文件 (确立双启动防变砖机制)
echo -e "\e[1;33m[2/7] 正在推送双启动 extlinux 引导配置...\e[0m"
scp myb-stm32mp257x-2GB_extlinux.conf ${TARGET_USER}@${TARGET_IP}:/boot/mmc1_extlinux/
if [ $? -ne 0 ]; then echo -e "\e[1;31m[-] 步骤 2 失败，请检查引导配置文件！\e[0m"; exit 1; fi

# 3. 推送专属串口欢迎界面
echo -e "\e[1;33m[3/7] 正在推送 AFC-7 专属串口欢迎框架...\e[0m"
scp issue ${TARGET_USER}@${TARGET_IP}:/etc/issue

# 4. 发送并更新 TSN 功能加载与测试脚本
echo -e "\e[1;33m[4/7] 正在同步 TSN 网络核心加载与功能裁剪脚本...\e[0m"
scp ttt-ip-init-systemd.sh ${TARGET_USER}@${TARGET_IP}:/usr/sbin/ttt-ip-init-systemd.sh
ssh ${TARGET_USER}@${TARGET_IP} "chmod +x /usr/sbin/ttt-ip-init-systemd.sh"

# ------------------------------------------------------------------------------
# 【第二阶段：核心重构与底层安全分区强刷】
# ------------------------------------------------------------------------------

# 5. 关闭丢弃的旧 eeprom 芯片服务
echo -e "\e[1;33m[5/7] 正在执行系统净化：关闭并屏蔽冗余外设服务...\e[0m"
ssh ${TARGET_USER}@${TARGET_IP} "systemctl disable eeprom-pnsn.service && systemctl mask eeprom-pnsn.service" 2>/dev/null

# 6. 安全推送底层 FIP 固件映像
echo -e "\e[1;33m[6/7] 正在向暂存区推送 FIP 核心固件 (打通 M33 侧 HPDMA 越界访问权限)...\e[0m"
scp fip-myb-stm32mp257x-2GB-optee-emmc.setup ${TARGET_USER}@${TARGET_IP}:/home/root/fip-myb-stm32mp257x-2GB-optee-emmc.bin
if [ $? -ne 0 ]; then echo -e "\e[1;31m[-] FIP 固件映像文件传输失败，中止后续写入！\e[0m"; exit 1; fi

# 7. 闭环执行原子强刷指令 (核心机制)
echo -e "\e[1;33m[7/7] 正在向 eMMC 安全启动分区(p3/p4)强刷 FIP 固件...\e[0m"
echo -e "\e[1;34m    ⚡ [正在写入 mmcblk1p3 & p4，请勿断开飞控板电源]...\e[0m"

# 通过单次 SSH 会话串联解密、写入与同步，并在板载侧实现失败拦截
ssh ${TARGET_USER}@${TARGET_IP} "
    if [ -f '/sys/block/mmcblk1/holders/mmcblk1p3/../ro' ]; then
        echo 0 > /sys/block/mmcblk1/holders/mmcblk1p3/../ro 2>/dev/null
        echo 0 > /sys/block/mmcblk1/holders/mmcblk1p4/../ro 2>/dev/null
    fi && \
    dd if=/home/root/fip-myb-stm32mp257x-2GB-optee-emmc.bin of=/dev/mmcblk1p3 bs=4096 conv=fdatasync 2>/dev/null && \
    dd if=/home/root/fip-myb-stm32mp257x-2GB-optee-emmc.bin of=/dev/mmcblk1p4 bs=4096 conv=fdatasync 2>/dev/null && \
    rm -f /home/root/fip-myb-stm32mp257x-2GB-optee-emmc.bin && \
    sync
"
if [ $? -ne 0 ]; then
    echo -e "\e[1;41m ❌ CRITICAL ERROR: FIP 固件分区写入失败！ \e[0m"
    echo -e "\e[1;31m[原因] 可能是由于 eMMC 块设备写保护或存储异常引起。\e[0m"
    echo -e "\e[1;31m[安全保护] 已紧急拦截重启服务。请保持飞控板供电，并联系技术负责人排查！\e[0m"
    exit 1
fi
echo -e "\e[1;32m    ✨ FIP 安全分区强刷成功，硬件权属关系已刷新。\e[0m"

# ------------------------------------------------------------------------------
# 【第三阶段：硬件全景巡检配置与重启】
# ------------------------------------------------------------------------------

# 8. 推送硬件巡检工具
echo -e "\e[1;33m[*] 正在配置全景硬件资源巡检脚本...\e[0m"
scp check_afc7_hw.sh ${TARGET_USER}@${TARGET_IP}:~/

# 9. 状态净化验证、赋予权限并最终重启
echo -e "\e[1;33m[*] 校验系统净化状态并下发自动重启指令...\e[0m"
ssh ${TARGET_USER}@${TARGET_IP} "
    echo -e '\e[1;34m--- 冗余服务裁剪净化验证 ---\e[0m';
    systemctl status eeprom-pnsn.service 2>&1 | grep -E 'Loaded:|Active:';
    if systemctl list-unit-files 2>/dev/null | grep -q st-thermal.service; then
        systemctl status st-thermal.service 2>&1 | grep -E 'Loaded:|Active:';
    else
        echo '    st-thermal.service (温控连带服务) 已完成纯净化移除。';
    fi
    echo -e '\e[1;34m------------------------------------\e[0m';
    cd ~ && chmod +x check_afc7_hw.sh && sync && reboot
"

echo -e "\e[1;32m=================================================================\e[0m"
echo -e "\e[1;32m   🎉 AFC-7 系统一键部署完毕！开发板正在进行热重启...           \e[0m"
echo -e "\e[1;32m   重启完成后，请引导用户/同事登录终端执行以下核验：               \e[0m"
echo -e "\e[1;32m   1. 运行 \e[1;33mdmesg | head -n 5\e[1;32m 确认 Machine Model 为 AFC-7 \e[0m"
echo -e "\e[1;32m   2. 运行 \e[1;35m./check_afc7_hw.sh\e[1;32m 闭环进行硬件资源权属确认。  \e[0m"
echo -e "\e[1;32m=================================================================\e[0m"