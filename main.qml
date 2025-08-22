import QtQuick 2.15
import QtQuick.Window 2.15
import QtMultimedia
import MyMqtt 1.0
import "./src/qml/basic"
import "./src/qml/LeftPage"
import "./src/qml/RightPage"

Window {
    width: 1000
    height: 600
    visible: true
    color: "#f0f0f0"
    title: qsTr("MQTT Client V1.0")

    Video{
        id: backgroundVideo
        anchors.fill: parent
        source: "qrc:/Resources/Rain Background.webm"
        autoPlay: true
        loops: MediaPlayer.Infinite
        z: -1
        fillMode: VideoOutput.Stretch
    }


    MQTT_WorkClass{
        id: mqttObj
    }

    Row{
        id: mainRow
        spacing: 60
        width: parent.width
        height: parent.height


        //leftpage: parameters information
        LeftPage{
            id: leftPage
            //color: "#f0f0f0"
            width: parent.width * 0.4
            height: mainRow.height
        }

        //rightpage: output & debug info
        RightPage{
            id: rightPage
            //color: "#f0f0f0"
            width: parent.width * 0.52
            height: mainRow.height
        }
    }

}
