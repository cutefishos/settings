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
import QtGraphicalEffects 1.0

import FishUI 1.0 as FishUI
import Cutefish.Settings 1.0
import Cutefish.Accounts 1.0
import "../"

ItemPage {
    headerTitle: qsTr("User")

    UserAccount {
        id: loggedUser
    }

    UsersModel {
        id: userModel
    }

    AccountsManager {
        id: accountsManager
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent
            spacing: FishUI.Units.largeSpacing * 2

            RoundedItem {
                visible: _userView.count > 0

                ListView {
                    id: _userView
                    model: userModel
                    Layout.fillWidth: true
                    spacing: FishUI.Units.largeSpacing * 2
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
            }

            StandardButton {
                id: _addUserButton
                text: qsTr("Add user")
                Layout.fillWidth: true
                onClicked: {
                    var component = Qt.createComponent("AddUserDialog.qml")
                    if (component.status === Component.Ready) {
                        var dialog = component.createObject(rootWindow)
                        dialog.open()
                    }
                }
            }
        }
    }
}
