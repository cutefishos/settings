import QtQuick 2.4
import QtQuick.Layouts 1.3
import MeuiKit 1.0 as Meui

Item {
    id: control
    height: Meui.Units.largeSpacing * 2

    Layout.fillWidth: true

    Rectangle {
        anchors.centerIn: parent
        height: 1
        width: control.width
        color: Meui.Theme.disabledTextColor
        opacity: Meui.Theme.darkMode ? 0.3 : 0.2
    }
}
