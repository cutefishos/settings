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
