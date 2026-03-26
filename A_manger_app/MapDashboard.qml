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

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // 左侧面板
        Rectangle {
            Layout.preferredWidth: 300
            Layout.fillHeight: true
            color: "#f5f5f5"
            border.color: "#e0e0e0"
            border.width: 1
            z: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 15
                spacing: 10

                Text {
                    text: "实时警报信息"
                    font.pixelSize: 18
                    font.bold: true
                }

                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    
                    TextArea {
                        id: alertLogArea
                        readOnly: true
                        text: "暂无警报..."
                        wrapMode: Text.WrapAnywhere
                    }
                }
            }
        }

        // 右侧地图
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            
            WebEngineView {
                id: mapView
                anchors.fill: parent
                url: "qrc:/qt/qml/ManagerApp/map.html"
            }
        }
    }
}
