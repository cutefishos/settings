import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import FishUI 1.0 as FishUI

FishUI.Window {
    id: control

    property string userName
    signal passwordSubmitted(string password)

    title: qsTranslate("UserDelegateItem", "Change password")
    visible: false
    modality: Qt.WindowModal
    flags: Qt.Dialog | Qt.FramelessWindowHint
    minimizeButtonVisible: false
    windowButtonsTopMargin: 8
    windowButtonsRightMargin: 8
    header.height: 44
    contentTopMargin: 0
    background.color: FishUI.Theme.secondBackgroundColor

    width: 400
    height: mainLayout.implicitHeight + 32
    minimumWidth: 400
    maximumWidth: 400
    minimumHeight: mainLayout.implicitHeight + 32
    maximumHeight: mainLayout.implicitHeight + 32

    onVisibleChanged: {
        if (visible) {
            passwordField.forceActiveFocus()
        } else {
            passwordField.clear()
            verifyPasswordField.clear()
        }
    }

    Shortcut {
        sequence: "Escape"
        enabled: control.visible
        onActivated: control.close()
    }

    ColumnLayout {
        id: mainLayout
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16

        FishUI.Label {
            Layout.fillWidth: true
            Layout.rightMargin: 32
            text: control.title
            textFormat: Text.PlainText
            font.pixelSize: 14
            font.weight: Font.DemiBold
            wrapMode: Text.Wrap
        }

        FishUI.Label {
            Layout.fillWidth: true
            text: control.userName
            textFormat: Text.PlainText
            color: FishUI.Theme.disabledTextColor
            wrapMode: Text.Wrap
        }

        GridLayout {
            Layout.fillWidth: true
            columns: 2
            columnSpacing: 16
            rowSpacing: 16

            FishUI.Label {
                text: qsTranslate("UserDelegateItem", "Password")
            }

            FishUI.TextField {
                id: passwordField
                Layout.fillWidth: true
                placeholderText: qsTranslate("UserDelegateItem", "Password")
                echoMode: TextInput.Password
                selectByMouse: true
            }

            FishUI.Label {
                text: qsTranslate("UserDelegateItem", "Verify password")
            }

            FishUI.TextField {
                id: verifyPasswordField
                Layout.fillWidth: true
                placeholderText: qsTranslate("UserDelegateItem", "Verify password")
                echoMode: TextInput.Password
                selectByMouse: true
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignRight
            spacing: 8

            FishUI.ConfirmDialogButton {
                text: qsTranslate("UserDelegateItem", "Cancel")
                onClicked: control.close()
            }

            FishUI.ConfirmDialogButton {
                text: control.title
                primary: true
                enabled: passwordField.text !== ""
                         && passwordField.text === verifyPasswordField.text
                onClicked: {
                    control.passwordSubmitted(passwordField.text)
                    control.close()
                }
            }
        }
    }
}
