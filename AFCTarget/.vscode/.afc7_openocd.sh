#!/bin/bash

# 1. 显式加载用户环境配置，确保 ST_MP2_SDK_PATH 被引入
if [ -f "$HOME/.profile" ]; then
    source "$HOME/.profile"
fi

# 2. 严格校验变量是否存在
if [ -z "${ST_MP2_SDK_PATH}" ]; then
    echo "-----------------------------------------------------------------------"
    echo "错误: 未检测到环境变量 ST_MP2_SDK_PATH！"
    echo "请检查 WSL2 侧环境配置是否正常："
    echo "1. 确认 ~/.profile 是否正确启动了 .afc7_env"
    echo "2. 确认 .afc7_env 中的 ST_MP2_SDK_PATH 路径是否设置正确"
    echo "-----------------------------------------------------------------------"
    exit 1
fi

# 3. 校验环境脚本文件是否存在
ENV_SCRIPT="${ST_MP2_SDK_PATH}/environment-setup-cortexa35-ostl-linux"
if [ ! -f "$ENV_SCRIPT" ]; then
    echo "错误: 找不到 SDK 环境脚本: $ENV_SCRIPT"
    exit 1
fi

# 4. 加载 SDK 环境并执行 OpenOCD
source "$ENV_SCRIPT"
exec openocd "$@"