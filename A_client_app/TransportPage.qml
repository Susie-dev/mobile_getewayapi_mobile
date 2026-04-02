import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ClientApp 1.0

Item {
    id: root

    // 接收从列表页传来的参数
    property string currentOrderNo: "UNKNOWN"
    property real targetTemp: 4.0

    DataSimulator {
        id: simulator
    }

    // 整体背景
    Rectangle {
        anchors.fill: parent
        color: "#f0f2f5"
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // 顶部导航栏
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 70
            color: "#1976d2"
            layer.enabled: true

            RowLayout {
                anchors.fill: parent
                anchors.margins: 15
                
                Button {
                    text: "❮ 返回"
                    font.pixelSize: 16
                    background: Rectangle { color: "transparent" }
                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        font.bold: true
                    }
                    onClicked: {
                        simulator.stopSimulation();
                        stackView.pop();
                    }
                }

                Text {
                    text: "实时运输监控"
                    color: "white"
                    font.pixelSize: 22
                    font.bold: true
                    font.family: "Microsoft YaHei"
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                }
                
                Item { width: 50 } // 占位保持居中
            }
        }

        // 主内容区
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 20
            spacing: 20

            // 状态卡片
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 100
                radius: 12
                color: "white"
                layer.enabled: true

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 15
                    spacing: 10

                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            text: "当前订单:"
                            font.pixelSize: 14
                            color: "#666"
                        }
                        Text {
                            text: currentOrderNo
                            font.pixelSize: 16
                            font.bold: true
                            color: "#333"
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignRight
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        height: 1
                        color: "#eee"
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            text: "网络状态:"
                            font.pixelSize: 14
                            color: "#666"
                        }
                        Text {
                            text: simulator.connectionStatus
                            font.pixelSize: 14
                            font.bold: true
                            color: simulator.connectionStatus === "已连接到服务器" ? "#4caf50" : 
                                   (simulator.connectionStatus.indexOf("错误") !== -1 ? "#f44336" : "#ff9800")
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignRight
                        }
                    }
                }
            }

            // 日志控制台
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "#263238"
                radius: 12
                layer.enabled: true

                // 控制台标题栏
                Rectangle {
                    width: parent.width
                    height: 35
                    color: "#37474f"
                    radius: 12
                    Rectangle {
                        width: parent.width
                        height: 10
                        color: parent.color
                        anchors.bottom: parent.bottom
                    }
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 15
                        text: "Terminal - 实时上报数据"
                        color: "#b0bec5"
                        font.pixelSize: 12
                        font.family: "Courier"
                    }
                }

                ScrollView {
                    anchors.fill: parent
                    anchors.topMargin: 45
                    anchors.margins: 15
                    
                    Text {
                        text: simulator.currentJson === "" ? "等待生成数据...\n> 提示：如果没连上网，数据将暂存在本地缓存中。" : simulator.currentJson
                        color: "#69f0ae"
                        font.family: "Courier"
                        font.pixelSize: 13
                        wrapMode: Text.WrapAnywhere
                    }
                }
            }

            Button {
                text: simulator.isRunning ? "到达目的地 (结束运输)" : "开始启程 (启动上报)"
                Layout.fillWidth: true
                Layout.preferredHeight: 55
                Layout.topMargin: 10
                
                background: Rectangle {
                    color: simulator.isRunning ? "#e53935" : "#1e88e5"
                    radius: 8
                    layer.enabled: true
                }
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.bold: true
                    font.pixelSize: 18
                    font.letterSpacing: 1
                }
                
                onClicked: {
                    if (simulator.isRunning) {
                        simulator.stopSimulation();
                        stackView.pop();
                    } else {
                        simulator.startSimulation(currentOrderNo, targetTemp);
                    }
                }
            }
        }
    }
}
