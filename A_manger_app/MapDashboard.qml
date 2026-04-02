import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtWebEngine

Item {
    id: root
    
    // 提供给外部调用的接口
    function updateTruckOnMap(jsonString) {
        let safeJson = jsonString.replace(/'/g, "\\'");
        let script = "updateTruckData('" + safeJson + "');";
        mapView.runJavaScript(script);
        
        if (jsonString.indexOf('"is_alert":true') !== -1 || jsonString.indexOf('"is_alert": true') !== -1) {
            let data = JSON.parse(jsonString);
            let timeStr = new Date().toLocaleTimeString();
            alertLogArea.text = `[${timeStr}] ⚠️ 警告: 订单 ${data.order_id} 温度异常 (${data.cargo_status.current_temp}℃)!\n` + alertLogArea.text;
        }
    }

    // 播放历史轨迹
    function playHistoryOnMap(jsonString) {
        let safeJson = jsonString.replace(/'/g, "\\'");
        let script = "playHistory('" + safeJson + "');";
        mapView.runJavaScript(script);
        
        let data = JSON.parse(jsonString);
        alertLogArea.text = `[系统] 正在回放订单 ${data.order_id} 的历史轨迹，共包含 ${data.data.length} 个节点。\n` + alertLogArea.text;
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // 左侧面板 (科技感大屏风格)
        Rectangle {
            Layout.preferredWidth: 320
            Layout.fillHeight: true
            color: "#112240"
            
            // 右侧发光边框
            Rectangle {
                width: 1
                height: parent.height
                color: "#64ffda"
                opacity: 0.3
                anchors.right: parent.right
            }
            z: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 15

                RowLayout {
                    spacing: 8
                    Text {
                        text: "🚨"
                        font.pixelSize: 20
                    }
                    Text {
                        text: "实时警报面板"
                        font.pixelSize: 18
                        font.bold: true
                        font.family: "Microsoft YaHei"
                        color: "#ccd6f6"
                    }
                }
                
                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: "#233554"
                }

                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    
                    TextArea {
                        id: alertLogArea
                        readOnly: true
                        text: "暂无警报，全网运行平稳..."
                        color: "#64ffda"
                        font.family: "Courier"
                        font.pixelSize: 13
                        wrapMode: Text.WrapAnywhere
                        background: Rectangle { color: "transparent" }
                    }
                }
            }
        }

        // 右侧地图
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#0a192f"
            
            WebEngineView {
                id: mapView
                anchors.fill: parent
                anchors.margins: 10 // 留出一点边距，有大屏模块感
                url: "qrc:/qt/qml/ManagerApp/map.html"
            }
        }
    }
}
