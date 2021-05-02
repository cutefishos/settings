import QtQuick 2.12
import QtQuick.Layouts 1.12
import FishUI 1.0 as FishUI

Rectangle {
    Layout.fillWidth: true

    default property alias content : _mainLayout.data

    color: FishUI.Theme.darkMode ? "#363636" : "#FFFFFF"
    radius: FishUI.Theme.mediumRadius

    implicitHeight: _mainLayout.implicitHeight +
                    _mainLayout.anchors.topMargin +
                    _mainLayout.anchors.bottomMargin

    ColumnLayout {
        id: _mainLayout
        anchors.fill: parent
        anchors.leftMargin: FishUI.Units.largeSpacing * 1.5
        anchors.rightMargin: FishUI.Units.largeSpacing * 1.5
        anchors.topMargin: FishUI.Units.largeSpacing
        anchors.bottomMargin: FishUI.Units.largeSpacing
    }
}
