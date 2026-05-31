#!/bin/bash
# ==========================================================================
# 🚨 AFC-7 平台低空开发环境一键部署程序 (第一部分：环境审计与引导链配置)
# ==========================================================================

# --------------------------------------------------------------------------
# 🛠️ 核心出厂默认值集中配置区 (开发人员在此一键维护，支持多用户自适应)
# --------------------------------------------------------------------------
# --- SDK 与项目根目录 (保留字面量 $HOME 以实现自适应) ---
DEF_ST_MP2_SDK_PATH='$HOME/st_mp2_sdk'
DEF_AFC7_SYSTEM_ROOT='$HOME/AFC-7'
DEF_AFC7_WSL_USER='zqhkd'
DEF_AFC7_WSL_KEY='3.1415926'

# --- 目标板卡硬件配置 ---
DEF_AFC7_BOARD_IP='192.168.1.10'
DEF_AFC7_BOARD_USER='root'
DEF_AFC7_BOARD_KEY='1234'
DEF_AFC7_BOARD_PORT='22'

# --- 硬件调试与工具链参数 ---
DEF_ST_LINK_VID_PID='0483:3748'
DEF_AFC7_GDB='gdb-multiarch'
DEF_AFC7_REMOTE_PATH='/home/root'

# --- OpenAMP 异构路径修补配置 ---
DEF_AFC7_OAMP_SYS_INC='Middlewares/Third_Party/OpenAMP/libmetal/lib/metal/system/generic'
DEF_AFC7_OAMP_PROC_INC='Middlewares/Third_Party/OpenAMP/libmetal/lib/metal/processor/arm'

# 目标写入位置
TARGET_ENV_FILE="$HOME/.afc7_env"
PROFILE_FILE="$HOME/.profile"

echo -e "\n====================== 🚀 [ AFC-7 WSL2 环境一键部署 ] ======================"
echo -e "[1/2] 正在加载并审查平台默认环境字典...\n"

# --------------------------------------------------------------------------
# 功能 1: 使用精美表格向用户展示即将注入的环境变量字典
# --------------------------------------------------------------------------
echo "--------------------------------------------------------------------------------------"
printf "| %-25s | %-52s |\n" "AFC-7 环境变量名称" "即将注入的出厂默认值 (Default Value)"
echo "--------------------------------------------------------------------------------------"
printf "| %-25s | %-52s |\n" "ST_MP2_SDK_PATH" "$DEF_ST_MP2_SDK_PATH"
printf "| %-25s | %-52s |\n" "AFC7_SYSTEM_ROOT" "$DEF_AFC7_SYSTEM_ROOT"
printf "| %-25s | %-52s |\n" "AFC7_WSL_USER" "$DEF_AFC7_WSL_USER"
printf "| %-25s | %-52s |\n" "AFC7_WSL_KEY" "$DEF_AFC7_WSL_KEY"
echo "|---------------------------|--------------------------------------------------------|"
printf "| %-25s | %-52s |\n" "AFC7_BOARD_IP" "$DEF_AFC7_BOARD_IP"
printf "| %-25s | %-52s |\n" "AFC7_BOARD_USER" "$DEF_AFC7_BOARD_USER"
printf "| %-25s | %-52s |\n" "AFC7_BOARD_KEY" "$DEF_AFC7_BOARD_KEY"
printf "| %-25s | %-52s |\n" "AFC7_BOARD_PORT" "$DEF_AFC7_BOARD_PORT"
echo "|---------------------------|--------------------------------------------------------|"
printf "| %-25s | %-52s |\n" "ST_LINK_VID_PID" "$DEF_ST_LINK_VID_PID"
printf "| %-25s | %-52s |\n" "AFC7_GDB" "$DEF_AFC7_GDB"
printf "| %-25s | %-52s |\n" "AFC7_REMOTE_PATH" "$DEF_AFC7_REMOTE_PATH"
echo "|---------------------------|--------------------------------------------------------|"
printf "| %-25s | %-52s |\n" "AFC7_OAMP_SYS_INC" "... /generic (详见脚本头部定义)"
printf "| %-25s | %-52s |\n" "AFC7_OAMP_PROC_INC" "... /arm (详见脚本头部定义)"
echo "--------------------------------------------------------------------------------------"

read -p "👉 是否确认将上述 13 项默认配置直接注入至系统环境？[Y/n]: " choice
choice=${choice:-Y}

if [[ "$choice" =~ ^[Yy]$ ]]; then
    # 🚨 核心防坑设计：通过 'EOF' (带单引号) 锁定文本块，
    # 确保写入文本时内部的 $HOME 字符串原样落地，绝不提前展开为当前用户名路径！
    cat << 'EOF' > "$TARGET_ENV_FILE"
