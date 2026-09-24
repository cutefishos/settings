import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Cutefish.Settings 1.0
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
                anchors.right: parent.right
            }
        }
    }
}
