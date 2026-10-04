import QtQuick
import FishUI as FishUI

// The sidebar's tile: a coloured rounded square with a white glyph, for a
// ListRow's leading slot.
Rectangle {
    id: control

    property string source
    property color tint

    width: 24
    height: 24
    radius: 6

    gradient: Gradient {
        GradientStop { position: 0.0; color: Qt.lighter(control.tint, 1.15) }
        GradientStop { position: 1.0; color: control.tint }
    }

    FishUI.IconItem {
        anchors.centerIn: parent
        width: 16
        height: width
        source: control.source.startsWith("qrc:") ? control.source
                                                  : "qrc:/images/sidebar/dark/" + control.source
        // The sidebar's own glyphs are white already; FishUI's shared ones need tinting.
        color: "white"
    }
}