# ==========================================
# AFC-7 平台全局环境变量配置
# ==========================================

# --- SDK 与项目根目录 (波浪线自适应) ---
# ST MP2 官方交叉编译 SDK 绝对路径
EOF
    # 动态拼接内容块，保证多用户自适应通用
    echo "export ST_MP2_SDK_PATH=\"$DEF_ST_MP2_SDK_PATH\"" >> "$TARGET_ENV_FILE"
    echo "export AFC7_SYSTEM_ROOT=\"$DEF_AFC7_SYSTEM_ROOT\"" >> "$TARGET_ENV_FILE"
    echo "export AFC7_WSL_USER=\"$DEF_AFC7_WSL_USER\"" >> "$TARGET_ENV_FILE"
    echo "export AFC7_WSL_KEY=\"$DEF_AFC7_WSL_KEY\"" >> "$TARGET_ENV_FILE"

    cat << 'EOF' >> "$TARGET_ENV_FILE"

# --- 目标板卡硬件配置 ---
EOF
    echo "export AFC7_BOARD_IP=\"$DEF_AFC7_BOARD_IP\"" >> "$TARGET_ENV_FILE"
    echo "export AFC7_BOARD_USER=\"$DEF_AFC7_BOARD_USER\"" >> "$TARGET_ENV_FILE"
    echo "export AFC7_BOARD_KEY=\"$DEF_AFC7_BOARD_KEY\"" >> "$TARGET_ENV_FILE"
    echo "export AFC7_BOARD_PORT=\"$DEF_AFC7_BOARD_PORT\"" >> "$TARGET_ENV_FILE"

    cat << 'EOF' >> "$TARGET_ENV_FILE"

# --- 硬件调试与工具链参数 ---
EOF
    echo "export ST_LINK_VID_PID=\"$DEF_ST_LINK_VID_PID\"" >> "$TARGET_ENV_FILE"
    echo "export AFC7_GDB=\"$DEF_AFC7_GDB\"" >> "$TARGET_ENV_FILE"
    echo "export AFC7_REMOTE_PATH=\"$DEF_AFC7_REMOTE_PATH\"" >> "$TARGET_ENV_FILE"

    cat << 'EOF' >> "$TARGET_ENV_FILE"

# --- OpenAMP 异构路径修补配置 (相对于项目工程根目录) ---
EOF
    echo "export AFC7_OAMP_SYS_INC=\"$DEF_AFC7_OAMP_SYS_INC\"" >> "$TARGET_ENV_FILE"
    echo "export AFC7_OAMP_PROC_INC=\"$DEF_AFC7_OAMP_PROC_INC\"" >> "$TARGET_ENV_FILE"

    echo -e "\n\033[32m✅ 写入成功！全新环境字典已安全映射至: $TARGET_ENV_FILE\033[0m"
else
    echo -e "\n\033[33m💡 操作已取消。后续开发人员可直接编辑本脚本头部的【默认值集中配置区】进行路径定制。\033[0m\n"
    exit 0
fi

# --------------------------------------------------------------------------
# 功能 2: 引导链幂等性审计 (精准拦截，绝不重复污染 ~/.profile)
# --------------------------------------------------------------------------
echo -e "\n🔬 正在对系统的全局引导链 ~/.profile 进行幂等性审计..."

BLOCK_MARKER="加载 AFC-7 平台全局环境变量"

if grep -q "$BLOCK_MARKER" "$PROFILE_FILE" 2>/dev/null; then
    echo -e "⚡ \033[32m[就绪] 检测到 ~/.profile 中已包含 AFC-7 环境加载链，跳过注入，避免重复添加。\033[0m"
else
    echo "🚀 正在向 ~/.profile 末尾追加防刷新的确定性自适应引导级指令..."
    cat << 'EOF' >> "$PROFILE_FILE"

# ==========================================
# 加载 AFC-7 平台全局环境变量
# （放这里确保 WSL 和 VS Code 启动时全局生效）
# ==========================================
if [ -f "$HOME/.afc7_env" ]; then
    source "$HOME/.afc7_env"
fi
EOF
    echo -e "\033[32m✅ 系统引导链注入成功！\033[0m"
fi

echo -e "\n==================== 🏁 [ 第一部分：环境配置完成 ] ===================="
echo " 1. 全局环境节点已安全生成: $TARGET_ENV_FILE"
echo " 2. 系统启动引导链已闭环对齐: $PROFILE_FILE"
echo " 💡 指引：请在当前终端运行 'source ~/.profile' 以激活全新的环境变量。"
echo -e "=========================================================================\n"