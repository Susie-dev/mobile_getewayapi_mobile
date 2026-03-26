et -euo pipefail

# ===================== 核心配置项（按需修改）=====================
SERVER_IP="192.168.211.134"  # 目标服务器IP
SERVER_PORT=5007              # 服务器端口
BASE_CONCURRENT=30            # 基础并发客户端数（原脚本的30个）
MAX_CONCURRENT=100            # 测试极限时的最大并发数
STEP_CONCURRENT=10            # 每次递增的并发数（如30→40→50...）
CLIENT_BIN="./client"         # client程序路径
LOG_FILE="./reactor_client.log"  # 客户端日志文件
PID_FILE="./client_pids.txt"   # 记录客户端进程PID的文件
# =================================================================

# 函数：检查client程序是否存在
check_client() {
    if [ ! -x "$CLIENT_BIN" ]; then
        echo "错误：客户端程序 $CLIENT_BIN 不存在或无执行权限！"
        exit 1
    fi
    rm -f $PID_FILE $LOG_FILE  # 清空历史PID和日志
}

# 函数：启动指定数量的客户端
start_clients() {
    local concurrent=$1
    echo -e "\n========== 启动 ${concurrent} 个客户端 ==========" | tee -a $LOG_FILE
    echo "启动时间: $(date +'%Y-%m-%d %H:%M:%S')" | tee -a $LOG_FILE

    # 批量启动客户端，记录PID
    for ((i=1; i<=concurrent; i++)); do
        $CLIENT_BIN $SERVER_IP $SERVER_PORT >> $LOG_FILE 2>&1 &
        echo $! >> $PID_FILE  # 保存进程ID
        # 轻微延迟，避免瞬间创建大量连接导致端口耗尽
        sleep 0.01
    done

    # 验证启动的进程数
    local running_pids=$(wc -l < $PID_FILE)
    echo "实际启动客户端数: $running_pids (预期: $concurrent)" | tee -a $LOG_FILE
    if [ $running_pids -ne $concurrent ]; then
        echo "警告：部分客户端启动失败！" | tee -a $LOG_FILE
    fi
}

# 函数：等待客户端运行完成，统计结果
wait_clients() {
    local concurrent=$1
    echo -e "\n========== 等待 ${concurrent} 个客户端执行完成 ==========" | tee -a $LOG_FILE
    start_time=$(date +%s)

    # 等待所有客户端进程结束
    while read -r pid; do
        if ps -p $pid > /dev/null; then
            wait $pid
            exit_code=$?
            if [ $exit_code -eq 0 ]; then
                success_count=$((success_count + 1))
            else
                fail_count=$((fail_count + 1))
                echo "客户端进程PID:$pid 执行失败（退出码:$exit_code）" | tee -a $LOG_FILE
            fi
        else
            fail_count=$((fail_count + 1))
            echo "客户端进程PID:$pid 未运行" | tee -a $LOG_FILE
        fi
    done < $PID_FILE

    end_time=$(date +%s)
    elapsed_time=$((end_time - start_time))
    echo -e "\n========== 压测结果汇总（并发数:${concurrent}）==========" | tee -a $LOG_FILE
    echo "总耗时: ${elapsed_time} 秒" | tee -a $LOG_FILE
    echo "成功执行的客户端数: $success_count" | tee -a $LOG_FILE
    echo "失败执行的客户端数: $fail_count" | tee -a $LOG_FILE
    echo "客户端成功率: $(echo "scale=2; $success_count/$concurrent*100" | bc)%" | tee -a $LOG_FILE
}

# 函数：测试服务器极限并发（逐步递增并发数）
test_max_concurrent() {
    echo -e "\n========== 开始测试服务器极限并发 ==========" | tee -a $LOG_FILE
    echo "测试范围：从 ${BASE_CONCURRENT} 开始，每次递增 ${STEP_CONCURRENT}，最大 ${MAX_CONCURRENT}" | tee -a $LOG_FILE

    for ((concurrent=BASE_CONCURRENT; concurrent<=MAX_CONCURRENT; concurrent+=STEP_CONCURRENT)); do
        success_count=0
        fail_count=0
        # 启动当前并发数的客户端
        start_clients $concurrent
        # 等待执行完成并统计
        wait_clients $concurrent
        # 检查是否达到极限（成功率<95%则停止）
        success_rate=$(echo "scale=2; $success_count/$concurrent*100" | bc)
        if (( $(echo "$success_rate < 95" | bc -l) )); then
            echo -e "\n========== 服务器极限并发已找到：${concurrent} 个客户端（成功率${success_rate}% < 95%）==========" | tee -a $LOG_FILE
            break
        fi
        # 清理PID文件，准备下一轮测试
        rm -f $PID_FILE
        # 间隔5秒，让服务器恢复
        echo -e "\n等待5秒，服务器恢复中...\n" | tee -a $LOG_FILE
        sleep 5
    done
}

# 函数：快速启动指定并发数（原脚本的30个客户端）
quick_start() {
    success_count=0
    fail_count=0
    start_clients $BASE_CONCURRENT
    wait_clients $BASE_CONCURRENT
}

# 主流程
echo "===================== Reactor服务器压测脚本 ====================="
echo "目标服务器: ${SERVER_IP}:${SERVER_PORT}"
echo "1. 快速启动基础并发（${BASE_CONCURRENT}个客户端）"
echo "2. 测试极限并发（从${BASE_CONCURRENT}到${MAX_CONCURRENT}，步长${STEP_CONCURRENT}）"
read -p "请选择测试模式（1/2）：" mode

check_client  # 先检查client程序

case $mode in
    1)
        quick_start
        ;;
    2)
        test_max_concurrent
        ;;
    *)
        echo "无效选择！默认执行快速启动模式"
        quick_start
        ;;
esac

echo -e "\n压测完成！详细日志：$LOG_FILE"
