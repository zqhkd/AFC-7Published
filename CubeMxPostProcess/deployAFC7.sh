#!/bin/bash
# /******************** (C) COPYRIGHT 2026 AFC Tech Co.*************************
#  * 作    者  ： 曾庆华
#  * 文 件 名 ： deployAFC7.sh
#  * 版    本 ： V5.0 【终极安全版】
#  * 描    述  ：二次部署只同步Core+Common，绝不覆盖补丁文件
#  * 运行环境 ： WSL2 (Ubuntu)
# *****************************************************************************/

EXCLUDE_LIST_WIN11_FILES=(
    ".mxproject"
    "afc7.inc"
    "cubeBak"
    ".vscode"
)

EXCLUDE_LIST_CUBEMX_POST=(
    "deployAFC7.sh"
    "syncWsl2WinAFC7.sh"
    "tempBak"
    ".git"
    ".gitignore"
    "README.md"
)

SOURCE_ROOT="$1"
if [ -z "$SOURCE_ROOT" ] || [ ! -d "$SOURCE_ROOT" ]; then
    echo "❌ 请传入 Win11 工程目录"
    exit 1
fi

PROJ_NAME=$(basename "$SOURCE_ROOT")
TARGET_DIR="$AFC7_SYSTEM_ROOT/$PROJ_NAME"
PATCH_DIR="$AFC7_SYSTEM_ROOT/CubeMxPostProcess"
INIT_FLAG="$TARGET_DIR/.afc7_initialized"

echo "====================================================="
echo "          AFC7 工程自动部署 V5.0 【终极安全版】"
echo " 工程目录：$PROJ_NAME"
echo " Win ：$SOURCE_ROOT"
echo " WSL：$TARGET_DIR"
echo "====================================================="

read -p "确认执行部署？(y/n): " c
[[ ! $c =~ [yY] ]] && exit 0

WIN_EXCLUDE=""
for f in "${EXCLUDE_LIST_WIN11_FILES[@]}"; do
    WIN_EXCLUDE+=" --exclude='$f'"
done

POST_EXCLUDE=""
for f in "${EXCLUDE_LIST_CUBEMX_POST[@]}"; do
    POST_EXCLUDE+=" --exclude='$f'"
done

# ========================= 【终极逻辑】 =========================
if [ -d "$TARGET_DIR" ] && [ -f "$INIT_FLAG" ]; then
    echo -e "\n[二次部署] 仅同步用户代码：Core + Common（不覆盖任何补丁文件）"
    mkdir -p "$TARGET_DIR"

    # 🔥 🔥 🔥 【你要的终于实现：只同步Core和Common】
    rsync -av $WIN_EXCLUDE "$SOURCE_ROOT/CM33/Core/"   "$TARGET_DIR/CM33/Core/"
    rsync -av $WIN_EXCLUDE "$SOURCE_ROOT/Common/"     "$TARGET_DIR/Common/"
    rsync -av --include='*.ioc' --exclude='*' "$SOURCE_ROOT/" "$TARGET_DIR/"

elif [ ! -d "$TARGET_DIR" ]; then
    echo -e "\n[全新部署] 全量同步 + 补丁初始化"
    mkdir -p "$TARGET_DIR"
    eval rsync -av --delete $WIN_EXCLUDE "$SOURCE_ROOT/." "$TARGET_DIR/"
    rsync -av --include='*.ioc' --exclude='*' "$SOURCE_ROOT/" "$TARGET_DIR/"

    rm -rf "$TARGET_DIR/Middlewares/Third_Party/OpenAMP"
    rm -rf "$TARGET_DIR/Utilities/ResourceManager"
    eval rsync -av $POST_EXCLUDE "$PATCH_DIR/." "$TARGET_DIR/"

    touch "$INIT_FLAG"
else
    echo -e "\n⚠️  目录已存在但未初始化！"
    echo "1) 全新全覆盖  2) 仅同步代码并标记"
    read -p "请选择 1/2：" choice

    mkdir -p "$TARGET_DIR"
    eval rsync -av --delete $WIN_EXCLUDE "$SOURCE_ROOT/." "$TARGET_DIR/"
    rsync -av --include='*.ioc' --exclude='*' "$SOURCE_ROOT/" "$TARGET_DIR/"

    case $choice in
        1)
            rm -rf "$TARGET_DIR/Middlewares/Third_Party/OpenAMP"
            rm -rf "$TARGET_DIR/Utilities/ResourceManager"
            eval rsync -av $POST_EXCLUDE "$PATCH_DIR/." "$TARGET_DIR/"
            touch "$INIT_FLAG"
            ;;
        2)
            touch "$INIT_FLAG"
            ;;
        *)
            exit 1
            ;;
    esac
fi

# 每次必修复
echo -e "\n>>> 修复 CMake & DEBUG 代码"
CMAKE_FILE="$TARGET_DIR/CM33/mx-generated.cmake"
if [ -f "$CMAKE_FILE" ]; then
    sed -i '/# Pre_build commands/,/Executing PRE build command/ s/^[^#]/# &/' "$CMAKE_FILE"
fi

MAIN_FILE="$TARGET_DIR/CM33/Core/Src/main.c"
if [ -f "$MAIN_FILE" ]; then
    sed -i '/#if defined(DEBUG)/,/#endif/d' "$MAIN_FILE"
fi

# 打开VSCode
if command -v code &>/dev/null; then
    echo -e "\n>>> 打开 VSCode 项目"
    DISTRO=$(wsl.exe --list --quiet | head -1 | tr -d '\r')
    code --remote "wsl+$DISTRO" "$TARGET_DIR"
fi

echo -e "\n窗口将在10秒后关闭..."
sleep 10
exit 0