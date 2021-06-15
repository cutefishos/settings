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
import QtQuick.Dialogs 1.2
import QtGraphicalEffects 1.0

import Cutefish.Settings 1.0
import Cutefish.Accounts 1.0
import FishUI 1.0 as FishUI

import "../"

Item {
    id: control

    height: mainLayout.implicitHeight

    UserAccount {
        id: currentUser
        userId: model.userId
    }

    FileDialog {
        id: fileDialog
        folder: shortcuts.pictures
        nameFilters: ["Image files (*.jpg *.png)", "All files (*)"]
        onAccepted: {
            currentUser.iconFileName = fileDialog.fileUrl.toString().replace("file://", "")
            _userImage.source = fileDialog.fileUrl
            _userImage.update()
        }
    }

    ColumnLayout {
        id: mainLayout
        anchors.fill: parent
        spacing: 0

        RowLayout {
            id: _itemLayout
            spacing: 0

            Item {
                id: _topItem

                Layout.fillWidth: true
                height: _topLayout.implicitHeight

                MouseArea {
                    anchors.fill: parent
                    onDoubleClicked: additionalSettings.toggle()
                }

                RowLayout {
                    id: _topLayout
                    anchors.fill: parent

                    Image {
                        id: _userImage
                        width: 32
                        height: 32
                        sourceSize: Qt.size(width, height)
                        source: iconFileName ? "file:///" + iconFileName : "image://icontheme/default-user"
                        visible: status === Image.Ready
                        Layout.alignment: Qt.AlignVCenter

                        MouseArea {
                            anchors.fill: parent
                            onClicked: fileDialog.open()
                            cursorShape: Qt.PointingHandCursor
                        }

                        layer.enabled: true
                        layer.effect: OpacityMask {
                            maskSource: Item {
                                width: _userImage.width
                                height: width

                                Rectangle {
                                    anchors.fill: parent
                                    radius: width / 2
                                }
                            }
                        }
                    }

                    Label {
                        Layout.alignment: Qt.AlignVCenter
                        text: "<b>%1</b>".arg(userName)
                        leftPadding: FishUI.Units.largeSpacing
                    }

                    Label {
                        Layout.alignment: Qt.AlignVCenter
                        text: realName
                        color: FishUI.Theme.disabledTextColor
                        visible: realName !== userName
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Label {
                        text: qsTr("Currently logged")
                        Layout.alignment: Qt.AlignVCenter
                        visible: currentUser.userId === loggedUser.userId
                    }

                    Button {
                        Layout.alignment: Qt.AlignVCenter

                        onClicked: additionalSettings.toggle()

                        implicitWidth: height

                        background: Item {}

                        Image {
                            anchors.centerIn: parent
                            width: 22
                            height: 22
                            sourceSize: Qt.size(width, height)
                            source: FishUI.Theme.darkMode ? additionalSettings.shown ? "qrc:/images/dark/up.svg" : "qrc:/images/dark/down.svg"
                                                        : additionalSettings.shown ? "qrc:/images/light/up.svg" : "qrc:/images/light/down.svg"
                        }
                    }
                }
            }
        }

        Hideable {
            id: additionalSettings

            Item {
                height: FishUI.Units.largeSpacing
            }

            GridLayout {
                Layout.fillWidth: true
                Layout.bottomMargin: FishUI.Units.smallSpacing
                rowSpacing: FishUI.Units.largeSpacing
                columns: 2

                Label {
                    text: qsTr("Automatic login")
                    Layout.fillWidth: true
                }

                Switch {
                    id: automaticLoginSwitch
                    Layout.fillHeight: true
                    leftPadding: 0
                    rightPadding: 0
                    onCheckedChanged: currentUser.automaticLogin = checked
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight

                    Component.onCompleted: {
                        automaticLoginSwitch.checked = currentUser.automaticLogin
                    }
                }
            }

            Button {
                text: qsTr("Delete this user")
                enabled: model.userId !== loggedUser.userId
                onClicked: accountsManager.deleteUser(userId, true)
                Layout.fillWidth: true
            }
        }
    }
}
