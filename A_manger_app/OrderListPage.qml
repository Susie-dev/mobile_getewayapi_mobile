import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    // 模拟的订单数据模型
    ListModel {
        id: orderModel
        ListElement { orderNo: "ORD_20260326_001"; goods: "山东烟台红富士苹果 5吨"; status: "运输中"; temp: "4.2℃ (正常)" }
        ListElement { orderNo: "ORD_20260326_002"; goods: "海南章姬草莓 2吨"; status: "待接单"; temp: "--" }
        ListElement { orderNo: "ORD_20260326_003"; goods: "进口智利车厘子 1吨"; status: "待接单"; temp: "--" }
    }

    // 暴露一个方法供外部更新订单状态
    function updateOrderData(jsonString) {
        try {
            let data = JSON.parse(jsonString);
            let orderId = data.order_id;
            let temp = data.cargo_status.current_temp;
            let isAlert = data.cargo_status.is_alert;
            
            // 在模型中查找并更新
            for (let i = 0; i < orderModel.count; i++) {
                if (orderModel.get(i).orderNo === orderId) {
                    orderModel.setProperty(i, "status", "运输中");
                    let tempStr = temp + "℃ " + (isAlert ? "(异常!)" : "(正常)");
                    orderModel.setProperty(i, "temp", tempStr);
                    break;
                }
            }
        } catch(e) {}
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 30
        spacing: 20

        RowLayout {
            spacing: 15
            Text {
                text: "📋"
                font.pixelSize: 28
            }
            Text {
                text: "全网订单数据大盘"
                font.pixelSize: 28
                font.bold: true
                font.family: "Microsoft YaHei"
                color: "#ccd6f6"
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#112240"
            radius: 12
            border.color: "#233554"
            border.width: 1
            
            layer.enabled: true

            ListView {
                anchors.fill: parent
                anchors.margins: 20
                model: orderModel
                spacing: 15
                clip: true

                delegate: Rectangle {
                    width: ListView.view.width
                    height: 90
                    color: "#0a192f"
                    radius: 8
                    border.color: "#233554"
                    border.width: 1
                    
                    // 鼠标悬停高亮
                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        onEntered: parent.border.color = "#64ffda"
                        onExited: parent.border.color = "#233554"
                        propagateComposedEvents: true // 允许底层按钮接收点击
                    }

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 20
                        spacing: 20

                        Rectangle {
                            width: 50
                            height: 50
                            radius: 25
                            color: "#1d2d50"
                            Text {
                                anchors.centerIn: parent
                                text: "📦"
                                font.pixelSize: 24
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 5
                            Text { 
                                text: model.orderNo
                                font.bold: true
                                font.pixelSize: 18
                                font.family: "Courier"
                                color: "#ccd6f6"
                            }
                            Text { 
                                text: model.goods
                                color: "#8892b0"
                                font.pixelSize: 14
                            }
                        }

                        ColumnLayout {
                            Layout.alignment: Qt.AlignRight
                            spacing: 5
                            
                            RowLayout {
                                Layout.alignment: Qt.AlignRight
                                Rectangle {
                                    width: 8
                                    height: 8
                                    radius: 4
                                    color: model.status === "运输中" ? "#00e676" : "#ffb300"
                                }
                                Text { 
                                    text: model.status 
                                    color: model.status === "运输中" ? "#00e676" : "#ffb300"
                                    font.bold: true
                                    font.pixelSize: 16
                                }
                            }
                            
                            Text { 
                                text: "实时温度: " + model.temp
                                color: model.temp.indexOf("异常") !== -1 ? "#ff1744" : "#64ffda"
                                font.pixelSize: 14
                                font.family: "Courier"
                                Layout.alignment: Qt.AlignRight
                            }
                        }
                        
                        Button {
                            text: "▶ 回放轨迹"
                            Layout.leftMargin: 20
                            background: Rectangle {
                                color: parent.pressed ? "#112240" : "transparent"
                                border.color: "#64ffda"
                                border.width: 1
                                radius: 4
                            }
                            contentItem: Text {
                                text: parent.text
                                color: "#64ffda"
                                font.pixelSize: 14
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            onClicked: {
                                network.requestOrderHistory(model.orderNo);
                            }
                        }
                    }
                }
            }
        }
    }
}
