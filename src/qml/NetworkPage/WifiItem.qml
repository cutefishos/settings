import QtQuick 2.4
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0
import QtQuick.Window 2.3

import FishUI 1.0 as FishUI
import Cutefish.NetworkManagement 1.0 as NM
import "../"

Item {
    id: control

    property bool passwordIsStatic: (model.securityType === NM.Enums.StaticWep || model.securityType === NM.Enums.WpaPsk ||
                                     model.securityType === NM.Enums.Wpa2Psk || model.securityType === NM.Enums.SAE)
    property bool predictableWirelessPassword: !model.uuid && model.type === NM.Enums.Wireless && passwordIsStatic

    Rectangle {
        anchors.fill: parent
        radius: FishUI.Theme.smallRadius
        color: mouseArea.containsMouse ? Qt.rgba(FishUI.Theme.textColor.r,
                                                 FishUI.Theme.textColor.g,
                                                 FishUI.Theme.textColor.b,
                                                 0.1) : "transparent"

        Behavior on color {
            ColorAnimation {
                duration: 125
                easing.type: Easing.InOutCubic
            }
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton

        onClicked: {
            if (model.uuid || !predictableWirelessPassword) {
                if (connectionState === NM.Enums.Deactivated) {
                    if (!predictableWirelessPassword && !model.uuid) {
                        handler.addAndActivateConnection(model.devicePath, model.specificPath);
                    } else {
                        handler.activateConnection(model.connectionPath, model.devicePath, model.specificPath);
                    }
                } else {
                    handler.deactivateConnection(model.connectionPath, model.devicePath);
                }
            } else if (predictableWirelessPassword) {
                passwordDialog.show()
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: FishUI.Units.smallSpacing
        spacing: FishUI.Units.largeSpacing

        Image {
            width: 22
            height: width
            sourceSize: Qt.size(width, height)
            source: "qrc:/images/" + (FishUI.Theme.darkMode ? "dark/" : "light/") + model.connectionIcon + ".svg"
        }

        Label {
            text: model.itemUniqueName
        }

        Item {
            Layout.fillWidth: true
        }

        FishUI.BusyIndicator {
            id: busyIndicator
            width: 22
            height: width
            visible: connectionState === NM.Enums.Activating ||
                     connectionState === NM.Enums.Deactivating
            running: busyIndicator.visible
        }

        // Activated
        Image {
            width: 16
            height: width
            sourceSize: Qt.size(width, height)
            source: "qrc:/images/checked.svg"
            visible: model.connectionState === NM.Enums.Activated

            ColorOverlay {
                anchors.fill: parent
                source: parent
                color: FishUI.Theme.highlightColor
                opacity: 1
                visible: true
            }
        }

        // Locked
        Image {
            width: 16
            height: width
            sourceSize: Qt.size(width, height)
            source: "qrc:/images/locked.svg"
            visible: (model.securityType === -1 | model.securityType === 0) ? false : true

            ColorOverlay {
                anchors.fill: parent
                source: parent
                color: FishUI.Theme.textColor
                opacity: 1
                visible: true
            }
        }

        IconButton {
            source: "qrc:/images/info.svg"
            onClicked: {
                var component = Qt.createComponent("WirelessDetailsDialog.qml")
                if (component.status === Component.Ready) {
                    var dialog = component.createObject(rootWindow)
                    dialog.open()
                }
            }
        }
    }

    Window {
        id: passwordDialog
        title: model.itemUniqueName

        width: 300
        height: mainLayout.implicitHeight + FishUI.Units.largeSpacing * 2
        minimumWidth: width
        minimumHeight: height
        maximumHeight: height
        maximumWidth: width
        flags: Qt.Dialog
        modality: Qt.WindowModal

        signal accept()

        onVisibleChanged: {
            passwordField.text = ""
            passwordField.forceActiveFocus()
            showPasswordCheckbox.checked = false
        }

        onAccept: {
            handler.addAndActivateConnection(model.devicePath, model.specificPath, passwordField.text)
            passwordDialog.close()
        }

        Rectangle {
            anchors.fill: parent
            color: FishUI.Theme.backgroundColor
        }

        ColumnLayout {
            id: mainLayout
            anchors.fill: parent
            anchors.margins: FishUI.Units.largeSpacing

            TextField {
                id: passwordField
                focus: true
                echoMode: showPasswordCheckbox.checked ? TextInput.Normal : TextInput.Password
                selectByMouse: true
                placeholderText: qsTr("Password")
                validator: RegExpValidator {
                    regExp: {
                        if (model.securityType === NM.Enums.StaticWep)
                            return /^(?:[\x20-\x7F]{5}|[0-9a-fA-F]{10}|[\x20-\x7F]{13}|[0-9a-fA-F]{26}){1}$/;
                        return /^(?:[\x20-\x7F]{8,64}){1}$/;
                    }
                }
                onAccepted: passwordDialog.accept()
                Layout.fillWidth: true
            }

            Item {
                height: FishUI.Units.smallSpacing
            }

            CheckBox {
                id: showPasswordCheckbox
                checked: false
                text: qsTr("Show password")
            }

            Item {
                height: FishUI.Units.largeSpacing
            }

            RowLayout {
                Button {
                    text: qsTr("Cancel")
                    Layout.fillWidth: true
                    onClicked: passwordDialog.close()
                }

                Button {
                    text: qsTr("Connect")
                    enabled: passwordField.acceptableInput
                    Layout.fillWidth: true
                    flat: true
                    onClicked: passwordDialog.accept()
                }
            }
        }
    }
}
