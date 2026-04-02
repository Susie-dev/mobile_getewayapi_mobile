import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ManagerApp 1.0

Window {
    width: 1280
    height: 800
    visible: true
    title: qsTr("冷链溯源 - 全局监控调度大屏")
    color: "#0a192f" // 整体暗色背景，适合大屏

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
        height: 64
        color: "#112240"
        z: 2
        
        // 底部高亮边框
        Rectangle {
            width: parent.width
            height: 2
            color: "#64ffda"
            anchors.bottom: parent.bottom
        }
        
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 25
            anchors.rightMargin: 25

            RowLayout {
                spacing: 10
                Text {
                    text: "🌍"
                    font.pixelSize: 28
                }
                Text {
                    text: "冷链运输实时监控中心"
                    color: "#ccd6f6"
                    font.pixelSize: 24
                    font.bold: true
                    font.family: "Microsoft YaHei"
                    font.letterSpacing: 2
                }
            }
            
            Item { Layout.fillWidth: true } // 弹簧占位

            TabBar {
                id: tabBar
                Layout.preferredWidth: 360
                Layout.preferredHeight: 64
                background: Rectangle { color: "transparent" }
                
                TabButton {
                    text: "实时地图监控"
                    contentItem: Text {
                        text: parent.text
                        color: parent.checked ? "#64ffda" : "#8892b0"
                        font.bold: parent.checked
                        font.pixelSize: 16
                        font.family: "Microsoft YaHei"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        color: "transparent"
                        Rectangle {
                            width: parent.width * 0.6
                            height: 3
                            color: "#64ffda"
                            anchors.bottom: parent.bottom
                            anchors.horizontalCenter: parent.horizontalCenter
                            visible: parent.parent.checked
                        }
                    }
                }
                TabButton {
                    text: "全网订单大盘"
                    contentItem: Text {
                        text: parent.text
                        color: parent.checked ? "#64ffda" : "#8892b0"
                        font.bold: parent.checked
                        font.pixelSize: 16
                        font.family: "Microsoft YaHei"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        color: "transparent"
                        Rectangle {
                            width: parent.width * 0.6
                            height: 3
                            color: "#64ffda"
                            anchors.bottom: parent.bottom
                            anchors.horizontalCenter: parent.horizontalCenter
                            visible: parent.parent.checked
                        }
                    }
                }
            }

            Item { Layout.fillWidth: true } // 弹簧占位

            RowLayout {
                spacing: 15
                
                Rectangle {
                    width: 10
                    height: 10
                    radius: 5
                    color: network.connectionStatus === "已连接到服务器" ? "#00e676" : "#ff1744"
                    
                    // 呼吸灯效果
                    SequentialAnimation on opacity {
                        loops: Animation.Infinite
                        PropertyAnimation { to: 0.3; duration: 1000 }
                        PropertyAnimation { to: 1.0; duration: 1000 }
                        running: network.connectionStatus === "已连接到服务器"
                    }
                }
                
                Text {
                    text: "SERVER: " + (network.connectionStatus === "已连接到服务器" ? "ONLINE" : "OFFLINE")
                    color: network.connectionStatus === "已连接到服务器" ? "#00e676" : "#ff1744"
                    font.pixelSize: 14
                    font.family: "Courier"
                    font.bold: true
                }
                
                Button {
                    text: network.connectionStatus === "已连接到服务器" ? "断开" : "重连"
                    Layout.preferredHeight: 32
                    background: Rectangle {
                        color: "transparent"
                        border.color: network.connectionStatus === "已连接到服务器" ? "#8892b0" : "#64ffda"
                        border.width: 1
                        radius: 4
                    }
                    contentItem: Text {
                        text: parent.text
                        color: network.connectionStatus === "已连接到服务器" ? "#8892b0" : "#64ffda"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pixelSize: 12
                    }
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
