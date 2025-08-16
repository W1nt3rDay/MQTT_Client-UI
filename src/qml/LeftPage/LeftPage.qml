import QtQuick 2.15
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import QtQuick.Controls.Basic

Rectangle {

    Column {
        anchors.fill: parent
        spacing: 10
        padding: 50

        //Server IP textfield
        Row{
            spacing: 20
            Label{
                text: "Server IP:"
                font.bold: true
                font.pixelSize: 20
            }
            TextField{
                id: serverIPTextField
                width: 200
                height: 28
                leftPadding: 5
                font.pixelSize: 18
                font.family: "Times New Roman"
                placeholderText: "192.168.1.1"
            }
        }

        //Port textfield
        Row{
            spacing: 64
            Label{
                text: "Port:"
                font.bold: true
                font.pixelSize: 20
            }
            TextField{
                id: portField
                width: 200
                height: 28
                leftPadding: 5
                font.pixelSize: 18
                font.family: "Times New Roman"
                placeholderText: "1883"
            }
        }

        //Client ID textfield
        Row{
            spacing: 23
            Label{
                text: "Client ID:"
                font.bold: true
                font.pixelSize: 20
            }
            TextField{
                id: clientIdField
                width: 200
                height: 28
                leftPadding: 5
                font.pixelSize: 18
                font.family: "Times New Roman"
                }
        }

        //Username textfield
        Row{
            spacing: 11
            Label{
                text: "Username:"
                font.bold: true
                font.pixelSize: 20
            }
            TextField{
                id: userNameField
                width: 200
                height: 28
                leftPadding: 5
                font.pixelSize: 18
                font.family: "Times New Roman"
                }
        }

        //Password textfield
        Row{
            spacing: 16
            Label{
                text: "Password:"
                font.bold: true
                font.pixelSize: 20
            }
            TextField{
                id: passwordField
                width: 200
                height: 28
                leftPadding: 5
                font.pixelSize: 18
                font.family: "Times New Roman"
            }
        }

        // connection Button
        Button {
            id: connectButton
            text: isConnected ? "Disconnect" : "Connect"
            anchors.left: parent.left
            anchors.leftMargin: 110
            width: 180
            height: 40
            font.bold: true
            font.pixelSize: 16
            property bool isConnected: false
            property bool isHovered: false

            // 重写contentItem以同时控制Image和Text
            contentItem: Item {
                id: contentContainer
                anchors.fill: parent

                scale: connectButton.isHovered ? 1.1 : 1.0
                Behavior on scale {
                    NumberAnimation {
                        duration: 200
                        easing.type: Easing.OutQuad
                    }
                }

                Image {
                    id: playImg
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: 16
                    width: 35
                    height: 35
                    source: connectButton.isConnected ? "/Resources/stop.png" : "/Resources/play.png"
                }

                Text {
                    id: buttonText
                    text: connectButton.text
                    font.bold: connectButton.font.bold
                    font.pixelSize: connectButton.font.pixelSize
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: playImg.right
                    anchors.leftMargin: 15
                }
            }

            background: Rectangle{
                border.width: 2
                border.color: connectButton.isConnected ? "#e53935" : "#4caf50"
                Behavior on border.color {
                    ColorAnimation { duration: 300 }
                }
                color: "transparent"
                radius: parent.height/2

                // 背景缩放也直接使用scale属性
                scale: connectButton.isHovered ? 1.05 : 1.0
                Behavior on scale {
                    NumberAnimation { duration: 200 }
                }
            }

            MouseArea{
                anchors.fill: parent
                hoverEnabled: true
                onEntered: {
                    cursorShape = Qt.PointingHandCursor
                    connectButton.isHovered = true
                }
                onExited:{
                    cursorShape = Qt.ArrowCursor
                    connectButton.isHovered = false
                }
                onClicked: {
                    connectButton.isConnected = !connectButton.isConnected
                    console.log(connectButton.isConnected ? "Connected" : "Disconnected")
                    mqttClient.setServerIP(serverIPTextField.text)
                    mqttClient.setServerPort(portField.text)
                    mqttClient.setClientID(clientIdField.text)
                    mqttClient.setUserName(userNameField.text)
                    mqttClient.setPassword(passwordField.text)
                    //mqttClient.start()
                }
            }
        }

        //Subscribe Topic textfield
        Row {
            id: subscribeRow
            spacing: 21

            // 标签样式优化
            Label {
                text: "Subscribe Topic:"
                font.bold: true
                font.pixelSize: 18
                color: "#333333"  // 深灰色更显专业
                anchors.verticalCenter: parent.verticalCenter  // 与输入框垂直居中对齐
            }

            Item{
                width: 220
                height: 36
                // 带样式的输入框
                TextField {
                    id: topicField
                    width: 220  // 增加宽度，适应较长的主题名
                    height: 36
                    leftPadding: 10
                    rightPadding: 30
                    font.pixelSize: 16
                    font.family: "Segoe UI, Arial, sans-serif"  // 更通用的字体选择
                    placeholderText: "例如: sensors/temperature"  // 更贴合MQTT主题的示例
                    inputMethodHints: Qt.ImhNoPredictiveText  // 关闭预测文本，适合技术输入
                    // 自定义验证状态属性
                    property bool isInvalid: false

                    // 自定义背景
                    background: Rectangle {
                        id: fieldBg
                        color: topicField.focus ? "#ffffff" : "#f5f5f5"
                        border.color: topicField.isInvalid ? "#e53935" :
                                      topicField.focus ? "#4caf50" : "#dddddd"
                        border.width: 1.5
                        radius: 6

                        // 聚焦时的微妙阴影效果
                        DropShadow {
                            anchors.fill: fieldBg
                            radius: 3
                            samples: 5
                            color: "#80000000"
                            source: fieldBg
                            opacity: topicField.focus ? 0.3 : 0
                            Behavior on opacity {
                                NumberAnimation { duration: 200 }
                            }
                        }

                        // 状态变化动画
                        Behavior on border.color {
                            ColorAnimation { duration: 200 }
                        }
                        Behavior on color {
                            ColorAnimation { duration: 200 }
                        }
                    }

                    // 聚焦时的提示
                    onFocusChanged: {
                        if (focus && text.length === 0) {
                            console.log("提示: MQTT主题可以包含通配符 # 和 +")
                        }
                    }

                    ToolTip {
                        id: topicErrorTip
                        text: "主题只能包含字母、数字、下划线及/#+-.等字符"
                        visible: topicField.isInvalid && topicField.focus
                        delay: 500
                    }

                    onTextChanged: {
                        // 使用自定义正则验证，不依赖error属性
                        const validRegex = /^[\w\/#\+\-]*$/
                        isInvalid = text.length > 0 && !validRegex.test(text)
                    }
                }
                Image {
                    id:removeIconImg
                    height: 24
                    width: 24
                    visible: topicField.text.length > 0
                    anchors.right: parent.right
                    anchors.rightMargin: 5
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
                            topicField.isInvalid = false
                            topicField.clear()
                        }
                    }
                }

            }
        }
        //subscribe button
        Button{
            id: subscribeButton
            text: isSubscribed ? "Unsubscribe" : "Subscribe"
            width: 120
            height: 36
            font.bold: true
            font.pixelSize: 14
            property bool isSubscribed: false  //subscribe status

            background: Rectangle{
                radius: 6
                border.width: 2
                /*status color: subscribed-deepGreen,
                 *              unsubscribed-lightGreen,
                 *              hover-brightGreen
                 */
                border.color: {
                    if (subscribeButton.isSubscribed)return "#e53935"
                    return subscribeButton.hovered ? "#66bb6a" : "#4caf50"
                }
                color: subscribeButton.hovered ? "#f1f8e9" : "transparent"

                //status change animation
                Behavior on border.color {
                    ColorAnimation{ duration: 200}
                }
                Behavior on color {
                    ColorAnimation{ duration: 200}
                }
            }

            MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        console.log("111")
                        subscribeButton.isSubscribed = !subscribeButton.isSubscribed;
                        if (subscribeButton.isSubscribed) {
                            console.log("Subscribed Topic:", topicField.text);
                            // 实际订阅逻辑...
                        } else {
                            console.log("Unsubscribed Topic:", topicField.text);
                            // 实际取消订阅逻辑...
                        }
                    }
            }

        }


    }
}
