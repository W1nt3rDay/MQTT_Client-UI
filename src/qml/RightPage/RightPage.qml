import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Basic
import Qt5Compat.GraphicalEffects
import QtCore

Rectangle {
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
            title: "Message Area"
            font.pixelSize: 18
            font.bold: true
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: parent.height * 0.6
            clip: true

            background: Rectangle {
                color: "#f0f0f0"
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
                    Layout.preferredHeight: 24

                    Button {
                        id: clearReceiveBtn
                        text: "Clear"
                        width: 50; height: 28
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

                    CheckBox { id: autoScrollCheck; text: "Auto-scroll"; checked: true; font.pixelSize: 14 }
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

                    // add: Transition {
                    //     id: addItemTransition
                    //     NumberAnimation {
                    //         property: "opacity";
                    //         from: 0; to: 1;
                    //         duration: 500;
                    //         easing.type: Easing.InOutQuad
                    //     }

                    // }

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
            title: "Publish Area"
            font.bold: true
            font.pixelSize: 18
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: parent.height * 0.4

            background: Rectangle {
                color: "#f0f0f0"
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
                            color: "#333333"
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
                                    color: msgTopicField.focus ? "#ffffff" : "#f5f5f5"
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
                    height: 75
                    border.color: "#ddd"
                    border.width: 1
                    radius: 4
                    color: "white"

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
                                    mqttObj.MQTT_PublishData(msgTopicField.text, mqttMessageInput.text, 1)
                                }
                            }
                        }

                    }

                    Button {
                        id: clearPublishBtn
                        text: "clear"
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

                    CheckBox {
                        id: retainCheck
                        anchors.verticalCenter: parent.verticalCenter
                        text: "Retain"
                        font.pixelSize: 14
                    }

                    Item { Layout.fillWidth: true }

                    Label {
                        id: statusLabel
                        anchors.verticalCenter: parent.verticalCenter
                        text: "Ready"
                        font.pixelSize: 14
                        color: "#666"
                    }
                }
            }
        }
    }

    // 数据模型
    ListModel {
        id: receiveModel
        // ListElement { time: "14:23:45"; topic: "sensors/temp"; payload: "25.8°C" }
        // ListElement { time: "14:24:10"; topic: "sensors/humidity"; payload: "62%" }
    }

    // 发布消息函数
    function publishMessage(topic, payload, qos) {
        const timeStr = Qt.formatTime(new Date(), "hh:mm:ss")
        statusLabel.text = `已发布到 ${topic}`
        statusLabel.color = "#2e7d32"

        setTimeout(() => {
            statusLabel.text = "就绪"
            statusLabel.color = "#666"
        }, 1500)

        console.log(`[${timeStr}] 发布到 ${topic}: ${payload}`)
    }

    // 接收消息函数
    function receiveMessage(topic, payload) {
        const timeStr = Qt.formatTime(new Date(), "hh:mm:ss")
        if (receiveModel.count >= 500) receiveModel.remove(0, 1)
        receiveModel.append({ time: timeStr, topic: topic, payload: payload })
        if (autoScrollCheck.checked) receiveList.positionViewAtEnd()
    }
}
