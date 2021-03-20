import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.0
import Qt.labs.platform 1.0 as LabsPlatform

import Cutefish.Settings 1.0
import Cutefish.Accounts 1.0
import MeuiKit 1.0 as Meui

import "../"

Item {
    id: control

    height: mainLayout.implicitHeight

    UserAccount {
        id: currentUser
        userId: model.userId
    }

    LabsPlatform.FileDialog {
        id: currentUserFileDialog
        folder: LabsPlatform.StandardPaths.writableLocation(LabsPlatform.StandardPaths.PicturesLocation)
        nameFilters: ["Pictures (*.png *.jpg *.gif)"]
        onFileChanged: {
            currentUser.iconFileName = currentFile.toString().replace("file://", "")
            _userImage.source = currentFile
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

            Image {
                id: _userImage
                width: 50
                height: 50
                sourceSize: Qt.size(width, height)
                source: iconFileName ? "file:///" + iconFileName : "image://icontheme/default-user"
                visible: status === Image.Ready

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

            Label {
                text: qsTr("Currently logged")
                rightPadding: Meui.Units.largeSpacing
                visible: currentUser.userId === loggedUser.userId
            }

            Button {
                onClicked: additionalSettings.toggle()

                implicitWidth: height

                Image {
                    anchors.centerIn: parent
                    width: 22
                    height: 22
                    sourceSize: Qt.size(width, height)
                    source: Meui.Theme.darkMode ? additionalSettings.shown ? "qrc:/images/dark/up.svg" : "qrc:/images/dark/down.svg"
                                                : additionalSettings.shown ? "qrc:/images/light/up.svg" : "qrc:/images/light/down.svg"
                }
            }
        }

        Item {
            height: Meui.Units.largeSpacing
        }

        Hideable {
            id: additionalSettings

            GridLayout {
                Layout.fillWidth: true
                Layout.bottomMargin: Meui.Units.smallSpacing
                rowSpacing: Meui.Units.largeSpacing
                columns: 2

                Label {
                    text: qsTr("Avatar")
                    Layout.fillWidth: true
                }

                Button {
                    text: qsTr("Choose")
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                    onClicked: currentUserFileDialog.open()
                }

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
            }

            HorizontalDivider {}
        }
    }
}
