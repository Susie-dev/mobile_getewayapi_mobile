#!/bin/bash
set -eo pipefail

# 配置项
SERVER_IP="192.168.211.134"
SERVER_PORT=5007
START_CONN=1000
STEP_CONN=1000
MAX_CONN=50000
PID_FILE="./conn_pids.txt"
LOG_FILE="./conn_test.log"
NC_CMD="ncat"
DELAY_PER_CONN=0.01

# 前置检查
pre_check() {
    for cmd in $NC_CMD ss bc grep; do
        command -v $cmd &> /dev/null || { echo "请安装$cmd"; exit 1; }
    done
    rm -f $PID_FILE $LOG_FILE
    echo "【前置检查】依赖已就绪" | tee -a $LOG_FILE
}

# 建立长连接（简化，确保不退出）
create_conn() {
    local conn_id=$1
    # 用ncat后台挂起，直接sleep维持连接（避免-e参数兼容性问题）
    nohup $NC_CMD $SERVER_IP $SERVER_PORT > /dev/null 2>&1 &
    echo $! >> $PID_FILE
    [ $((conn_id%1000)) -eq 0 ] && echo "已发起${conn_id}个连接" | tee -a $LOG_FILE
    sleep $DELAY_PER_CONN
}

# 统计存活连接（修复grep选项问题）
count_alive_conn() {
    # 用引号包裹匹配串，避免“-”被识别为选项
    alive_conn=$(ss -ant | grep " ${SERVER_IP}:${SERVER_PORT} " | wc -l)
    echo ${alive_conn:-0}
}

# 主测试
main_test() {
    for ((conn=START_CONN; conn<=MAX_CONN; conn+=STEP_CONN)); do
        echo -e "\n【测试】尝试建立${conn}个连接" | tee -a $LOG_FILE
        for ((i=1; i<=STEP_CONN; i++)); do
            create_conn $((conn-STEP_CONN+i))
        done
        sleep 5

        alive_conn=$(count_alive_conn)
        success_rate=$(echo "scale=2; $alive_conn/$conn*100" | bc -l)
        echo "【结果】存活数:${alive_conn} | 成功率:${success_rate}%" | tee -a $LOG_FILE

        [ $(echo "$success_rate<95" | bc -l) -eq 1 ] && { echo "最大连接数:$((conn-STEP_CONN))"; break; }
        sleep 30
    done
}

# 清理
cleanup() {
    pkill -f "$NC_CMD $SERVER_IP $SERVER_PORT"
    rm -f $PID_FILE
    echo "【清理】完成" | tee -a $LOG_FILE
}

pre_check
main_test
cleanup
echo "日志: $LOG_FILE"