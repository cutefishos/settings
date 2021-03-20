import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import MeuiKit 1.0 as Meui
import Cutefish.Settings 1.0
import Cutefish.Accounts 1.0

Dialog {
    id: control

    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    modal: true
    padding: Meui.Units.largeSpacing * 2

    onRejected: clear()

    AccountsManager {
        id: manager

        onUserAdded: {
            if (account.userName === userNameField.text) {
                account.passwordMode = UserAccount.RegularPasswordMode;
                account.setPassword(Password.cryptPassword(passwordField.text));

                control.clear()
            }
        }
    }

    onVisibleChanged: {
        if (visible)
            userNameField.forceActiveFocus()
    }

    function clear() {
        userNameField.clear()
        passwordField.clear()
        verifyPasswordField.clear()
        accountTypeCombo.currentIndex = 0
    }

    footer: DialogButtonBox {
        padding: Meui.Units.largeSpacing * 2

        Button {
            id: cancelButton
            text: qsTr("Cancel")
            DialogButtonBox.buttonRole: DialogButtonBox.Cancel
            onClicked: control.reject()
        }

        Button {
            id: addButton
            text: qsTr("Add")
            enabled: userNameField.text != "" &&
                     passwordField.text != "" &&
                     passwordField.text == verifyPasswordField.text
            DialogButtonBox.buttonRole: DialogButtonBox.Ok
            flat: true
            onClicked: {
                if (manager.createUser(userNameField.text, "", accountTypeCombo.currentIndex))
                    control.accept()
            }
        }
    }

    ColumnLayout {
        GridLayout {
            columns: 2
            columnSpacing: Meui.Units.largeSpacing
            rowSpacing: Meui.Units.smallSpacing

            Label {
                text: qsTr("User name")
            }

            TextField {
                id: userNameField
                placeholderText: qsTr("User name")
                Layout.fillWidth: true
                selectByMouse: true
            }

            Label {
                text: qsTr("Password")
            }

            TextField {
                id: passwordField
                placeholderText: qsTr("Password")
                echoMode: TextField.Password
                Layout.fillWidth: true
                selectByMouse: true
            }

            Label {
                text: qsTr("Verify password")
            }

            TextField {
                id: verifyPasswordField
                placeholderText: qsTr("Verify password")
                echoMode: TextField.Password
                Layout.fillWidth: true
                selectByMouse: true
            }

            Label {
                text: qsTr("Account type")
            }

            ComboBox {
                id: accountTypeCombo
                model: [qsTr("Standard"), qsTr("Administrator")]
                Layout.fillWidth: true
            }
        }
    }
}
