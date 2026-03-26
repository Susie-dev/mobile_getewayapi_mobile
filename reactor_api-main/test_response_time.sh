#!/bin/bash
set -eo pipefail

# 配置项
SERVER_IP="192.168.211.134"
SERVER_PORT=5007
CLIENT_BIN="./client"
TEST_TIMES=1000  # 客户端要发送的报文总数

# 前置检查
[ ! -x "$CLIENT_BIN" ] && { echo "错误：$CLIENT_BIN 无执行权限"; exit 1; }

# 关键：启动1次客户端，直接批量发送TEST_TIMES次报文，统计总耗时→算平均响应时间
echo "【开始测试】客户端单次启动，批量发送${TEST_TIMES}次报文"
start_total=$(date +%s%N)
# 假设客户端支持传“发送次数”参数（如./client IP PORT 次数），若不支持则手动改客户端
$CLIENT_BIN $SERVER_IP $SERVER_PORT $TEST_TIMES
end_total=$(date +%s%N)

# 计算总耗时（毫秒）和平均响应时间
total_ms=$(echo "scale=3; ($end_total - $start_total)/1000000" | bc -l)
avg_resp_ms=$(echo "scale=3; $total_ms / $TEST_TIMES" | bc -l)

echo -e "\n【测试结果】"
echo "总发送次数：${TEST_TIMES}次"
echo "总耗时：${total_ms}ms"
echo "平均响应时间：${avg_resp_ms}ms"