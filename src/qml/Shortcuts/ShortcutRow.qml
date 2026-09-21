import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import FishUI 1.0 as FishUI
import "../"

// One action: what it does on the left, how to trigger it on the right.
ListRow {
    id: control

    // Filled by the Repeater from the model's roles.
    required property string name
    required property string category
    required property string description
    required property var accelerators
    required property bool isDefault

    text: description

    Label {
        visible: control.accelerators.length === 0
        text: qsTr("None")
        color: FishUI.Theme.disabledTextColor
    }

    Column {
        spacing: 4
        Layout.alignment: Qt.AlignVCenter
        Layout.topMargin: 6
        Layout.bottomMargin: 6

        Repeater {
            model: control.accelerators

            ShortcutKeys {
                parts: modelData.parts
                anchors.right: parent.right
            }
        }
    }
}
