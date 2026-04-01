import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ManagerApp 1.0

Window {
    width: 1280
    height: 800
    visible: true
    title: qsTr("冷链溯源 - 全局监控调度大屏")

    ManagerNetwork {
        id: network
        onTruckDataReceived: function(jsonString) {
            // 将实时数据传递给地图大屏
            mapDashboard.updateTruckOnMap(jsonString);
            // 将实时数据传递给订单大盘更新状态
            orderListPage.updateOrderData(jsonString);
        }
        onHistoryDataReceived: function(jsonString) {
            // 收到历史数据，强制切换到地图页并播放轨迹
            tabBar.currentIndex = 0;
            mapDashboard.playHistoryOnMap(jsonString);
        }
    }

    Component.onCompleted: {
        network.connectToServer();
    }

    // 顶部状态栏
    Rectangle {
        id: header
        width: parent.width
        height: 60
        color: "#1a237e"
        z: 2
        
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 20
            anchors.rightMargin: 20

            Text {
                text: "冷链运输实时监控中心"
                color: "white"
                font.pixelSize: 24
                font.bold: true
            }
            
            Item { Layout.fillWidth: true } // 弹簧占位

            TabBar {
                id: tabBar
                Layout.preferredWidth: 300
                background: Rectangle { color: "transparent" }
                
                TabButton {
                    text: "实时地图监控"
                    contentItem: Text {
                        text: parent.text
                        color: parent.checked ? "white" : "#b0bec5"
                        font.bold: parent.checked
                        font.pixelSize: 16
                        horizontalAlignment: Text.AlignHCenter
                    }
                    background: Rectangle {
                        color: parent.checked ? "rgba(255,255,255,0.2)" : "transparent"
                        radius: 4
                    }
                }
                TabButton {
                    text: "全网订单大盘"
                    contentItem: Text {
                        text: parent.text
                        color: parent.checked ? "white" : "#b0bec5"
                        font.bold: parent.checked
                        font.pixelSize: 16
                        horizontalAlignment: Text.AlignHCenter
                    }
                    background: Rectangle {
                        color: parent.checked ? "rgba(255,255,255,0.2)" : "transparent"
                        radius: 4
                    }
                }
            }

            Item { Layout.fillWidth: true } // 弹簧占位

            RowLayout {
                spacing: 15
                Text {
                    text: "后端连接状态: " + network.connectionStatus
                    color: network.connectionStatus === "已连接到服务器" ? "#00ff00" : "#ffcccc"
                    font.pixelSize: 16
                }
                Button {
                    text: network.connectionStatus === "已连接到服务器" ? "断开" : "重连"
                    onClicked: {
                        if (network.connectionStatus === "已连接到服务器") {
                            network.disconnectFromServer();
                        } else {
                            network.connectToServer();
                        }
                    }
                }
            }
        }
    }

    // 多页面切换容器
    StackLayout {
        anchors.top: header.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        currentIndex: tabBar.currentIndex

        MapDashboard {
            id: mapDashboard
        }

        OrderListPage {
            id: orderListPage
        }
    }
}
