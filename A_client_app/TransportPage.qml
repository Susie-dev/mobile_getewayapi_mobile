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

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // 顶部导航栏
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 60
            color: "#1a237e"

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                
                Button {
                    text: "< 返回"
                    onClicked: {
                        simulator.stopSimulation();
                        stackView.pop();
                    }
                }

                Text {
                    text: "运输监控"
                    color: "white"
                    font.pixelSize: 20
                    font.bold: true
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

            Text {
                text: "当前订单: " + currentOrderNo
                font.pixelSize: 18
                font.bold: true
                Layout.alignment: Qt.AlignHCenter
            }

            Text {
                text: "网络状态: " + simulator.connectionStatus
                font.pixelSize: 14
                color: simulator.connectionStatus === "已连接到服务器" ? "green" : (simulator.connectionStatus.indexOf("错误") !== -1 ? "red" : "gray")
                Layout.alignment: Qt.AlignHCenter
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "black"
                radius: 8

                ScrollView {
                    anchors.fill: parent
                    anchors.margins: 10
                    
                    Text {
                        text: simulator.currentJson === "" ? "等待生成数据..." : simulator.currentJson
                        color: "#00ff00"
                        font.family: "Courier"
                        wrapMode: Text.WrapAnywhere
                    }
                }
            }

            Button {
                text: simulator.isRunning ? "停止运输 (送达)" : "开始运输 (实时上报)"
                Layout.fillWidth: true
                Layout.preferredHeight: 50
                font.pixelSize: 18
                
                background: Rectangle {
                    color: simulator.isRunning ? "#f44336" : "#4caf50"
                    radius: 5
                }
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.bold: true
                }
                
                onClicked: {
                    if (simulator.isRunning) {
                        simulator.stopSimulation();
                        // 实际业务中这里可以调用 orderModel.updateOrderStatus() 设为 DELIVERED
                        stackView.pop();
                    } else {
                        simulator.startSimulation(currentOrderNo, targetTemp);
                    }
                }
            }
        }
    }
}
