#!/bin/bash

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'
BOLD='\033[1m'

DIR1="/home/zqhkd/AFC-7/CubeMxPostProcess"
DIR2="/home/zqhkd/AFC-7/AFCTarget"
EXCLUDE_FILES=("deployAFC7.sh" "compareDiffs.sh")

# 显示帮助函数
show_help() {
    echo "使用方法: $0 [选项]"
    echo ""
    echo "选项:"
    echo "  -s    简洁模式（只列出差异文件，不显示详细对比）"
    echo "  -h, --help    显示此帮助信息"
    echo ""
    echo "默认模式（无参数）: 详细模式，显示 unified diff 格式的差异详情"
    echo ""
    echo "示例:"
    echo "  $0          # 详细模式"
    echo "  $0 -s       # 简洁模式"
    echo "  $0 -h       # 显示帮助"
    echo ""
    echo "说明:"
    echo "  以 CubeMxPostProcess 为基准，检查每个文件是否在 AFCTarget 中存在且内容相同"
    echo "  排除文件: ${EXCLUDE_FILES[@]}"
    echo ""
    echo "差异显示说明:"
    echo "  ${RED}红色${NC}行: 左侧（CubeMxPostProcess）独有的内容"
    echo "  ${GREEN}绿色${NC}行: 右侧（AFCTarget）独有的内容"
    echo "  普通行: 上下文内容"
}

# 解析命令行参数
VERBOSE=true
while [[ $# -gt 0 ]]; do
    case $1 in
        -s)
            VERBOSE=false
            shift
            ;;
        -h|--help)
            show_help
            exit 0
            ;;
        *)
            echo "未知选项: $1"
            echo "使用 '$0 -h' 查看帮助"
            exit 1
            ;;
    esac
done

TOTAL=0
MISSING=0
DIFF=0

echo ""
echo -e "${BOLD}     文件比较工具compareDiffs ${NC}"
echo -e "${GREEN}基准目录:${NC} $DIR1"
echo -e "${YELLOW}目标目录:${NC} $DIR2"
echo -e "${DIM}排除文件: ${EXCLUDE_FILES[@]}${NC}"
echo -e "${BOLD}${CYAN}════════════════════════════════════════════════════════════════════════════${NC}"
echo -e "${BOLD}文件结果如下："

while IFS= read -r file1; do
    rel_path="${file1#$DIR1/}"
    
    skip=0
    for exclude in "${EXCLUDE_FILES[@]}"; do
        if [ "$rel_path" = "$exclude" ] || [ "$(basename "$rel_path")" = "$exclude" ]; then
            skip=1
            break
        fi
    done
    [ $skip -eq 1 ] && continue
    
    TOTAL=$((TOTAL + 1))
    file2="$DIR2/$rel_path"
    
    if [ ! -f "$file2" ]; then
        echo -e "${YELLOW}❌ 缺失:${NC} $rel_path"
        MISSING=$((MISSING + 1))
    elif ! cmp -s "$file1" "$file2"; then
        echo -e "${RED}⚠️  不同: $rel_path${NC}"
        DIFF=$((DIFF + 1))
        
        if [ "$VERBOSE" = true ]; then
            echo ""
            echo -e "${BOLD}${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
            echo -e "${BOLD}差异详情 (统一格式):${NC}"
            echo -e "${BOLD}${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
            diff -u "$file1" "$file2" | head -40 | while IFS= read -r line; do
                if [[ $line == -* ]]; then
                    echo -e "${RED}$line${NC}"
                elif [[ $line == +* ]]; then
                    echo -e "${GREEN}$line${NC}"
                else
                    echo "$line"
                fi
            done
            echo ""
        fi
    fi
done < <(find "$DIR1" -type f)

echo -e "${BOLD}════════════════════════════════════════════════════════════════════════════${NC}"
echo -e "${BOLD}📊 统计:${NC} 总文件=$TOTAL, 缺失=$MISSING, 不同=$DIFF"

if [ $MISSING -eq 0 ] && [ $DIFF -eq 0 ]; then
    echo -e "${GREEN}✅ 所有文件完全一致${NC}"
else
    echo -e "${RED}❌ 发现 $((MISSING + DIFF)) 个问题${NC}"
fi
echo ""