import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Basic
import Qt5Compat.GraphicalEffects
import QtCore

Item {
    id: mqttInterface

    ColumnLayout {
        id: mainLayout
        width: parent.width
        height: parent.height
        anchors.top: parent.top
        anchors.topMargin: 14
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
        spacing: 10

        //history input
        Settings{
            id: settings
            property string publsihTopic: ""
            property string publishMessage: ""
        }

        // 订阅消息区（优化版）
        GroupBox {
            id: receiveGroup
            //title: "Message Area"
            font.pixelSize: 18
            font.bold: true
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: parent.height * 0.6
            clip: true

            background: Rectangle {
                color: "transparent"
                radius: 6
                border.width: 1
                border.color: "#e0e0e0"
                DropShadow {
                    anchors.fill: parent
                    radius: 4
                    samples: 6
                    color: "#10000000"
                    opacity: 0.2
                    source: parent
                }
            }

            ColumnLayout {
                anchors.fill: parent
                spacing: 4
                //padding: 8

                // 工具栏：按钮 + checkBox
                RowLayout {
                    id: toolBar
                    width: parent.width
                    spacing: 6
                    Layout.preferredHeight: 12

                    Button {
                        id: clearReceiveBtn
                        text: "Clear"
                        width: 24; height: 28
                        font.pixelSize: 14
                        background: Rectangle {
                            radius: 4
                            color: clearReceiveBtn.hovered ? "#e8f0fe" : "transparent"
                            border.color: "#ddd"; border.width: 1
                        }
                        onClicked: receiveModel.clear()
                    }

                    Button {
                        id: saveReceiveBtn
                        text: "Save"
                        width: 50; height: 28
                        font.pixelSize: 14
                        background: Rectangle {
                            radius: 4
                            color: saveReceiveBtn.hovered ? "#e8f0fe" : "transparent"
                            border.color: "#ddd"; border.width: 1
                        }
                        onClicked: console.log("保存接收数据")
                    }

                    CheckBox { id: autoScrollCheck; text: "Auto-scroll"; checked: true; font.pixelSize: 14}
                    CheckBox { id: showTimeCheck; text: "Display-time"; checked: true; font.pixelSize: 14 }
                    CheckBox { id: showTopicCheck; text: "Display-topic"; checked: true; font.pixelSize: 14 }
                }

                Connections {
                    target: mqttObj
                    function onLogSend(msg){
                        var now = new Date();
                        receiveModel.append({
                            time: Qt.formatTime(now, "hh:mm:ss"),
                            topic: "Log",
                            payload: msg,
                        })
                    }
                }
                //receive message list
                ListView {
                    id: receiveList
                    model: receiveModel
                    spacing: 8
                    clip: true
                    Layout.fillWidth: true
                    Layout.fillHeight: true   // 让 ListView 占剩余空间

                    delegate: Rectangle {
                        width: ListView.view.width * 0.95
                        height: bubbleText.implicitHeight
                        radius: 8
                        color: index % 2 === 0 ? "#ffffff" : "#f5f9ff"
                        border.color: "#ddd"; border.width: 1

                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            onEntered: bubbleText.color = "#1b5e20"
                            onExited: bubbleText.color = "#2e7d32"
                        }

                        // 简单淡入动画
                                opacity: 0
                                Behavior on opacity { NumberAnimation { duration: 500 } }
                                Component.onCompleted: opacity = 1

                        Text {
                            id: bubbleText
                            anchors.fill: parent
                            anchors.margins: 8
                            font.pixelSize: 14
                            wrapMode: Text.WrapAnywhere
                            color: "#2e7d32"
                            text: {
                                var timeStr = showTimeCheck.checked ? "[" + model.time + "] " : "";
                                var topicStr = showTopicCheck.checked ? "[Topic: " + model.topic + "] " : "";
                                return timeStr + topicStr + model.payload;
                            }
                        }
                    }

                    onCountChanged: {
                        if (autoScrollCheck.checked && count > 0) {
                            Qt.callLater(positionViewAtEnd)
                        }
                    }


                }
            }

        }


        // 发布消息区
        GroupBox {
            id: publishGroup
            //title: "Publish Area"
            font.bold: true
            font.pixelSize: 18
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: parent.height * 0.4

            background: Rectangle {
                color: "transparent"
                radius: 6
                border.width: 1
                border.color: "#e0e0e0"
                DropShadow {
                    anchors.fill: parent
                    radius: 4
                    samples: 6
                    color: "#10000000"
                    opacity: 0.2
                    source: parent
                }
            }

            ColumnLayout {
                width: parent.width
                spacing: 4

                // Publish Topic field
                RowLayout {
                    width: parent.width
                    spacing: 4

                    //Publish Topic textField
                    Row {
                        id: msgTopicRow
                        spacing: 10

                        Label {
                            text: "Publish Topic:"
                            font.bold: true
                            font.pixelSize: 14
                            //color: "#333333"
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Item{
                            width: 400
                            height: 36
                            TextField {
                                id: msgTopicField
                                width: 394
                                height: 36
                                leftPadding: 5
                                rightPadding: 35
                                font.pixelSize: 14
                                text: settings.publsihTopic
                                font.family: "Segoe UI, Arial, sans-serif"
                                placeholderText: "example: sensors/temperature"
                                inputMethodHints: Qt.ImhNoPredictiveText
                                property bool isInvalid: false

                                background: Rectangle {
                                    id: msgTopicFieldBg
                                    color: msgTopicField.focus ? "#ffffff" : "transparent"
                                    border.color: msgTopicField.isInvalid ? "#e53935" :
                                                  msgTopicField.focus ? "#4caf50" : "#dddddd"
                                    border.width: 1
                                    radius: 4

                                    DropShadow {
                                        anchors.fill: msgTopicFieldBg
                                        radius: 3
                                        samples: 5
                                        color: "#80000000"
                                        source: msgTopicFieldBg
                                        opacity: msgTopicField.focus ? 0.3 : 0
                                        Behavior on opacity {
                                            NumberAnimation { duration: 200 }
                                        }
                                    }

                                    Behavior on border.color {
                                        ColorAnimation { duration: 200 }
                                    }
                                    Behavior on color {
                                        ColorAnimation { duration: 200 }
                                    }
                                }
                                onFocusChanged: {
                                    if (focus && text.length === 0) {
                                        console.log("提示: MQTT主题可以包含通配符 # 和 +")
                                    }
                                }
                                onTextChanged: {
                                    settings.publsihTopic = msgTopicField.text

                                }
                            }
                            Image {
                                id:clearImg
                                height: 24
                                width: 24
                                visible: msgTopicField.text.length > 0
                                anchors.right: parent.right
                                anchors.rightMargin: 10
                                anchors.verticalCenter:parent.verticalCenter
                                source: "/Resources/remove 01.png"
                                MouseArea{
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onEntered: {
                                        cursorShape = Qt.PointingHandCursor
                                    }
                                    onExited: {
                                        cursorShape = Qt.ArrowCursor
                                    }
                                    onClicked: {
                                        msgTopicField.isInvalid = false
                                        msgTopicField.clear()
                                    }
                                }
                            }

                        }


                    }
                }


                //message input textfield
                Rectangle {
                    width: 500
                    height: 120
                    border.color: "#ddd"
                    border.width: 1
                    radius: 4
                    color: "#00000080"

                    ScrollView {
                        id: scrollView
                        anchors.fill: parent
                        clip: true

                        TextArea {
                            id: mqttMessageInput
                            wrapMode: TextArea.Wrap
                            placeholderText: "Enter MQTT message..."
                            font.pixelSize: 14
                            focus: true
                            leftPadding: 5
                            topPadding: 5
                            bottomPadding: 5
                            rightPadding: 5
                        }
                    }
                }


                // Publish control
                Row {
                    height: parent.height
                    width: parent.width
                    spacing: 40

                    // Puclish Icon
                    Rectangle {
                        id: iconBg
                        width: 48
                        height: 48
                        radius: height/2
                        color: "#98FB98"

                        Image {
                            id: publishIconImg
                            anchors.centerIn: parent
                            width: 36
                            height: 36
                            source: "/Resources/ic-published.png"
                            MouseArea{
                                anchors.fill: parent
                                hoverEnabled: true
                                onEntered: {
                                    cursorShape = Qt.PointingHandCursor
                                }
                                onExited: {
                                    cursorShape = Qt.ArrowCursor
                                }
                                onClicked: {
                                    //MQTT_PublishData(char* topic, char* message, quint8 qos)
                                    console.log("原始消息（含转义字符）:", JSON.stringify(msgTopicField.text));
                                    console.log("原始消息（含转义字符）:", JSON.stringify(mqttMessageInput.text));
                                    mqttObj.MQTT_PublishData(msgTopicField.text, mqttMessageInput.text, 0)
                                }
                            }
                        }

                    }

                    Button {
                        id: clearPublishBtn
                        //text: "clear"
                        //color: "#666"
                        Text {
                            text: "clear"
                            color: "#f8f8f8"
                            anchors.centerIn: parent
                            font.pixelSize: 16
                        }
                        width: 50
                        height: 32
                        anchors.verticalCenter: parent.verticalCenter
                        font.pixelSize: 14

                        background: Rectangle {
                            radius: 4
                            color: clearPublishBtn.hovered ? "#f0f0f0" : "transparent"
                            border.color: "#ddd"
                            border.width: 1
                        }

                        MouseArea{
                            anchors.fill: parent
                            hoverEnabled: true
                            onEntered: {
                                cursorShape = Qt.PointingHandCursor
                            }
                            onExited: {
                                cursorShape = Qt.ArrowCursor
                            }
                            onClicked: {
                                mqttMessageInput.text = ""
                            }
                        }
                    }

                    // CheckBox {
                    //     id: retainCheck
                    //     anchors.verticalCenter: parent.verticalCenter
                    //     text: "Retain"
                    //     font.pixelSize: 14
                    // }

                    Item { Layout.fillWidth: true }

                    Label {
                        id: statusLabel
                        anchors.verticalCenter: parent.verticalCenter
                        text: "Ready"
                        font.pixelSize: 14
                        color: "#f8f8f8"
                    }
                }
            }
        }
    }

    // 数据模型
    ListModel {
        id: receiveModel
        // ListElement { time: "14:23:45"; topic: "sensors/temp"; payload: "25.8°C" }
        ListElement { time: "08:03:30"; topic: "Welcome"; payload: "princess&pigcess!\n" }
    }


}
