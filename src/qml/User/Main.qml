// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import FishUI as FishUI
import Cutefish.Settings
import Cutefish.Accounts
import "../"

ItemPage {
    headerTitle: qsTr("Users")

    UserAccount {
        id: loggedUser
    }

    UsersModel {
        id: userModel
    }

    AccountsManager {
        id: accountsManager

        onUserCreated: function(account) {
            if (addUserDialog.waitingForUser) {
                account.passwordMode = UserAccount.RegularPasswordMode
                account.setPassword(Password.cryptPassword(addUserDialog.pendingPassword))
                addUserDialog.finishUserCreation()
                addUserDialog.close()
            }
        }

        onUserCreationFailed: function(message) {
            addUserDialog.finishUserCreation()
            addUserDialog.errorMessage = message
        }
    }

    AddUserDialog {
        id: addUserDialog
        accountManager: accountsManager
    }

    Scrollable {
        id: _scrollable
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent
            spacing: Metrics.sectionSpacing

            ListView {
                id: _userView
                model: userModel
                Layout.fillWidth: true
                spacing: Metrics.sectionSpacing
                interactive: false

                Layout.preferredHeight: {
                    var totalHeight = 0
                    for (var i = 0; i < _userView.visibleChildren.length; ++i) {
                        totalHeight += _userView.visibleChildren[i].height
                    }
                    return totalHeight
                }

                delegate: UserDelegateItem {
                    width: _userView.width
                }
            }

            StandardButton {
                id: _addUserButton
                text: qsTr("Add user")
                Layout.fillWidth: true
                onClicked: addUserDialog.show()
            }

        }
    }
}
