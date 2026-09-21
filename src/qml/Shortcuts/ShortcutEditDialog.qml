import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import FishUI 1.0 as FishUI
import Cutefish.Settings 1.0

// Edits the shortcuts of one action: remove any of them, add another by
// pressing it, or go back to the defaults. Adding is confirm-by-repeat, as in
// the ChromeOS Shortcuts app: a conflict or a warning is shown once, and
// pressing the same combination again applies it anyway.
FishUI.Window {
    id: dialog

    required property ShortcutsModel shortcutsModel
    required property string actionName

    property var accelerators: []
    property bool isDefault: true
    property bool recording: false
    // The shortcut being replaced while recording; empty when adding.
    property var replacing: null
    property var pendingParts: []
    property string message: ""
    property bool messageIsWarning: false
    // Meta went down alone and nothing else has been pressed since: releasing
    // it records the Meta tap, which is how the launcher key is bound.
    property bool metaTapCandidate: false

    title: shortcutsModel.descriptionFor(actionName)
    visible: false
    modality: Qt.WindowModal
    flags: Qt.Dialog | Qt.FramelessWindowHint
    minimizeButtonVisible: false
    windowButtonsTopMargin: 8
    windowButtonsRightMargin: 8
    header.height: 44
    contentTopMargin: 0
    background.color: FishUI.Theme.secondBackgroundColor

    width: 440
    height: mainLayout.implicitHeight + 32
    minimumWidth: 440
    maximumWidth: 440
    minimumHeight: mainLayout.implicitHeight + 32
    maximumHeight: mainLayout.implicitHeight + 32

    function toggleMaximized() {}

    Component.onCompleted: refresh()

    Connections {
        target: dialog.shortcutsModel
        function onAcceleratorsChanged() { dialog.refresh() }
    }

    onVisibleChanged: {
        if (!visible)
            stopRecording()
    }
    onClosing: stopRecording()

    function refresh() {
        var action = shortcutsModel.action(actionName)
        accelerators = action.accelerators !== undefined ? action.accelerators : []
        isDefault = action.isDefault !== undefined ? action.isDefault : true
    }

    function startRecording(target) {
        replacing = target
        pendingParts = []
        metaTapCandidate = false
        message = ""
        recording = true
        shortcutsModel.setRecording(true)
        recorder.forceActiveFocus()
    }

    function stopRecording() {
        if (!recording)
            return
        recording = false
        replacing = null
        pendingParts = []
        shortcutsModel.setRecording(false)
    }

    function handleKey(event) {
        event.accepted = true
        // Holding the key must not turn into the second press that confirms a
        // conflict or a warning.
        if (event.isAutoRepeat)
            return
        var modifiers = shortcutsModel.compositorModifiers(event.modifiers)
        if (shortcutsModel.isModifierKey(event.key)) {
            pendingParts = shortcutsModel.modifierParts(modifiers)
            metaTapCandidate = isMetaKey(event.key) && modifiers === ShortcutsModel.MetaDown
            return
        }
        metaTapCandidate = false
        if (event.key === Qt.Key_Escape && modifiers === 0) {
            stopRecording()
            return
        }
        var keycode = event.nativeScanCode > 8 ? event.nativeScanCode - 8 : 0
        submit(keycode, shortcutsModel.keysymForQtKey(event.key), modifiers,
               shortcutsModel.partsForKey(keycode, event.key, modifiers))
    }

    function isMetaKey(key) {
        return key === Qt.Key_Meta || key === Qt.Key_Super_L || key === Qt.Key_Super_R
    }

    function handleKeyRelease(event) {
        if (!metaTapCandidate || !isMetaKey(event.key))
            return
        event.accepted = true
        metaTapCandidate = false
        var keycode = event.nativeScanCode > 8 ? event.nativeScanCode - 8 : 0
        submit(keycode, shortcutsModel.keysymForQtKey(event.key), 0,
               shortcutsModel.modifierParts(ShortcutsModel.MetaDown))
    }

    function submit(keycode, keysym, modifiers, parts) {
        pendingParts = parts
        var result = replacing
                ? shortcutsModel.replaceAccelerator(actionName, replacing.keysym, replacing.modifiers,
                                               keycode, keysym, modifiers)
                : shortcutsModel.addAccelerator(actionName, keycode, keysym, modifiers)
        if (result.result === "success") {
            message = ""
            stopRecording()
            return
        }
        message = result.message
        messageIsWarning = result.result === "conflict-can-override"
                || result.result === "non-meta-accelerator-warning"
    }

    Shortcut {
        sequence: "Escape"
        enabled: dialog.visible && !dialog.recording
        onActivated: dialog.close()
    }

    ColumnLayout {
        id: mainLayout
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        Label {
            Layout.fillWidth: true
            Layout.rightMargin: 32
            text: dialog.title
            textFormat: Text.PlainText
            font.pixelSize: 14
            font.weight: Font.DemiBold
            color: FishUI.Theme.textColor
            wrapMode: Text.Wrap
        }

        Label {
            visible: dialog.accelerators.length === 0 && !dialog.recording
            text: qsTr("No shortcut. Add one below.")
            color: FishUI.Theme.disabledTextColor
        }

        Repeater {
            model: dialog.accelerators

            RowLayout {
                Layout.fillWidth: true
                spacing: FishUI.Units.largeSpacing
                readonly property bool beingReplaced: dialog.recording && dialog.replacing
                                                      && dialog.replacing.keysym === modelData.keysym
                                                      && dialog.replacing.modifiers === modelData.modifiers

                ShortcutKeys {
                    parts: modelData.parts
                    dimmed: parent.beingReplaced
                }

                Label {
                    visible: modelData.type === "user"
                    text: qsTr("Custom")
                    color: FishUI.Theme.disabledTextColor
                    font.pointSize: 8
                }

                Item { Layout.fillWidth: true }

                Label {
                    text: qsTr("Change")
                    color: changeArea.containsMouse ? FishUI.Theme.highlightColor : FishUI.Theme.textColor
                    visible: !dialog.recording && !modelData.locked

                    MouseArea {
                        id: changeArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: dialog.startRecording(modelData)
                    }
                }

                Label {
                    text: qsTr("Remove")
                    color: removeArea.containsMouse ? FishUI.Theme.highlightColor : FishUI.Theme.textColor
                    visible: !dialog.recording && !modelData.locked

                    MouseArea {
                        id: removeArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            var result = dialog.shortcutsModel.removeAccelerator(dialog.actionName,
                                                                                 modelData.keysym,
                                                                                 modelData.modifiers)
                            dialog.message = result.message
                            dialog.messageIsWarning = false
                        }
                    }
                }
            }
        }

        // The recorder: focused while recording, so every key lands here.
        FishUI.RoundedRectangle {
            id: recorder
            Layout.fillWidth: true
            implicitHeight: 48
            radius: FishUI.Theme.mediumRadius
            visible: dialog.recording
            color: Qt.rgba(FishUI.Theme.highlightColor.r, FishUI.Theme.highlightColor.g,
                           FishUI.Theme.highlightColor.b, 0.12)
            border.width: 1
            border.color: FishUI.Theme.highlightColor
            focus: dialog.recording

            Keys.onPressed: function(event) { dialog.handleKey(event) }
            Keys.onReleased: function(event) { dialog.handleKeyRelease(event) }
            onActiveFocusChanged: {
                if (!activeFocus && dialog.recording)
                    dialog.stopRecording()
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                spacing: FishUI.Units.largeSpacing

                Label {
                    visible: dialog.pendingParts.length === 0
                    text: dialog.replacing ? qsTr("Press the new shortcut…") : qsTr("Press the shortcut…")
                    color: FishUI.Theme.textColor
                }

                ShortcutKeys {
                    visible: dialog.pendingParts.length > 0
                    parts: dialog.pendingParts
                }

                Item { Layout.fillWidth: true }

                Label {
                    text: qsTr("Cancel")
                    color: cancelArea.containsMouse ? FishUI.Theme.highlightColor : FishUI.Theme.textColor

                    MouseArea {
                        id: cancelArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: dialog.stopRecording()
                    }
                }
            }
        }

        Label {
            Layout.fillWidth: true
            visible: dialog.message.length > 0
            text: dialog.message
            color: dialog.messageIsWarning ? FishUI.Theme.textColor : "#E24B4B"
            wrapMode: Text.WordWrap
            font.pointSize: 9
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: 4
            spacing: FishUI.Units.largeSpacing

            FishUI.ConfirmDialogButton {
                text: qsTr("Add shortcut")
                enabled: !dialog.recording && dialog.accelerators.length < 5
                onClicked: dialog.startRecording(null)
            }

            FishUI.ConfirmDialogButton {
                text: qsTr("Restore default")
                visible: !dialog.isDefault
                enabled: !dialog.recording
                onClicked: {
                    var result = dialog.shortcutsModel.restoreDefault(dialog.actionName)
                    dialog.message = result.message
                    dialog.messageIsWarning = true
                }
            }

            Item { Layout.fillWidth: true }
        }
    }
}
