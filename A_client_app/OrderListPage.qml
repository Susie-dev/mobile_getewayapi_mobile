import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ClientApp 1.0

Item {
    id: root

    OrderModel {
        id: orderModel
    }

    Component.onCompleted: {
        orderModel.loadOrders();
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // 顶部导航栏
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 60
            color: "#1a237e"

            Text {
                anchors.centerIn: parent
                text: "任务大厅"
                color: "white"
                font.pixelSize: 20
                font.bold: true
            }
        }

        // 订单列表
        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: orderModel
            spacing: 10
            clip: true
            
            // 添加一点内边距
            topMargin: 10
            bottomMargin: 10

            delegate: Rectangle {
                width: ListView.view.width - 20
                height: 140
                anchors.horizontalCenter: parent.horizontalCenter
                color: "white"
                radius: 8
                border.color: "#e0e0e0"

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 15
                    spacing: 5

                    RowLayout {
                        Layout.fillWidth: true
                        Text { 
                            text: model.orderNo 
                            font.bold: true 
                            color: "#555"
                            Layout.fillWidth: true
                        }
                        Text { 
                            text: model.status === "PENDING" ? "待接单" : (model.status === "IN_TRANSIT" ? "运输中" : "已完成")
                            color: model.status === "PENDING" ? "#ff9800" : "#4caf50"
                            font.bold: true
                        }
                    }

                    Text { text: "货物: " + model.goodsName; color: "#333" }
                    Text { text: "从: " + model.origin; color: "#666"; font.pixelSize: 12 }
                    Text { text: "到: " + model.destination; color: "#666"; font.pixelSize: 12 }
                    Text { text: "要求温控: " + model.targetTemp + "℃"; color: "#1976d2"; font.bold: true }

                    Button {
                        Layout.alignment: Qt.AlignRight
                        text: model.status === "PENDING" ? "接单并装车" : "查看运输"
                        
                        onClicked: {
                            if (model.status === "PENDING") {
                                orderModel.updateOrderStatus(model.orderNo, "IN_TRANSIT");
                            }
                            // 跳转到运输监控页，并传递订单号和目标温度
                            stackView.push("TransportPage.qml", { 
                                "currentOrderNo": model.orderNo,
                                "targetTemp": model.targetTemp
                            });
                        }
                    }
                }
            }
        }
    }
}
