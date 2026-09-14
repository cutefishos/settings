import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import FishUI 1.0 as FishUI

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
        ScrollBar.vertical: ScrollBar {}
        FishUI.WheelHandler { target: list }

        delegate: Item {
            id: row
            required property var modelData
            required property int index
            width: list.width
            height: control.rowHeight
            opacity: control.dragging && control.pressedIndex === index ? 0.45 : 1

            RowLayout {
                anchors.fill: parent
                spacing: 12

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2

                    Label {
                        Layout.fillWidth: true
                        text: row.modelData.name
                        textFormat: Text.PlainText
                        elide: Text.ElideRight
                        color: FishUI.Theme.textColor
                        font.pixelSize: 14
                    }

                    Label {
                        Layout.fillWidth: true
                        text: row.modelData.description
                        textFormat: Text.PlainText
                        elide: Text.ElideRight
                        color: FishUI.Theme.disabledTextColor
                        font.pixelSize: 13
                    }
                }

                Item {
                    id: handle
                    objectName: "languageDragHandle_" + row.index
                    Layout.preferredWidth: 32
                    Layout.preferredHeight: 32
                    activeFocusOnTab: true
                    Accessible.role: Accessible.Button
                    Accessible.name: qsTr("Reorder %1").arg(row.modelData.name)
                    Keys.onUpPressed: control.language.moveLanguage(row.index, row.index - 1)
                    Keys.onDownPressed: control.language.moveLanguage(row.index, row.index + 1)
                    ReorderHandle {
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

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.RightButton
                onClicked: actions.popup()
            }

            FishUI.DesktopMenu {
                id: actions

                FishUI.MenuItem {
                    text: qsTr("Remove Language")
                    enabled: list.count > 1
                    onTriggered: control.language.removeLanguage(row.index)
                }
            }

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: 1
                visible: row.index < list.count - 1
                color: FishUI.Theme.disabledTextColor
                opacity: FishUI.Theme.darkMode ? 0.3 : 0.1
            }
        }
    }

    Rectangle {
        z: 2
        visible: control.dragging && control.inside && control.targetIndex !== control.pressedIndex
        x: 3
        y: (control.targetIndex > control.pressedIndex ? control.targetIndex + 1 : control.targetIndex)
           * control.rowHeight - list.contentY
        width: control.width - x
        height: 2
        radius: 1
        color: FishUI.Theme.highlightColor

        Rectangle {
            x: -3
            anchors.verticalCenter: parent.verticalCenter
            width: 6
            height: 6
            radius: 3
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
            Rectangle {
                width: 14
                height: 1
                radius: 0.5
                color: FishUI.Theme.disabledTextColor
            }
        }
    }
}
