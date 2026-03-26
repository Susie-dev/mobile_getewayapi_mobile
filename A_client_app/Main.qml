import QtQuick
import QtQuick.Controls

Window {
    width: 400
    height: 700
    visible: true
    title: qsTr("冷链溯源 - 司机端 (Client)")

    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: "LoginPage.qml"
    }
}
