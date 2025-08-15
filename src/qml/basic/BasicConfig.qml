pragma Singleton
import QtQuick 2.15

QtObject{
    readonly property real screenWidth: Screen.width                   //屏幕宽度
    readonly property real screenHeight: Screen.height                 //屏幕高度
}
