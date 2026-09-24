import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import Cutefish.Settings 1.0
import FishUI 1.0 as FishUI

FishUI.Window {
    id: control

    windowButtonsTopMargin: Metrics.dialogWindowButtonsMargin
    windowButtonsRightMargin: Metrics.dialogWindowButtonsMargin
    header.height: 44
    contentTopMargin: 0

    width: contentWidth
    height: contentHeight
    minimumWidth: contentWidth
    minimumHeight: contentHeight
    maximumWidth: contentWidth
    maximumHeight: contentHeight

    property int contentWidth: 400
    property int contentHeight: mainLayout.implicitHeight + 32
    property var accountManager
    property string pendingUserName
    property string pendingPassword
    property bool waitingForUser: false
    property string errorMessage: ""

    title: qsTr("Add new user")
    visible: false
    minimizeButtonVisible: false
    modality: Qt.WindowModal

    background.color: FishUI.Theme.secondBackgroundColor
    flags: Qt.Dialog | Qt.FramelessWindowHint

    onClosing: function(close) { close.accepted = !waitingForUser }

    onVisibleChanged: {
        if (visible) {
            errorMessage = ""
            _nameTextField.forceActiveFocus()
        } else {
            _nameTextField.clear()
            _passwordTextField.clear()
            _verifyPasswordTextField.clear()
            accountTypeCombo.currentIndex = 0
        }
    }

    function finishUserCreation() {
        waitingForUser = false
        pendingUserName = ""
        pendingPassword = ""
    }

    ColumnLayout {
        id: mainLayout
        anchors.fill: parent
        anchors.margins: Metrics.dialogPadding
        spacing: Metrics.dialogSpacing

        FishUI.Label {
            Layout.fillWidth: true
            Layout.rightMargin: Metrics.xxl
            text: control.title
            textFormat: Text.PlainText
            font.pixelSize: 14
            font.weight: Font.DemiBold
            color: FishUI.Theme.textColor
            wrapMode: Text.Wrap
        }

        GridLayout {
            enabled: !control.waitingForUser
            Layout.fillWidth: true
            columns: 2
            columnSpacing: Metrics.dialogSpacing
            rowSpacing: Metrics.formRowSpacing

            FishUI.Label {
                text: qsTr("User name")
            }

            FishUI.TextField {
                id: _nameTextField
                placeholderText: qsTr("User name")
                selectByMouse: true
                Layout.fillWidth: true
                Keys.onEscapePressed: control.close()
            }

            FishUI.Label {
                text: qsTr("Password")
            }

            FishUI.TextField {
                id: _passwordTextField
                placeholderText: qsTr("Password")
                echoMode: TextInput.Password
                selectByMouse: true
                Layout.fillWidth: true
                Keys.onEscapePressed: control.close()
            }

            FishUI.Label {
                text: qsTr("Verify password")
            }

            FishUI.TextField {
                id: _verifyPasswordTextField
                placeholderText: qsTr("Verify password")
                echoMode: TextInput.Password
                selectByMouse: true
                Layout.fillWidth: true
                Keys.onEscapePressed: control.close()
            }

            FishUI.Label {
                text: qsTr("Account type")
            }

            FishUI.ComboBox {
                id: accountTypeCombo
                model: [qsTr("Standard"), qsTr("Administrator")]
                Layout.fillWidth: true
            }
        }

        FishUI.Label {
            Layout.fillWidth: true
            visible: control.errorMessage !== ""
            text: control.errorMessage
            textFormat: Text.PlainText
            wrapMode: Text.Wrap
        }

        RowLayout {
            spacing: Metrics.dialogButtonSpacing
            Layout.alignment: Qt.AlignRight

            FishUI.ConfirmDialogButton {
                text: qsTr("Cancel")
                enabled: !control.waitingForUser
                onClicked: control.close()
            }

            FishUI.ConfirmDialogButton {
                text: qsTr("Add")
                primary: true
                enabled: !control.waitingForUser && _nameTextField.text !== ""
                         && _passwordTextField.text !== ""
                         && _passwordTextField.text === _verifyPasswordTextField.text
                onClicked: {
                    control.errorMessage = ""
                    control.pendingUserName = _nameTextField.text
                    control.pendingPassword = _passwordTextField.text
                    control.waitingForUser = true
                    control.accountManager.createUser(control.pendingUserName,
                                                      "",
                                                      accountTypeCombo.currentIndex)
                }
            }
        }
    }
}
