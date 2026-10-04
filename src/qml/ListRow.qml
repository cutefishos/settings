import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cutefish.Settings
import FishUI as FishUI

// One row of a ListCard. The press background fills the whole row and only
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
    property bool separatorVisible: true
    // Lifts the label by half this, onto a control with captions hanging below it.
    property real labelBottomMargin: 0
    readonly property bool highlighted: interactive && enabled && _mouseArea.pressed
    signal clicked()

    // Set by a ListView delegate to its index: a view's rows are not siblings
    // in row order, so their edges come from the view instead.
    property int viewIndex: -1

    // The next visible row, whose highlight hides this row's separator.
    property Item _nextRow: null

    property real minimumRowHeight: Metrics.rowHeight
    readonly property real rowHeight: Math.max(minimumRowHeight, _row.implicitHeight + Metrics.cardVerticalPadding)

    readonly property real horizontalPadding: Metrics.cardHorizontalPadding
    readonly property color pressedColor: Qt.rgba(FishUI.Theme.textColor.r, FishUI.Theme.textColor.g,
                                                  FishUI.Theme.textColor.b, FishUI.Theme.darkMode ? 0.05 : 0.15)

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
        var view = control.ListView.view
        if (viewIndex >= 0 && view) {
            firstItem = viewIndex === 0
            lastItem = viewIndex === view.count - 1
            _nextRow = view.itemAtIndex(viewIndex + 1)
            return
        }
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
        var index = rows.indexOf(control)
        _nextRow = index >= 0 && index + 1 < rows.length ? rows[index + 1] : null
    }

    // A row appearing or disappearing changes its neighbours' corners too.
    function refreshSiblings() {
        if (viewIndex >= 0) {
            updateEdges()
            return
        }
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
    onViewIndexChanged: updateEdges()
    Component.onCompleted: refreshSiblings()

    // Every row gets this signal, so each updates only itself: refreshing all
    // siblings from each makes a Repeater of n rows cost n^4.
    Connections {
        target: control.parent
        function onChildrenChanged() { control.updateEdges() }
    }

    Connections {
        target: control.viewIndex >= 0 ? control.ListView.view : null
        function onCountChanged() { control.updateEdges() }
        // Scrolling creates or reuses the delegate for the next row.
        function onContentYChanged() { control.updateEdges() }
    }

    // One shape only: overlapping translucent pieces would darken where they meet.
    Item {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: control.rowHeight
        clip: true
        visible: control.highlighted

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
            color: control.pressedColor
        }
    }

    MouseArea {
        id: _mouseArea
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: control.rowHeight
        enabled: control.interactive
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
        spacing: Metrics.rowSpacing

        Row {
            id: _leading
            visible: children.length > 0
            Layout.alignment: Qt.AlignVCenter
        }

        FishUI.Label {
            Layout.fillWidth: true
            Layout.bottomMargin: control.labelBottomMargin
            visible: control.text.length > 0
            text: control.text
            textFormat: Text.PlainText
            color: control.enabled ? FishUI.Theme.textColor : FishUI.Theme.disabledTextColor
            // A window as narrow as half a screen wraps a long label beside its
            // control rather than cutting it.
            wrapMode: Text.WordWrap
            maximumLineCount: 2
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

    Separator {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: control.horizontalPadding
        anchors.rightMargin: control.horizontalPadding
        // A highlight hides the separators it touches, as the translucent fill
        // would otherwise show them through.
        visible: control.separatorVisible && !control.lastItem
                 && !(control.highlighted && _details.height === 0)
                 && !(control._nextRow && control._nextRow.highlighted)
    }
}
