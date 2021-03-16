import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.0
import Qt.labs.platform 1.0 as LabsPlatform

import MeuiKit 1.0 as Meui
import Cutefish.Settings 1.0
import Cutefish.Accounts 1.0

ItemPage {
    headerTitle: qsTr("User")

    UserAccount {
        id: currentUser
    }

    UsersModel {
        id: userModel
    }

    AccountsManager {
        id: accountsManager
    }

    AddUserDialog {
        id: addUserDialog
    }

    Scrollable {
        anchors.fill: parent
        anchors.bottomMargin: Meui.Units.largeSpacing
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent

            Label {
                text: qsTr("Currently logged in as")
                color: Meui.Theme.disabledTextColor
                bottomPadding: Meui.Units.largeSpacing
            }

            RowLayout {
                LabsPlatform.FileDialog {
                    id: currentUserFileDialog
                    folder: LabsPlatform.StandardPaths.writableLocation(LabsPlatform.StandardPaths.PicturesLocation)
                    nameFilters: ["Pictures (*.png *.jpg *.gif)"]
                    onFileChanged: {
                        currentUser.iconFileName = currentFile.toString().replace("file://", "")
                        currentUserImage.source = currentFile
                        currentUserImage.update()
                    }
                }

                Image {
                    id: currentUserImage
                    Layout.preferredWidth: 64
                    Layout.preferredHeight: 64
                    width: 64
                    height: width
                    sourceSize: Qt.size(width, height)
                    source: currentUser.iconFileName ? "file://" + currentUser.iconFileName : "image://icontheme/default-user"
                    asynchronous: true
                    fillMode: Image.PreserveAspectCrop
                    cache: false

                    property bool counter: false

                    MouseArea {
                        id: userImageMouseArea
                        anchors.fill: parent
                        acceptedButtons: Qt.LeftButton
                        onClicked: currentUserFileDialog.open()
                        cursorShape: Qt.PointingHandCursor
                        hoverEnabled: true
                    }

                    ColorOverlay {
                        id: colorOverlay
                        anchors.fill: currentUserImage
                        source: currentUserImage
                        color: "#000000"
                        opacity: userImageMouseArea.pressed ? 0.3 : 0.2
                        visible: userImageMouseArea.containsMouse || userImageMouseArea.pressed
                    }

                    layer.enabled: true
                    layer.effect: OpacityMask {
                        maskSource: Item {
                            width: currentUserImage.width
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
                    id: currentUserLabel
                    text: currentUser.displayName
                    font.pointSize: 16
                    bottomPadding: Meui.Units.smallSpacing
                    leftPadding: Meui.Units.largeSpacing
                }

                Label {
                    Layout.alignment: Qt.AlignVCenter
                    id: currentUserLabel2
                    text: currentUser.userName
                    color: Meui.Theme.disabledTextColor
                    visible: currentUser.displayName !== currentUser.userName
                    font.pointSize: 16
                    bottomPadding: Meui.Units.smallSpacing
                }

                Item {
                    Layout.fillWidth: true
                }

                Button {
                    text: additionalSettings.shown ? qsTr("Hide additional settings") : qsTr("Show additional settings")
                    onClicked: additionalSettings.toggle()
                }
            }

            Hideable {
                id: additionalSettings

                Label {
                    text: qsTr("Additional settings")
                    color: Meui.Theme.disabledTextColor
                    Layout.bottomMargin: Meui.Units.largeSpacing
                }

                RowLayout {
                    Layout.fillWidth: true
                    Layout.bottomMargin: Meui.Units.smallSpacing

                    Label {
                        text: qsTr("Automatic login")
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Switch {
                        id: automaticLoginSwitch
                        Layout.fillHeight: true
                        leftPadding: 0
                        rightPadding: 0
                        onCheckedChanged: currentUser.automaticLogin = checked
                    }

                    Component.onCompleted: {
                        automaticLoginSwitch.checked = currentUser.automaticLogin
                    }
                }

                HorizontalDivider {}
            }

            Label {
                id: otherAccountsLabel
                text: qsTr("Other Accounts")
                color: Meui.Theme.disabledTextColor
                topPadding: Meui.Units.largeSpacing
                bottomPadding: Meui.Units.largeSpacing
                visible: _userView.count > 1
            }

            ListView {
                id: _userView
                model: userModel
                Layout.fillWidth: true

                Layout.preferredHeight: itemHeight * (_userView.count - 1)

                property var itemHeight: 50 + Meui.Units.largeSpacing

                delegate: Item {
                    width: _userView.width
                    height: _userView.itemHeight
                    visible: userId !== currentUser.userId

                    RowLayout {
                        id: _itemLayout
                        anchors.fill: parent
                        spacing: Meui.Units.largeSpacing

                        Image {
                            width: 64
                            height: width
                            sourceSize: Qt.size(width, height)
                            source: iconFileName ? "file:///" + iconFileName : "image://icontheme/default-user"
                            visible: status === Image.Ready

                            layer.enabled: true
                            layer.effect: OpacityMask {
                                maskSource: Item {
                                    width: currentUserImage.width
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
                            text: userName
                            font.pointSize: 16
                            bottomPadding: Meui.Units.smallSpacing
                            leftPadding: Meui.Units.largeSpacing
                        }

                        Label {
                            Layout.alignment: Qt.AlignVCenter
                            text: realName
                            color: Meui.Theme.disabledTextColor
                            visible: realName !== userName
                            font.pointSize: 16
                            bottomPadding: Meui.Units.smallSpacing
                        }

                        Item {
                            Layout.fillWidth: true
                        }
                    }
                }
            }

            Item {
                height: Meui.Units.largeSpacing * 2
            }

            Button {
                id: _addUserButton
                text: qsTr("Add user")
                onClicked: addUserDialog.open()
            }
        }
    }
}
