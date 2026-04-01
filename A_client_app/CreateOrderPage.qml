import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ClientApp 1.0

Item {
    id: root

    ColumnLayout {
        anchors.centerIn: parent
        width: parent.width * 0.8
        spacing: 30

        Text {
            text: "创建新订单"
            font.pixelSize: 28
            font.bold: true
            Layout.alignment: Qt.AlignHCenter
            color: "#1a237e"
        }

        TextField {
            id: goodsInput
            Layout.fillWidth: true
            placeholderText: "请输入货物名称 (如: 苹果)"
            font.pixelSize: 18
        }

        TextField {
            id: tempInput
            Layout.fillWidth: true
            placeholderText: "目标温度要求 (如: 4.0)"
            font.pixelSize: 18
            validator: DoubleValidator { bottom: -30.0; top: 30.0 }
        }

        Button {
            text: "生成订单并开始运输"
            Layout.fillWidth: true
            Layout.preferredHeight: 50
            font.pixelSize: 18
            
            background: Rectangle {
                color: "#4caf50"
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
                if (goodsInput.text === "") {
                    return;
                }
                
                // 1. 随机生成唯一的订单号: ORD_时间戳_随机数
                let timestamp = new Date().getTime();
                let randomNum = Math.floor(Math.random() * 1000);
                let newOrderNo = "ORD_" + timestamp + "_" + randomNum;
                
                let targetTemp = parseFloat(tempInput.text);
                if (isNaN(targetTemp)) {
                    targetTemp = 4.0; // 默认 4.0 度
                }

                // 2. 将数据存入本地 SQLite (未来可以加入货主信息等)
                // (此处为了演示流畅，我们直接把参数传给下一个页面)

                // 3. 跳转到运输监控页面
                stackView.push("TransportPage.qml", { 
                    "currentOrderNo": newOrderNo,
                    "goodsName": goodsInput.text,
                    "targetTemp": targetTemp
                });
            }
        }
    }
}
