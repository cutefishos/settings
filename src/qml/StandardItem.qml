import QtQuick 2.4
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import FishUI 1.0 as FishUI

Item {
    id: control

    height: mainLayout.implicitHeight + FishUI.Theme.smallRadius * 2

    property alias key: keyLabel.text
    property alias value: valueLabel.text

    Layout.fillWidth: true

    Rectangle {
        id: background
        anchors.fill: parent
        color: "transparent"
        radius: FishUI.Theme.smallRadius
    }

    RowLayout {
        id: mainLayout
        anchors.fill: parent

        Label {
            id: keyLabel
            color: FishUI.Theme.textColor
        }

        Item {
            Layout.fillWidth: true
        }

        Label {
            id: valueLabel
            color: FishUI.Theme.disabledTextColor
        }
    }
}
