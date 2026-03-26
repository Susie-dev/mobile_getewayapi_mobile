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
        anchors.margins: 20
        spacing: 15

        Text {
            text: "全网订单数据大盘"
            font.pixelSize: 24
            font.bold: true
            color: "#333"
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "white"
            radius: 8
            border.color: "#e0e0e0"

            ListView {
                anchors.fill: parent
                anchors.margins: 10
                model: orderModel
                spacing: 10
                clip: true

                delegate: Rectangle {
                    width: ListView.view.width
                    height: 80
                    color: "#f8f9fa"
                    radius: 5
                    border.color: "#e0e0e0"

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 15
                        spacing: 20

                        ColumnLayout {
                            Layout.fillWidth: true
                            Text { text: "订单号: " + model.orderNo; font.bold: true; font.pixelSize: 16 }
                            Text { text: "货物: " + model.goods; color: "#555" }
                        }

                        ColumnLayout {
                            Layout.alignment: Qt.AlignRight
                            Text { 
                                text: model.status 
                                color: model.status === "运输中" ? "#4caf50" : "#ff9800"
                                font.bold: true
                                font.pixelSize: 16
                            }
                            Text { text: "实时温度: " + model.temp; color: "#1976d2" }
                        }
                    }
                }
            }
        }
    }
}
