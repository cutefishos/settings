import QtQuick 2.4
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0
import QtQuick.Window 2.3

import MeuiKit 1.0 as Meui
import Cutefish.NetworkManagement 1.0 as NM

Item {
    id: control

    property bool passwordIsStatic: (model.securityType === NM.NetworkModel.StaticWep || model.securityType === NM.NetworkModel.WpaPsk ||
                                     model.securityType === NM.NetworkModel.Wpa2Psk || model.securityType === NM.NetworkModel.SAE)
    property bool predictableWirelessPassword: !model.uuid && model.type === NM.NetworkModel.Wireless && passwordIsStatic

    Rectangle {
        anchors.fill: parent
        radius: Meui.Theme.smallRadius
        color: mouseArea.containsMouse ? Qt.rgba(Meui.Theme.textColor.r,
                                                 Meui.Theme.textColor.g,
                                                 Meui.Theme.textColor.b,
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
                if (connectionState === NM.NetworkModel.Deactivated) {
                    if (!predictableWirelessPassword && !model.uuid) {
                        networking.addAndActivateConnection(model.devicePath, model.specificPath);
                    } else {
                        networking.activateConnection(model.connectionPath, model.devicePath, model.specificPath);
                    }
                } else {
                    networking.deactivateConnection(model.connectionPath, model.devicePath);
                }
            } else if (predictableWirelessPassword) {
                passwordDialog.show()
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: Meui.Units.smallSpacing
        spacing: Meui.Units.largeSpacing

        Image {
            width: 22
            height: width
            sourceSize: Qt.size(width, height)
            source: "qrc:/images/" + (Meui.Theme.darkMode ? "dark/" : "light/") + model.connectionIcon + ".svg"
        }

        Label {
            text: model.itemUniqueName
        }

        Item {
            Layout.fillWidth: true
        }

        Meui.BusyIndicator {
            id: busyIndicator
            width: 22
            height: width
            visible: connectionState === NM.NetworkModel.Activating ||
                     connectionState === NM.NetworkModel.Deactivating
            running: busyIndicator.visible
        }

        // Activated
        Image {
            width: 16
            height: width
            sourceSize: Qt.size(width, height)
            source: "qrc:/images/checked.svg"
            visible: model.connectionState === NM.NetworkModel.Activated

            ColorOverlay {
                anchors.fill: parent
                source: parent
                color: Meui.Theme.highlightColor
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
            visible: model.securityType !== -1 && model.securityType !== 0

            ColorOverlay {
                anchors.fill: parent
                source: parent
                color: Meui.Theme.textColor
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
        height: mainLayout.implicitHeight + Meui.Units.largeSpacing * 2
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
            networking.addAndActivateConnection(model.devicePath, model.specificPath, passwordField.text)
            passwordDialog.close()
        }

        Rectangle {
            anchors.fill: parent
            color: Meui.Theme.backgroundColor
        }

        ColumnLayout {
            id: mainLayout
            anchors.fill: parent
            anchors.margins: Meui.Units.largeSpacing

            TextField {
                id: passwordField
                focus: true
                echoMode: showPasswordCheckbox.checked ? TextInput.Normal : TextInput.Password
                selectByMouse: true
                placeholderText: qsTr("Password")
                validator: RegExpValidator {
                    regExp: {
                        if (model.securityType === NM.NetworkModel.StaticWep)
                            return /^(?:[\x20-\x7F]{5}|[0-9a-fA-F]{10}|[\x20-\x7F]{13}|[0-9a-fA-F]{26}){1}$/;
                        return /^(?:[\x20-\x7F]{8,64}){1}$/;
                    }
                }
                onAccepted: passwordDialog.accept()
                Layout.fillWidth: true
            }

            Item {
                height: Meui.Units.smallSpacing
            }

            CheckBox {
                id: showPasswordCheckbox
                checked: false
                text: qsTr("Show password")
            }

            Item {
                height: Meui.Units.largeSpacing
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
