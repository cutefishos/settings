import QtQuick 2.12
import QtQuick.Layouts 1.12
import FishUI 1.0 as FishUI

// A ListRow that leads to another page.
ListRow {
    id: control

    // The current choice, shown before the chevron.
    property string value

    FishUI.Label {
        Layout.maximumWidth: control.width / 2
        visible: control.value.length > 0
        text: control.value
        textFormat: Text.PlainText
        color: FishUI.Theme.disabledTextColor
        elide: Text.ElideRight
    }

    FishUI.Icon {
        Layout.preferredWidth: 16
        Layout.preferredHeight: 16
        name: "chevron-right"
        mirror: control.LayoutMirroring.enabled
        opacity: 0.6
    }
}
