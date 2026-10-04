import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FishUI as FishUI
import "../"

// One choice of several: its name and a check when it is the current one.
ListRow {
    id: control

    Accessible.role: Accessible.RadioButton
    Accessible.checked: checked

    FishUI.Icon {
        Layout.preferredWidth: 18
        Layout.preferredHeight: 18
        Layout.rightMargin: Metrics.trailingGlyphMargin(name, 18)
        name: "check"
        color: FishUI.Theme.highlightColor
        visible: control.checked
    }
}
