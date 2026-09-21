import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import FishUI 1.0 as FishUI

// One row of a ListCard. The hover background fills the whole row and only
// rounds the corners it shares with the card, which the row
// finds out by itself from its visible siblings; a separator runs between
// rows. Content goes into the row's layout after the label; `leading` before
// it; `details` is a panel that unfolds below the row while `expanded`.
Item {
    id: control

    readonly property bool isListRow: true

    property string text
    // Exposed for accessibility only; the row itself shows selection through
    // whatever the content puts in it, such as a check mark.
    property bool checked: false
    property bool firstItem: true
    property bool lastItem: true
    default property alias content: _row.data
    // Goes before the label: an icon, a colour swatch.
    property alias leading: _leading.data
    property alias contentSpacing: _row.spacing
    property alias details: _details.content
    property alias detailsSpacing: _details.spacing
    property bool expanded: false
    // A row that only holds a control, such as a switch, neither highlights
    // nor reacts to clicks.
    property bool interactive: true
    signal clicked()

    readonly property real rowHeight: Math.max(40, _row.implicitHeight + FishUI.Units.largeSpacing)

    readonly property real horizontalPadding: FishUI.Units.largeSpacing * 1.5
    readonly property color hoverColor: Qt.rgba(FishUI.Theme.textColor.r, FishUI.Theme.textColor.g,
                                                FishUI.Theme.textColor.b,
                                                // Light cards are white, so 0.05 would match the page background.
                                                _mouseArea.pressed ? (FishUI.Theme.darkMode ? 0.05 : 0.15)
                                                                   : 0.1)

    Layout.fillWidth: true
    implicitHeight: rowHeight + _details.implicitHeight
    activeFocusOnTab: interactive

    Accessible.role: Accessible.Button
    Accessible.name: text
    Accessible.onPressAction: clicked()

    Keys.onSpacePressed: clicked()
    Keys.onReturnPressed: clicked()
    Keys.onEnterPressed: clicked()

    function updateEdges() {
        if (!parent)
            return
        var rows = []
        var siblings = parent.children
        for (var i = 0; i < siblings.length; ++i) {
            if (siblings[i].isListRow && siblings[i].visible)
                rows.push(siblings[i])
        }
        firstItem = rows.length === 0 || rows[0] === control
        lastItem = rows.length === 0 || rows[rows.length - 1] === control
    }

    // A row appearing or disappearing changes its neighbours' corners too.
    function refreshSiblings() {
        if (!parent)
            return
        var siblings = parent.children
        for (var i = 0; i < siblings.length; ++i) {
            if (siblings[i].isListRow)
                siblings[i].updateEdges()
        }
    }

    onParentChanged: refreshSiblings()
    onVisibleChanged: refreshSiblings()
    Component.onCompleted: refreshSiblings()

    Connections {
        target: control.parent
        function onChildrenChanged() { control.refreshSiblings() }
    }

    // One shape only: overlapping translucent pieces would darken where they meet.
    Item {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: control.rowHeight
        clip: true
        visible: control.interactive && _mouseArea.containsMouse

        FishUI.RoundedRectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            // Push the corners that meet a neighbouring row, or the row's own
            // unfolded details, outside the clip.
            anchors.topMargin: control.firstItem ? 0 : -radius
            anchors.bottomMargin: control.lastItem && !control.expanded ? 0 : -radius
            radius: FishUI.Theme.bigRadius
            color: control.hoverColor
        }
    }

    MouseArea {
        id: _mouseArea
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: control.rowHeight
        enabled: control.interactive
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton
        onClicked: {
            control.forceActiveFocus()
            control.clicked()
        }
    }

    RowLayout {
        id: _row
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: control.horizontalPadding
        anchors.rightMargin: control.horizontalPadding
        height: control.rowHeight
        spacing: FishUI.Units.largeSpacing

        Row {
            id: _leading
            visible: children.length > 0
            Layout.alignment: Qt.AlignVCenter
        }

        Label {
            Layout.fillWidth: true
            visible: control.text.length > 0
            text: control.text
            textFormat: Text.PlainText
            color: FishUI.Theme.textColor
            elide: Text.ElideRight
        }
    }

    Hideable {
        id: _details
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: _row.bottom
        anchors.leftMargin: control.horizontalPadding
        anchors.rightMargin: control.horizontalPadding
        height: implicitHeight
        shown: control.expanded
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
