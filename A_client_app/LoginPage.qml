import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 30
        width: parent.width * 0.8

        Text {
            text: "司机接单系统"
            font.pixelSize: 28
            font.bold: true
            Layout.alignment: Qt.AlignHCenter
            color: "#333"
        }

        TextField {
            id: phoneInput
            Layout.fillWidth: true
            placeholderText: "请输入手机号 (测试: 13800000000)"
            font.pixelSize: 16
        }

        TextField {
            id: pwdInput
            Layout.fillWidth: true
            placeholderText: "请输入密码"
            echoMode: TextInput.Password
            font.pixelSize: 16
        }

        Button {
            text: "登 录"
            Layout.fillWidth: true
            Layout.preferredHeight: 50
            font.pixelSize: 18
            
            background: Rectangle {
                color: "#1a237e"
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
                // 模拟登录成功，跳转到任务列表
                stackView.push("OrderListPage.qml")
            }
        }
    }
}
