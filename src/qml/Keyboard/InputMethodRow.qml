import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import FishUI 1.0 as FishUI
import "../"

// One selectable input method: icon, name and a check when it is current.
ListRow {
    id: control

    property string iconSource

    Accessible.role: Accessible.RadioButton
    Accessible.checked: checked

    leading: Item {
        width: 24
        height: 24

        FishUI.IconItem {
            anchors.centerIn: parent
            width: 18
            height: 18
            source: control.iconSource ? control.iconSource : "input-keyboard"
            smooth: true
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
