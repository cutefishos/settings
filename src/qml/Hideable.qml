import QtQuick 2.4
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0

Item {
    id: _root
    implicitHeight: shown ? _content.implicitHeight : 0
    clip: true
    Layout.fillWidth: true

    Behavior on implicitHeight {
        NumberAnimation {
            duration: 125
            easing.type: Easing.InOutCubic
        }
    }

    property bool shown: false
    default property alias content: _content.data

    function show() {
        shown = true
    }

    function hide() {
        shown = false
    }

    function toggle() {
        shown = !shown
    }

    ColumnLayout {
        id: _content
        anchors.fill: parent
    }
}
