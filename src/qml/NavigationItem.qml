import QtQuick 2.12
import QtQuick.Layouts 1.12
import FishUI 1.0 as FishUI

// A ListRow that leads to another page.
ListRow {
    id: control

    FishUI.Icon {
        Layout.preferredWidth: 16
        Layout.preferredHeight: 16
        name: "chevron-right"
        mirror: control.LayoutMirroring.enabled
        opacity: 0.6
    }
}
