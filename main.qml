import QtQuick 2.15
import QtQuick.Window 2.15

import "./src/qml/basic"
import "./src/qml/LeftPage"
import "./src/qml/RightPage"

Window {
    width: 1000
    height: 600
    visible: true
    color: "#f0f0f0"
    title: qsTr("MQTT Client V1.0")


    Row{
        id: mainRow
        spacing: 60
        width: parent.width
        height: parent.height
        Component.onCompleted: {
                    console.log("Row 实际宽高:", mainRow.width, mainRow.height)
                    console.log("LeftPage 实际宽高:", leftPage.width, leftPage.height)
                    console.log("RightPage 实际宽高:", rightPage.width, rightPage.height)
        }

        //leftpage: parameters information
        LeftPage{
            id: leftPage
            color: "#f0f0f0"
            width: parent.width * 0.4
            height: mainRow.height
        }

        //rightpage: output & debug info
        RightPage{
            id: rightPage
            color: "#f0f0f0"
            width: parent.width * 0.52
            height: mainRow.height
        }
    }

}
