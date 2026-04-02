import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    // 背景渐变
    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#e3f2fd" }
            GradientStop { position: 1.0; color: "#bbdefb" }
        }
    }

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 40
        width: parent.width * 0.85

        // Logo 区域
        Rectangle {
            width: 80
            height: 80
            radius: 40
            color: "#1565c0"
            Layout.alignment: Qt.AlignHCenter
            
            Text {
                anchors.centerIn: parent
                text: "🚚"
                font.pixelSize: 40
            }
        }

        Text {
            text: "冷链司机接单系统"
            font.pixelSize: 28
            font.bold: true
            font.family: "Microsoft YaHei"
            Layout.alignment: Qt.AlignHCenter
            color: "#0d47a1"
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 20

            TextField {
                id: phoneInput
                Layout.fillWidth: true
                Layout.preferredHeight: 50
                placeholderText: "请输入手机号"
                font.pixelSize: 16
                background: Rectangle {
                    color: "white"
                    radius: 8
                    border.color: phoneInput.activeFocus ? "#1976d2" : "#e0e0e0"
                    border.width: phoneInput.activeFocus ? 2 : 1
                }
                leftPadding: 15
            }

            TextField {
                id: pwdInput
                Layout.fillWidth: true
                Layout.preferredHeight: 50
                placeholderText: "请输入密码"
                echoMode: TextInput.Password
                font.pixelSize: 16
                background: Rectangle {
                    color: "white"
                    radius: 8
                    border.color: pwdInput.activeFocus ? "#1976d2" : "#e0e0e0"
                    border.width: pwdInput.activeFocus ? 2 : 1
                }
                leftPadding: 15
            }
        }

        Button {
            text: "登 录"
            Layout.fillWidth: true
            Layout.preferredHeight: 50
            Layout.topMargin: 20
            
            background: Rectangle {
                color: parent.pressed ? "#0d47a1" : "#1976d2"
                radius: 25
                
                // 阴影效果
                layer.enabled: true
            }
            contentItem: Text {
                text: parent.text
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.bold: true
                font.pixelSize: 18
                font.letterSpacing: 2
            }

            onClicked: {
                stackView.push("CreateOrderPage.qml")
            }
        }
        
        Text {
            text: "遇到问题？联系调度中心"
            color: "#1976d2"
            font.pixelSize: 14
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 10
            
            MouseArea {
                anchors.fill: parent
                onClicked: console.log("Call dispatch")
            }
        }
    }
}
