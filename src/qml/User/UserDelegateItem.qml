// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import QtCore
import QtQuick.Dialogs

import Cutefish.Settings
import Cutefish.Accounts
import FishUI as FishUI

import "../"

RoundedItem {
    id: control

    height: mainLayout.implicitHeight + Metrics.cardVerticalPadding * 2

    UserAccount {
        id: currentUser
        userId: model.userId
    }

    FileDialog {
        id: fileDialog
        currentFolder: StandardPaths.standardLocations(StandardPaths.PicturesLocation)[0]
        nameFilters: [qsTr("Image files (*.jpg *.jpeg *.png *.webp *.bmp *.tif *.tiff *.avif)"), qsTr("All files (*)")]
        onAccepted: {
            currentUser.iconFileName = decodeURIComponent(fileDialog.selectedFile.toString().replace("file://", ""))
            _userImage.source = fileDialog.selectedFile
            _userImage.update()
        }
    }

    ChangePasswordDialog {
        id: changePasswordDialog
        userName: currentUser.userName
        onPasswordSubmitted: function(password) {
            currentUser.setPassword(Password.cryptPassword(password))
        }
    }

    Component {
        id: deleteUserDialogComponent

        FishUI.ConfirmDialog {
            property var targetUserId
            property string targetUserName

            title: qsTr("Delete this user")
            text: qsTr("Delete user “%1”? Their home directory and its contents will be permanently deleted. This cannot be undone.").arg(targetUserName)
            cancelText: qsTr("Cancel")
            confirmText: qsTr("Delete")
            destructive: true
            confirmEnabled: targetUserId !== loggedUser.userId
            onAccepted: {
                if (targetUserId !== loggedUser.userId)
                    accountsManager.deleteUser(targetUserId, true)
            }
            onClosing: destroy()
        }
    }

    ColumnLayout {
        id: mainLayout
        spacing: 0

        RowLayout {
            id: _itemLayout
            spacing: 0

            Item {
                id: _topItem

                Layout.fillWidth: true
                height: _topLayout.implicitHeight + Metrics.m

                MouseArea {
                    anchors.fill: parent
                    onDoubleClicked: additionalSettings.toggle()
                }

                RowLayout {
                    id: _topLayout
                    anchors.fill: parent
                    spacing: Metrics.rowSpacing

                    Item {
                        Layout.preferredWidth: 48
                        Layout.preferredHeight: 48
                        Layout.alignment: Qt.AlignVCenter

                        // The account's initial, as on the lock screen, for an account with
                        // no picture: the icon theme has no avatar the size of a picture.
                        FishUI.RoundedRectangle {
                            anchors.fill: parent
                            radius: width / 2
                            continuousCorners: false
                            visible: _userImage.status !== Image.Ready
                            color: Qt.rgba(FishUI.Theme.textColor.r, FishUI.Theme.textColor.g,
                                           FishUI.Theme.textColor.b, 0.12)

                            FishUI.Label {
                                anchors.centerIn: parent
                                text: userName.length > 0 ? userName.charAt(0).toUpperCase() : "?"
                                font.pixelSize: 22
                                color: FishUI.Theme.disabledTextColor
                            }
                        }

                        Image {
                            id: _userImage

                            anchors.fill: parent
                            source: iconFileName ? "file:///" + iconFileName : ""
                            visible: status === Image.Ready

                            layer.enabled: true
                            layer.effect: FishUI.OpacityMask {
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

                        MouseArea {
                            anchors.fill: parent
                            onClicked: fileDialog.open()
                            cursorShape: Qt.PointingHandCursor
                        }
                    }

                    FishUI.Label {
                        Layout.alignment: Qt.AlignVCenter
                        text: userName
                        textFormat: Text.PlainText
                    }

                    FishUI.Label {
                        Layout.alignment: Qt.AlignVCenter
                        text: realName
                        color: FishUI.Theme.disabledTextColor
                        visible: realName !== userName
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    FishUI.Label {
                        text: qsTr("Logged in")
                        Layout.alignment: Qt.AlignVCenter
                        // As close to the arrow as a row choice's value is to its arrow.
                        Layout.rightMargin: FishUI.Units.mediumSpacing - Metrics.rowSpacing - _disclosure.iconMargins
                        color: FishUI.Theme.disabledTextColor
                        visible: currentUser.userId === loggedUser.userId
                    }

                    FishUI.RoundImageButton {
                        id: _disclosure
                        Layout.rightMargin: Metrics.trailingGlyphMargin(iconName, iconSize, iconMargins)
                        // A navigation row's chevron size; the hover circle stays short of the card's edge.
                        size: 26
                        iconMargins: 5
                        iconName: additionalSettings.shown ? "chevron-up" : "chevron-down"
                        onClicked: additionalSettings.toggle()
                        Layout.alignment: Qt.AlignVCenter
                    }
                }
            }
        }

        Hideable {
            id: additionalSettings
            spacing: Metrics.cardSpacing

            GridLayout {
                Layout.fillWidth: true
                Layout.topMargin: Metrics.cardSpacing
                columnSpacing: Metrics.formColumnSpacing
                rowSpacing: Metrics.formRowSpacing
                columns: 2

                FishUI.Label {
                    text: qsTr("Account type")
                }

                FishUI.Label {
                    text: currentUser.accountType === 0 ? qsTr("Standard")
                                                        : qsTr("Administrator")
                }
            }

            StandardButton {
                text: qsTr("Change password")
                onClicked: changePasswordDialog.show()
                backgroundColor: FishUI.Theme.darkMode ? "#363636" : FishUI.Theme.backgroundColor
                Layout.fillWidth: true
            }

            StandardButton {
                text: qsTr("Delete this user")
                enabled: model.userId !== loggedUser.userId
                onClicked: deleteUserDialogComponent.createObject(control, {
                    "parentWindow": control.Window.window,
                    "targetUserId": currentUser.userId,
                    "targetUserName": currentUser.userName
                })
                backgroundColor: FishUI.Theme.darkMode ? "#363636" : FishUI.Theme.backgroundColor
                Layout.fillWidth: true
            }
        }
    }
}
