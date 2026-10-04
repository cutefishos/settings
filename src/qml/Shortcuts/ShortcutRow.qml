import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cutefish.Settings
import FishUI as FishUI
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

    FishUI.Label {
        visible: control.accelerators.length === 0
        text: qsTr("None")
        color: FishUI.Theme.disabledTextColor
    }

    Column {
        spacing: Metrics.xs
        Layout.alignment: Qt.AlignVCenter
        Layout.topMargin: Metrics.xs
        Layout.bottomMargin: Metrics.xs

        Repeater {
            model: control.accelerators

            ShortcutKeys {
                parts: modelData.parts
                dimmed: !control.enabled
                anchors.right: parent.right
            }
        }
    }
}
