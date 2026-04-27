# 定义基准版本
BASE="v0.6.0-DualCore-Baseline"
PRJ_A="01MyirLd257Test/TLD257HWFun"
PRJ_B="AFCA35"

echo "========== AFC-7 框架内核对比报告 (Base: $BASE) ==========" > compare_report.txt

for file in "Makefile" ".vscode/tasks.json" ".vscode/launch.json"; do
    echo "--------------------------------------------------" >> compare_report.txt
    echo "🔍 正在对比文件: $file" >> compare_report.txt
    echo "--------------------------------------------------" >> compare_report.txt
    
    # 执行 diff 并记录结果
    git diff $BASE:$PRJ_A/$file $BASE:$PRJ_B/$file >> compare_report.txt
done

# 新创建的脚本文件默认只有“读写”权限，没有“执行”权限，所以需要运行下面语句更改权限：chmod +x filesCompare.sh
echo "✅ 对比完成！请执行 'cat compare_report.txt' 查看结果。"