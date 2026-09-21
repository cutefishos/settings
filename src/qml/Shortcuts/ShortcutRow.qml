import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import FishUI 1.0 as FishUI

// One action: what it does on the left, how to trigger it on the right.
Item {
    id: control

    // Filled by the Repeater from the model's roles. Declaring them again in
    // the delegate would only shadow these and leave the labels empty.
    required property string name
    required property string category
    required property string description
    required property var accelerators
    required property bool isDefault
    signal clicked()

    Layout.fillWidth: true
    implicitHeight: Math.max(40, layout.implicitHeight + FishUI.Units.smallSpacing * 2)

    Rectangle {
        anchors.fill: parent
        anchors.leftMargin: -FishUI.Units.smallSpacing
        anchors.rightMargin: -FishUI.Units.smallSpacing
        radius: FishUI.Theme.smallRadius
        color: mouseArea.pressed ? Qt.rgba(FishUI.Theme.textColor.r, FishUI.Theme.textColor.g,
                                           FishUI.Theme.textColor.b, 0.1)
             : mouseArea.containsMouse ? Qt.rgba(FishUI.Theme.textColor.r, FishUI.Theme.textColor.g,
                                                 FishUI.Theme.textColor.b, 0.05)
                                       : "transparent"
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: control.clicked()
    }

    RowLayout {
        id: layout
        anchors.fill: parent
        spacing: FishUI.Units.largeSpacing

        Label {
            Layout.fillWidth: true
            text: control.description
            color: FishUI.Theme.textColor
            elide: Text.ElideRight
        }

        Label {
            visible: control.accelerators.length === 0
            text: qsTr("None")
            color: FishUI.Theme.disabledTextColor
        }

        Column {
            spacing: 4
            Layout.alignment: Qt.AlignVCenter

            Repeater {
                model: control.accelerators

                ShortcutKeys {
                    parts: modelData.parts
                    anchors.right: parent.right
                }
            }
        }
    }
}
