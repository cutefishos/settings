import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import FishUI as FishUI
import "../"

Item {
    id: control
    objectName: "preferredLanguageList"

    required property var language
    readonly property int rowHeight: 52
    implicitHeight: Math.min(rowHeight * 7, list.contentHeight)
    property int pressedIndex: -1
    property int targetIndex: -1
    property bool dragging: false
    property bool pointerHeld: false
    property real pointerX: 0
    property real pointerY: 0
    property real pressY: 0
    property real grabOffset: 0
    readonly property bool inside: pointerX >= 0 && pointerX <= width && pointerY >= 0 && pointerY <= height

    function press(index, point) {
        pointerHeld = true
        pressedIndex = index
        targetIndex = index
        pointerX = point.x
        pressY = pointerY = point.y
        grabOffset = point.y + list.contentY - index * rowHeight
    }

    function updatePointer(point) {
        pointerX = point.x
        pointerY = point.y
        if (!dragging && list.count > 1
                && Math.abs(pointerY - pressY) >= Qt.styleHints.startDragDistance) {
            dragging = true
        }
        if (dragging)
            updateTarget()
    }

    function updateTarget() {
        var center = pointerY - grabOffset + rowHeight / 2 + list.contentY
        targetIndex = Math.max(0, Math.min(list.count - 1, Math.floor(center / rowHeight)))
    }

    function cancel() {
        dragging = false
        pressedIndex = -1
        targetIndex = -1
    }

    function release() {
        pointerHeld = false
        var from = pressedIndex
        var to = targetIndex
        var move = dragging && inside && from !== to
        dragging = false
        pressedIndex = -1
        targetIndex = -1
        if (move)
            language.moveLanguage(from, to)
    }

    Connections {
        target: control.language
        function onPreferredLanguagesChanged() { control.cancel() }
    }

    Shortcut {
        sequence: "Escape"
        enabled: control.dragging
        onActivated: control.cancel()
    }

    ListView {
        id: list
        objectName: "preferredLanguageView"
        anchors.fill: parent
        model: control.language.preferredLanguages
        clip: true
        currentIndex: -1
        interactive: control.pressedIndex < 0
        enabled: !control.language.saving
        boundsBehavior: Flickable.StopAtBounds
        FishUI.ScrollBar.vertical: FishUI.ScrollBar {}
        FishUI.WheelHandler { target: list }

        delegate: Item {
            id: row
            required property var modelData
            required property int index
            width: list.width
            height: control.rowHeight
            opacity: control.dragging && control.pressedIndex === index ? 0.45 : 1

            // Below the row content: a MouseArea always sets a cursor, so one on top
            // would override the drag handle's.
            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.RightButton
                onClicked: actions.popup()
            }

            RowLayout {
                anchors.fill: parent
                spacing: 12

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        FishUI.Label {
                            id: primaryLabel
                            // Shrinks to elide but never grows, so the badge stays next to the
                            // name. Rounded up: a width a fraction short of the text elides it.
                            Layout.fillWidth: true
                            Layout.maximumWidth: Math.ceil(implicitWidth)
                            text: row.modelData.name
                            textFormat: Text.PlainText
                            elide: Text.ElideRight
                            color: FishUI.Theme.textColor
                        }

                        Rectangle {
                            visible: row.index === 0
                            Layout.alignment: Qt.AlignVCenter
                            implicitWidth: Math.ceil(primaryBadge.implicitWidth) + 16
                            implicitHeight: Math.round(inkMetrics.tightBoundingRect.height) + 8
                            radius: height / 2
                            color: Qt.alpha(FishUI.Theme.highlightColor, FishUI.Theme.darkMode ? 0.2 : 0.12)

                            TextMetrics {
                                id: inkMetrics
                                font: primaryBadge.font
                                text: primaryBadge.text
                            }

                            TextMetrics {
                                id: capMetrics
                                font: primaryBadge.font
                                text: "H"
                            }

                            FishUI.Label {
                                id: primaryBadge

                                // Baselines that centre all the ink and just the capitals: the
                                // first reads high once descenders count, the second low.
                                readonly property real inkBaseline: (parent.height - inkMetrics.tightBoundingRect.height) / 2
                                                                    - inkMetrics.tightBoundingRect.y
                                readonly property real capBaseline: (parent.height + capMetrics.tightBoundingRect.height) / 2

                                anchors.horizontalCenter: parent.horizontalCenter
                                y: Math.round((inkBaseline + capBaseline) / 2 - baselineOffset)
                                text: qsTr("System language")
                                color: FishUI.Theme.highlightColor
                                font.pixelSize: Math.max(1, primaryLabel.font.pixelSize - 2)
                                font.weight: Font.Medium
                            }
                        }

                        Item {
                            Layout.fillWidth: true
                        }
                    }

                    FishUI.Label {
                        Layout.fillWidth: true
                        text: row.modelData.description
                        textFormat: Text.PlainText
                        elide: Text.ElideRight
                        color: FishUI.Theme.disabledTextColor
                        font.pixelSize: Math.max(1, primaryLabel.font.pixelSize - 1)
                    }
                }

                Item {
                    id: handle
                    objectName: "languageDragHandle_" + row.index
                    Layout.preferredWidth: 32
                    Layout.preferredHeight: 32
                    // The lines, not the drag area around them, end where a switch does.
                    Layout.rightMargin: -(Layout.preferredWidth - reorderLines.width) / 2
                    activeFocusOnTab: true
                    Accessible.role: Accessible.Button
                    Accessible.name: qsTr("Reorder %1").arg(row.modelData.name)
                    Keys.onUpPressed: control.language.moveLanguage(row.index, row.index - 1)
                    Keys.onDownPressed: control.language.moveLanguage(row.index, row.index + 1)
                    ReorderHandle {
                        id: reorderLines
                        anchors.centerIn: parent
                    }

                    MouseArea {
                        id: mouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: control.pointerHeld ? Qt.ClosedHandCursor : Qt.OpenHandCursor
                        preventStealing: true
                        onPressed: function(event) {
                            handle.forceActiveFocus()
                            control.press(row.index, mouse.mapToItem(control, event.x, event.y))
                        }
                        onPositionChanged: function(event) {
                            if (pressed && control.pressedIndex >= 0)
                                control.updatePointer(mouse.mapToItem(control, event.x, event.y))
                        }
                        onReleased: control.release()
                        onCanceled: {
                            control.pointerHeld = false
                            control.cancel()
                        }
                    }
                }
            }

            FishUI.DesktopMenu {
                id: actions

                FishUI.MenuItem {
                    text: qsTr("Remove Language")
                    enabled: list.count > 1
                    onTriggered: control.language.removeLanguage(row.index)
                }
            }

            Separator {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                visible: row.index < list.count - 1
            }
        }
    }

    FishUI.RoundedRectangle {
        z: 2
        visible: control.dragging && control.inside && control.targetIndex !== control.pressedIndex
        x: 3
        y: (control.targetIndex > control.pressedIndex ? control.targetIndex + 1 : control.targetIndex)
           * control.rowHeight - list.contentY
        width: control.width - x
        height: 2
        radius: 1
        continuousCorners: false
        color: FishUI.Theme.highlightColor

        FishUI.RoundedRectangle {
            x: -3
            anchors.verticalCenter: parent.verticalCenter
            width: 6
            height: 6
            radius: 3
            continuousCorners: false
            color: FishUI.Theme.highlightColor
        }
    }

    MouseArea {
        parent: control.Window.window ? control.Window.window.contentItem : control
        anchors.fill: parent
        z: 10000
        visible: control.pointerHeld
        acceptedButtons: Qt.NoButton
        hoverEnabled: true
        cursorShape: Qt.ClosedHandCursor
    }

    Timer {
        interval: 16
        repeat: true
        running: control.dragging
        onTriggered: {
            if (!control.inside)
                return
            var edge = control.rowHeight
            var delta = control.pointerY < edge ? -8 * (1 - control.pointerY / edge)
                        : control.pointerY > control.height - edge
                          ? 8 * (1 - (control.height - control.pointerY) / edge) : 0
            if (delta !== 0) {
                list.contentY = Math.max(0, Math.min(Math.max(0, list.contentHeight - list.height), list.contentY + delta))
                control.updateTarget()
            }
        }
    }

    component ReorderHandle: Column {
        spacing: 3
        Repeater {
            model: 3
            // Snapped: at 150% a plain Rectangle drew these one or two pixels thick.
            FishUI.RoundedRectangle {
                width: 14
                height: 1
                radius: 0.5
                continuousCorners: false
                color: FishUI.Theme.disabledTextColor
            }
        }
    }
}
