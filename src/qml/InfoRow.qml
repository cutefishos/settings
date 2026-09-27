import QtQuick 2.12
import QtQuick.Layouts 1.12
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
        // Long values such as a processor name would otherwise squeeze the label out.
        Layout.maximumWidth: Math.round(control.width * 0.65)
        text: control.value || ""
        textFormat: Text.PlainText
        color: FishUI.Theme.disabledTextColor
        horizontalAlignment: Text.AlignRight
        wrapMode: Text.WordWrap
        maximumLineCount: 2
        elide: Text.ElideRight
    }
}
