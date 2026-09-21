import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FishUI 1.0 as FishUI

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
    windowButtonsTopMargin: 8
    windowButtonsRightMargin: 8
    background.color: FishUI.Theme.secondBackgroundColor

    property string selectedCode: ""
    property bool adding: false

    onVisibleChanged: {
        if (visible) {
            selectedCode = ""
            searchField.clear()
            addDialog.language.search("")
            addDialog.language.loadLanguages()
            availableList.positionViewAtBeginning()
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
        anchors.margins: 16
        spacing: 16

        Label {
            Layout.fillWidth: true
            Layout.rightMargin: 32
            text: addDialog.title
            textFormat: Text.PlainText
            font.pixelSize: 14
            font.weight: Font.DemiBold
            color: FishUI.Theme.textColor
            wrapMode: Text.Wrap
        }

        TextField {
            id: searchField
            Layout.fillWidth: true
            placeholderText: qsTr("Search languages")
            selectByMouse: true
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
                availableList.positionViewAtBeginning()
            }
        }

        ListView {
            id: availableList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: addDialog.language.availableLanguages
            reuseItems: true
            cacheBuffer: 120
            enabled: !addDialog.adding
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: ScrollBar {}
            FishUI.WheelHandler { target: availableList }

            delegate: LanguageRow {
                required property string code
                required property string name
                required description
                required property int index
                width: availableList.width
                selected: addDialog.selectedCode === code
                languageName: name
                separatorVisible: index < availableList.count - 1
                showCheck: selected
                onClicked: addDialog.selectedCode = code
            }

            Label {
                anchors.centerIn: parent
                visible: availableList.count === 0
                text: addDialog.language.loading ? qsTr("Loading languages…") : qsTr("No languages found")
                color: FishUI.Theme.disabledTextColor
            }
        }

        Label {
            Layout.fillWidth: true
            visible: addDialog.language.errorMessage.length > 0
            text: addDialog.language.errorMessage
            wrapMode: Text.WordWrap
        }

        RowLayout {
            Layout.alignment: Qt.AlignRight
            spacing: 8

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
