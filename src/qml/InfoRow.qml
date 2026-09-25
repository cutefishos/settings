import QtQuick 2.12
import FishUI 1.0 as FishUI

// A read-only ListRow: the label, then its value in secondary text. Hidden
// while there is no value.
ListRow {
    id: control

    // A var: models leave missing roles undefined.
    property var value

    interactive: false
    visible: !!value

    FishUI.Label {
        text: control.value || ""
        textFormat: Text.PlainText
        color: FishUI.Theme.disabledTextColor
    }
}
