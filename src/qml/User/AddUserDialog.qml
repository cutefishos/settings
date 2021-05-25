/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     revenmartin <revenmartin@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import FishUI 1.0 as FishUI
import Cutefish.Settings 1.0
import Cutefish.Accounts 1.0
import "../"

Dialog {
    id: control

    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    modal: true

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

    contentItem: ColumnLayout {
        id: _mainLayout
        spacing: FishUI.Units.largeSpacing * 1.5

        GridLayout {
            columns: 2
            columnSpacing: FishUI.Units.largeSpacing * 2
            rowSpacing: FishUI.Units.smallSpacing * 2

            Label {
                text: qsTr("User name")
                Layout.alignment: Qt.AlignRight
            }

            TextField {
                id: userNameField
                placeholderText: qsTr("User name")
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignRight
                selectByMouse: true
            }

            Label {
                text: qsTr("Password")
                Layout.alignment: Qt.AlignRight
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
                Layout.alignment: Qt.AlignRight
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
                Layout.alignment: Qt.AlignRight
            }

            ComboBox {
                id: accountTypeCombo
                model: [qsTr("Standard"), qsTr("Administrator")]
                Layout.fillWidth: true
                topInset: 0
                bottomInset: 0
            }
        }

        RowLayout {
            id: footerLayout
            spacing: FishUI.Theme.hugeRadius / 2

            Button {
                id: cancelButton
                text: qsTr("Cancel")
                onClicked: control.reject()
                Layout.fillWidth: true
            }

            Button {
                id: addButton
                text: qsTr("Add")
                enabled: userNameField.text != "" &&
                         passwordField.text != "" &&
                         passwordField.text == verifyPasswordField.text
                Layout.fillWidth: true
                flat: true
                onClicked: {
                    if (manager.createUser(userNameField.text, "", accountTypeCombo.currentIndex))
                        control.accept()
                }
            }
        }
    }
}
