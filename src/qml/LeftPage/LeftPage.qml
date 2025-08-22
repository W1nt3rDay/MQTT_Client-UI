import QtQuick 2.15
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import QtQuick.Controls.Basic
import QtCore
import Qt.labs.lottieqt
Item {

    Column {
        anchors.fill: parent
        spacing: 10
        padding: 50

        //history input
        Settings{
            id: settings
            property string host: ""
            property int port: 1883
            property string clientID: ""
            property string username: ""
            property string password: ""
            property string topic: ""
        }

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
                text: settings.host
                background: Rectangle{
                    color: "transparent"
                    border.width: 1
                    radius: 4
                    border.color: "#666"
                }

                onTextChanged: {
                    settings.host = text
                }
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
                text: settings.port
                onTextChanged: {
                    settings.port = parseInt(text) || 1883
                }
                background: Rectangle{
                    color: "transparent"
                    border.width: 1
                    radius: 4
                    border.color: "#666"
                }
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
                background: Rectangle{
                    color: "transparent"
                    border.width: 1
                    radius: 4
                    border.color: "#666"
                }
                text: settings.clientID
                onTextChanged: {
                    settings.clientID = text
                }
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
                background: Rectangle{
                    color: "transparent"
                    border.width: 1
                    radius: 4
                    border.color: "#666"
                }
                text: settings.username
                onTextChanged: {
                    settings.username = text
                }
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
                text: settings.password
                onTextChanged: {
                    settings.password = text
                }
                background: Rectangle{
                    color: "transparent"
                    border.width: 1
                    radius: 4
                    border.color: "#666"
                }
            }
        }

        // connection Button
        Button {
            id: connectButton
            //text: isConnected ? "Disconnect" : "Connect"
            Text {
                text: connectButton.isConnected ? "Disconnect" : "Connect"
                color: "#f8f8f8"
                anchors.centerIn: parent
                font.pixelSize: 16
            }
            anchors.left: parent.left
            anchors.leftMargin: 110
            width: 180
            height: 40
            font.bold: true
            font.pixelSize: 16
            property bool isConnected: false
            property bool isHovered: false

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
                    if (connectButton.isConnected == false){
                        mqttObj.Set_MQTT_Addr(serverIPTextField.text,
                                              portField.text,
                                              clientIdField.text,
                                              userNameField.text,
                                              passwordField.text)
                        mqttObj.run()
                        mqttHeartTimer.start()
                    }
                    else{
                        mqttObj.MQTT_Disconnect()
                        mqttHeartTimer.stop()
                    }
                    connectButton.isConnected = !connectButton.isConnected
                }
            }
        }

        Timer{
            id: mqttHeartTimer
            interval: 30000
            repeat: true
            running: false
            onTriggered: {
                mqttObj.MQTT_SentHeart()
                console.log("MQTT hearbeat sent");
            }
        }

        //Subscribe Topic textfield
        Row {
            id: subscribeRow
            spacing: 21

            Label {
                text: "Subscribe Topic:"
                font.bold: true
                font.pixelSize: 18
                anchors.verticalCenter: parent.verticalCenter
            }

            Item{
                width: 220
                height: 36
                TextField {
                    id: topicField
                    width: 220
                    height: 36
                    leftPadding: 10
                    rightPadding: 30
                    font.pixelSize: 16
                    font.family: "Segoe UI, Arial, sans-serif"
                    placeholderText: "例如: sensors/temperature"
                    text: settings.topic
                    inputMethodHints: Qt.ImhNoPredictiveText
                    property bool isInvalid: false

                    background: Rectangle {
                        id: fieldBg
                        color: topicField.focus ? "#ffffff" : "transparent"
                        border.color: topicField.isInvalid ? "#e53935" :
                                      topicField.focus ? "#4caf50" : "#666"
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
                    onFocusChanged: {
                        if (focus && text.length === 0) {
                            console.log("提示: MQTT主题可以包含通配符 # 和 +")
                        }
                    }

                    // ToolTip {
                    //     id: topicErrorTip
                    //     text: "主题只能包含字母、数字、下划线及/#+-.等字符"
                    //     visible: topicField.isInvalid && topicField.focus
                    //     delay: 500
                    // }

                    onTextChanged: {
                        // const validRegex = /^[\w\/#\+\-]*$_/
                        // isInvalid = text.length > 0 && !validRegex.test(text)

                        settings.topic = text;
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
            //text: isSubscribed ? "Unsubscribe" : "Subscribe"
            Text {
                text: subscribeButton.isSubscribed ? "Unsubscribe" : "Subscribe"
                color: "#f8f8f8"  // 设置文本颜色为橙色
                anchors.centerIn: parent
                font.pixelSize: 16
            }
            width: 120
            height: 36
            font.bold: true
            font.pixelSize: 14
            property bool isSubscribed: false  //subscribe status

            background: Rectangle{
                radius: 6
                border.width: 2
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
                        subscribeButton.isSubscribed = !subscribeButton.isSubscribed;
                        if (subscribeButton.isSubscribed && connectButton.isConnected) {
                            console.log("Subscribed Topic:", topicField.text);
                            mqttObj.MQTT_SubscribeTopic(topicField.text, 0, 1)
                        } else {
                            console.log("Unsubscribed Topic:", topicField.text);
                            //TODO: unsubscribe logic
                        }
                    }
            }

        }

        Item{
            width: 264
            height: 264
            //color: "#f0f0f0"
            LottieAnimation{
                id: cuteAnim
                anchors.centerIn: parent
                scale: 0.1
                source: "qrc:/Resources/Cute_pig.json"
                loops: LottieAnimation.Infinite


                MouseArea{
                    anchors.fill: parent
                    onClicked: {
                        //cuteAnim.restart()
                    }
                }
            }
        }

    }
}
