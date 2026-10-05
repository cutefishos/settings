import QtQuick
import QtQuick.Layouts
import Cutefish.Settings
import FishUI as FishUI

// A pill-shaped search field with a search glyph and a clear button.
FishUI.TextField {
    id: control

    // The default suits the page colour; the sidebar passes its frosted fill.
    property color fill: FishUI.Theme.secondBackgroundColor

    readonly property bool mirrored: Qt.application.layoutDirection === Qt.RightToLeft
    readonly property real iconPadding: Metrics.s + _searchIcon.width + Metrics.xs
    readonly property real clearPadding: _clearButton.visible ? Metrics.xs + _clearButton.width + Metrics.s
                                                              : Metrics.s

    Layout.fillWidth: true
    implicitHeight: 28
    leftPadding: mirrored ? clearPadding : iconPadding
    rightPadding: mirrored ? iconPadding : clearPadding
    topPadding: 0
    bottomPadding: 0
    selectByMouse: true

    background: FishUI.RoundedRectangle {
        // Pill: radius is always half the height.
        radius: height / 2
        continuousCorners: false
        color: control.fill
        border.width: 1
        border.color: control.activeFocus ? FishUI.Theme.highlightColor : control.fill

        FishUI.RoundedRectangle {
            anchors.fill: parent
            anchors.margins: -2
            // The field's radius; inset grows it into a concentric ring.
            radius: parent.height / 2
            continuousCorners: false
            inset: -2
            color: "transparent"
            border.width: 2
            border.color: FishUI.Theme.highlightColor
            opacity: 0.25
            visible: control.activeFocus
        }
    }

    FishUI.Icon {
        id: _searchIcon
        anchors.left: parent.left
        anchors.leftMargin: Metrics.s
        anchors.verticalCenter: parent.verticalCenter
        width: 14
        height: 14
        name: "search"
        color: FishUI.Theme.disabledTextColor
    }

    FishUI.Icon {
        id: _clearButton
        anchors.right: parent.right
        anchors.rightMargin: Metrics.s
        anchors.verticalCenter: parent.verticalCenter
        width: 14
        height: 14
        visible: control.length > 0
        name: "clear"
        color: FishUI.Theme.disabledTextColor

        MouseArea {
            anchors.fill: parent
            // Easier to hit than the glyph alone.
            anchors.margins: -Metrics.xs
            onClicked: control.clear()
        }
    }
}
