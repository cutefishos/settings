import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import FishUI 1.0 as FishUI

// A row of a zero-margin RoundedItem; the hover background fills the whole row.
Item {
    id: control

    property alias text: _label.text
    property bool firstItem: true
    property bool lastItem: true
    signal clicked()

    readonly property real horizontalPadding: FishUI.Units.largeSpacing * 1.5
    readonly property color hoverColor: Qt.rgba(FishUI.Theme.textColor.r, FishUI.Theme.textColor.g,
                                                FishUI.Theme.textColor.b,
                                                // Light cards are white, so 0.05 would match the page background.
                                                _mouseArea.pressed ? (FishUI.Theme.darkMode ? 0.05 : 0.15)
                                                                   : 0.1)

    Layout.fillWidth: true
    implicitHeight: 40

    // One shape only: overlapping translucent pieces would darken where they meet.
    Item {
        anchors.fill: parent
        clip: true
        visible: _mouseArea.containsMouse

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            // Push the corners that meet a neighbouring row outside the clip.
            anchors.topMargin: control.firstItem ? 0 : -radius
            anchors.bottomMargin: control.lastItem ? 0 : -radius
            radius: FishUI.Theme.mediumRadius
            color: control.hoverColor
        }
    }

    MouseArea {
        id: _mouseArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton
        onClicked: control.clicked()
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: control.horizontalPadding
        anchors.rightMargin: control.horizontalPadding
        spacing: FishUI.Units.largeSpacing

        Label {
            id: _label
            Layout.fillWidth: true
            elide: Text.ElideRight
        }

        Image {
            width: 16
            height: 16
            sourceSize: Qt.size(width, height)
            Layout.preferredWidth: width
            Layout.preferredHeight: height
            source: FishUI.Theme.darkMode ? "qrc:/images/dark/go-next.svg"
                                          : "qrc:/images/light/go-next.svg"
            mirror: control.LayoutMirroring.enabled
            smooth: false
            antialiasing: true
            opacity: 0.6
        }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: control.horizontalPadding
        anchors.rightMargin: control.horizontalPadding
        height: 1
        color: FishUI.Theme.disabledTextColor
        opacity: FishUI.Theme.darkMode ? 0.3 : 0.1
        visible: !control.lastItem
    }
}
