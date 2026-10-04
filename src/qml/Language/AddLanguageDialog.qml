import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cutefish.Settings
import FishUI as FishUI
import "../"

FishUI.Window {
    id: addDialog

    required property var language

    title: qsTr("Add Language")
    visible: false
    modality: Qt.WindowModal
    flags: Qt.Dialog | Qt.FramelessWindowHint
    minimizeButtonVisible: false
    width: 460
    height: 560
    minimumWidth: 460
    maximumWidth: 460
    minimumHeight: 560
    maximumHeight: 560
    header.height: 44
    contentTopMargin: 0
    windowButtonsTopMargin: Metrics.dialogWindowButtonsMargin
    windowButtonsRightMargin: Metrics.dialogWindowButtonsMargin
    // The page colour, so the card stands out as it does on a page.
    background.color: FishUI.Theme.backgroundColor

    property string selectedCode: ""
    property bool adding: false

    onVisibleChanged: {
        if (visible) {
            selectedCode = ""
            searchField.clear()
            addDialog.language.search("")
            addDialog.language.loadLanguages()
            availableList.contentY = 0
            searchField.forceActiveFocus()
        }
    }
    onClosing: function(close) { close.accepted = !adding }

    Connections {
        target: addDialog.language
        function onSavingChanged() {
            if (addDialog.adding && !addDialog.language.saving) {
                addDialog.adding = false
                if (addDialog.language.errorMessage.length === 0)
                    addDialog.close()
            }
        }
    }

    Shortcut {
        sequence: "Escape"
        enabled: addDialog.visible && !addDialog.adding
        onActivated: addDialog.close()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Metrics.dialogPadding
        spacing: Metrics.dialogSpacing

        FishUI.Label {
            Layout.fillWidth: true
            Layout.rightMargin: Metrics.xxl
            text: addDialog.title
            textFormat: Text.PlainText
            font.pixelSize: 14
            font.weight: Font.DemiBold
            color: FishUI.Theme.textColor
            wrapMode: Text.Wrap
        }

        SearchField {
            id: searchField
            placeholderText: qsTr("Search languages")
            enabled: !addDialog.adding
            onTextChanged: {
                addDialog.selectedCode = ""
                searchTimer.restart()
            }
        }

        Timer {
            id: searchTimer
            interval: 100
            onTriggered: {
                addDialog.language.search(searchField.text)
                availableList.contentY = 0
            }
        }

        ListCard {
            Layout.fillHeight: true

            Flickable {
                id: availableList
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                contentWidth: width
                contentHeight: rows.implicitHeight
                enabled: !addDialog.adding
                boundsBehavior: Flickable.StopAtBounds
                FishUI.ScrollBar.vertical: FishUI.ScrollBar {}
                FishUI.WheelHandler { target: availableList }

                ColumnLayout {
                    id: rows
                    width: availableList.width
                    spacing: 0

                    Repeater {
                        id: repeater
                        model: addDialog.language.availableLanguages

                        delegate: LanguageRow {
                            required property string code
                            required property string name
                            required description
                            languageName: name
                            checked: addDialog.selectedCode === code
                            onClicked: addDialog.selectedCode = code
                        }
                    }
                }

                FishUI.Label {
                    // Declared children of a Flickable would otherwise scroll with its content.
                    parent: availableList
                    anchors.centerIn: parent
                    visible: repeater.count === 0
                    text: addDialog.language.loading ? qsTr("Loading languages…") : qsTr("No languages found")
                    color: FishUI.Theme.disabledTextColor
                }
            }
        }

        FishUI.Label {
            Layout.fillWidth: true
            visible: addDialog.language.errorMessage.length > 0
            text: addDialog.language.errorMessage
            wrapMode: Text.WordWrap
        }

        RowLayout {
            Layout.alignment: Qt.AlignRight
            spacing: Metrics.dialogButtonSpacing

            FishUI.ConfirmDialogButton {
                text: qsTr("Cancel")
                enabled: !addDialog.adding
                onClicked: addDialog.close()
            }

            FishUI.ConfirmDialogButton {
                text: qsTr("Add")
                primary: true
                enabled: addDialog.selectedCode.length > 0 && !addDialog.language.saving
                onClicked: {
                    addDialog.adding = true
                    addDialog.language.addLanguage(addDialog.selectedCode)
                }
            }
        }
    }
}
