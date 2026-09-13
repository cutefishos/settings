import QtQuick 2.12
import FishUI 1.0 as FishUI

Item {
    id: control
    width: 24
    height: 24

    property alias source: _image.source
    property color hoveredColor: FishUI.Theme.darkMode ? Qt.lighter(FishUI.Theme.secondBackgroundColor, 1.7)
                                                   : Qt.darker(FishUI.Theme.secondBackgroundColor, 1.2)
    property color pressedColor: FishUI.Theme.darkMode ? Qt.lighter(FishUI.Theme.secondBackgroundColor, 1.4)
                                                     : Qt.darker(FishUI.Theme.secondBackgroundColor, 1.3)

    signal clicked()

    Rectangle {
        anchors.fill: parent
        // Same inset as FishUI's window buttons, so both draw the same circle.
        anchors.margins: Math.round(control.width * 0.1)
        radius: width / 2
        color: !control.enabled ? "transparent"
             : _mouseArea.pressed ? control.pressedColor
             : _mouseArea.containsMouse ? control.hoveredColor
             : "transparent"
    }

    Image {
        id: _image
        anchors.centerIn: parent
        width: 18
        height: width
        sourceSize: Qt.size(width, height)
        smooth: false
        antialiasing: true
        mirror: control.LayoutMirroring.enabled
        opacity: control.enabled ? 1 : 0.35
    }

    MouseArea {
        id: _mouseArea
        anchors.fill: parent
        enabled: control.enabled
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton
        onClicked: control.clicked()
    }
}
