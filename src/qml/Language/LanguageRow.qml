import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cutefish.Settings 1.0
import FishUI 1.0 as FishUI
import "../"

// A language to pick: its own name, its name in the system language, and a
// check when it is the chosen one.
ListRow {
    id: control

    property string languageName
    property string description

    Accessible.role: Accessible.RadioButton
    Accessible.name: languageName
    Accessible.checked: checked

    ColumnLayout {
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignVCenter
        spacing: Metrics.textSpacing

        FishUI.Label {
            id: primaryLabel
            Layout.fillWidth: true
            text: control.languageName
            textFormat: Text.PlainText
            color: control.enabled ? FishUI.Theme.textColor : FishUI.Theme.disabledTextColor
            elide: Text.ElideRight
        }

        FishUI.Label {
            Layout.fillWidth: true
            text: control.description
            textFormat: Text.PlainText
            color: FishUI.Theme.disabledTextColor
            font.pixelSize: Math.max(1, primaryLabel.font.pixelSize - 1)
            elide: Text.ElideRight
        }
    }

    FishUI.Icon {
        Layout.preferredWidth: 18
        Layout.preferredHeight: 18
        name: "check"
        color: FishUI.Theme.highlightColor
        visible: control.checked
    }
}
