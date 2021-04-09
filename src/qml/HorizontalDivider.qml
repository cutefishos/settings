import QtQuick 2.4
import QtQuick.Layouts 1.3
import FishUI 1.0 as FishUI

Item {
    id: control
    height: FishUI.Units.largeSpacing * 2

    Layout.fillWidth: true

    Rectangle {
        anchors.centerIn: parent
        height: 1
        width: control.width
        color: FishUI.Theme.disabledTextColor
        opacity: FishUI.Theme.darkMode ? 0.3 : 0.2
    }
}
