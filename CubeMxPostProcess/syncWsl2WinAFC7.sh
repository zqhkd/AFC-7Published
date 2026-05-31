#!/bin/bash
# /******************** (C) COPYRIGHT 2026 AFC Tech Co.*************************
#  * 作    者  ： 曾庆华
#  * 文 件 名 ： syncWsl2WinAFC7.sh
#  * 版    本 ： V6.0 【最终永不报错版】
#  * 描    述 ： 双参数传入，不依赖环境变量，自动转换路径
#  * 运行环境 ： WSL2 (Ubuntu)
# *****************************************************************************/

EXCLUDE_LIST=(
    ".vscode"
    "build"
    "CM33/build"
    "*.o"
    "*.d"
    "*.elf"
    "*.bin"
    "*.hex"
    ".git"
)

PROJ_NAME="$1"    # 注意tasks.json中哑元必须是\"${workspaceFolderBasename}\"，而不是\"${workspaceFolder}\"
if [ -z "$PROJ_NAME" ]; then
    echo "❌ 用法：$0 <项目名>"
    exit 1
fi

echo "🔍 读取 Win11 环境变量 AFCProjectPath..."
RAW=$(powershell.exe -Command "[Environment]::GetEnvironmentVariable('AFCProjectPath','User')" | tr -d '\r\n')
WIN_BASE=$(wslpath -u "$RAW")
WIN_DEST="$WIN_BASE/$PROJ_NAME"
WSL_SRC="$AFC7_SYSTEM_ROOT/$PROJ_NAME"

echo "====================================================="
echo " 项目名    : $PROJ_NAME"
echo " WSL 源码  : $WSL_SRC"
echo " Win 目标  : $WIN_DEST"
echo "====================================================="

read -p "❓ 确定同步代码回 Win11？(y/n): " confirm
[[ "$confirm" != "y" ]] && exit 0

EXCLUDE_ARGS=""
for item in "${EXCLUDE_LIST[@]}"; do
    EXCLUDE_ARGS+=" --exclude='$item'"
done

# ===================== 【安全】同步时自动转 Windows 换行符 =====================
echo -e "\n🔄 同步 CM33/Core"
eval rsync -av --progress --iconv=utf-8,utf-8 $EXCLUDE_ARGS "$WSL_SRC/CM33/Core/." "$WIN_DEST/CM33/Core/"

# echo -e "\n🔄 同步 Middlewares"
# eval rsync -av --progress --iconv=utf-8,utf-8 $EXCLUDE_ARGS "$WSL_SRC/Middlewares/." "$WIN_DEST/Middlewares/"

echo -e "\n🔄 同步 Common"
eval rsync -av --progress --iconv=utf-8,utf-8 $EXCLUDE_ARGS "$WSL_SRC/Common/." "$WIN_DEST/Common/"

echo -e "\n✅ 同步完成！文件已转为 Windows 格式，CubeMX 安全！"
