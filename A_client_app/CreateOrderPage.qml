import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ClientApp 1.0

Item {
    id: root

    // 顶部背景
    Rectangle {
        id: headerBg
        width: parent.width
        height: 180
        color: "#1976d2"
        radius: 0
        
        // 底部圆角效果
        Rectangle {
            width: parent.width
            height: 30
            color: parent.color
            anchors.bottom: parent.bottom
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        Text {
            text: "创建运输订单"
            font.pixelSize: 32
            font.bold: true
            font.family: "Microsoft YaHei"
            color: "white"
            Layout.topMargin: 30
            Layout.bottomMargin: 40
        }

        // 表单卡片
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 280
            radius: 12
            color: "white"
            
            // 简单阴影
            layer.enabled: true

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 25

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 8
                    
                    Text {
                        text: "货物信息"
                        font.pixelSize: 14
                        color: "#666"
                        font.bold: true
                    }
                    
                    TextField {
                        id: goodsInput
                        Layout.fillWidth: true
                        Layout.preferredHeight: 50
                        placeholderText: "请输入货物名称 (如: 烟台苹果)"
                        font.pixelSize: 16
                        background: Rectangle {
                            color: "#f5f5f5"
                            radius: 8
                            border.width: 0
                        }
                        leftPadding: 15
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 8
                    
                    Text {
                        text: "目标温控要求 (℃)"
                        font.pixelSize: 14
                        color: "#666"
                        font.bold: true
                    }
                    
                    TextField {
                        id: tempInput
                        Layout.fillWidth: true
                        Layout.preferredHeight: 50
                        placeholderText: "例如: 4.0"
                        font.pixelSize: 16
                        validator: DoubleValidator { bottom: -30.0; top: 30.0 }
                        background: Rectangle {
                            color: "#f5f5f5"
                            radius: 8
                            border.width: 0
                        }
                        leftPadding: 15
                    }
                }
            }
        }

        Item {
            Layout.fillHeight: true // 占位把按钮推到底部
        }

        Button {
            text: "生成订单并开始运输"
            Layout.fillWidth: true
            Layout.preferredHeight: 55
            Layout.bottomMargin: 20
            
            background: Rectangle {
                color: parent.pressed ? "#388e3c" : "#4caf50"
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
                if (goodsInput.text === "") {
                    // 简单提示
                    goodsInput.placeholderText = "名称不能为空！"
                    goodsInput.placeholderTextColor = "red"
                    return;
                }
                
                let timestamp = new Date().getTime();
                let randomNum = Math.floor(Math.random() * 1000);
                let newOrderNo = "ORD_" + timestamp + "_" + randomNum;
                
                let targetTemp = parseFloat(tempInput.text);
                if (isNaN(targetTemp)) {
                    targetTemp = 4.0;
                }

                stackView.push("TransportPage.qml", { 
                    "currentOrderNo": newOrderNo,
                    "goodsName": goodsInput.text,
                    "targetTemp": targetTemp
                });
            }
        }
    }
}
