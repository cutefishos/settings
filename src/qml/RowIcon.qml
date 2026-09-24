import QtQuick 2.12
import FishUI 1.0 as FishUI

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

    Image {
        anchors.centerIn: parent
        width: 16
        height: width
        source: "qrc:/images/sidebar/dark/" + control.source
        antialiasing: false
        smooth: false
    }
}
