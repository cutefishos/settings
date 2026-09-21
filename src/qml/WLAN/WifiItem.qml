import QtQuick 2.15
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import QtQuick.Window 2.3

import FishUI 1.0 as FishUI
import Cutefish.Network 1.0 as NM
import "../"

// One wireless network. Clicking connects, asks for a password, or unfolds
// the connect/disconnect controls for the current connection.
ListRow {
    id: control

    property bool passwordIsStatic: (model.securityType === NM.Enums.StaticWep || model.securityType === NM.Enums.WpaPsk ||
                                     model.securityType === NM.Enums.Wpa2Psk || model.securityType === NM.Enums.SAE)
    property bool predictableWirelessPassword: !model.uuid && model.type === NM.Enums.Wireless && passwordIsStatic

    text: model.itemUniqueName
    checked: model.connectionState === NM.Enums.Activated

    onExpandedChanged: {
        if (!expanded)
            passwordField.clear()
        else
            passwordField.forceActiveFocus()
    }

    onClicked: {
        if (busyIndicator.visible)
            return

        if (model.uuid || !predictableWirelessPassword) {
            if (connectionState === NM.Enums.Deactivated) {
                if (!predictableWirelessPassword && !model.uuid) {
                    handler.addAndActivateConnection(model.devicePath, model.specificPath)
                } else {
                    handler.activateConnection(model.connectionPath, model.devicePath, model.specificPath)
                }
            } else {
                expanded = !expanded
            }
        } else if (predictableWirelessPassword) {
            connectDialog.name = model.itemUniqueName
            connectDialog.securityType = model.securityType
            connectDialog.devicePath = model.devicePath
            connectDialog.specificPath = model.specificPath
            connectDialog.show()
        }
    }

    leading: Image {
        width: 18
        height: width
        sourceSize: Qt.size(width, height)
        source: "qrc:/images/" + (FishUI.Theme.darkMode ? "dark/" : "light/") + model.connectionIcon + ".svg"
        smooth: false
    }

    FishUI.BusyIndicator {
        id: busyIndicator
        width: 18
        height: width
        visible: connectionState === NM.Enums.Activating ||
                 connectionState === NM.Enums.Deactivating
        running: busyIndicator.visible
    }

    // Activated
    FishUI.Icon {
        name: "check"
        color: FishUI.Theme.highlightColor
        visible: control.checked
    }

    // Locked
    FishUI.Icon {
        width: 16
        height: 16
        name: "lock"
        visible: (model.securityType === -1 | model.securityType === 0) ? false : true
    }

    IconButton {
        width: 16
        iconName: "info"
        iconColor: FishUI.Theme.highlightColor
        onClicked: {
            var component = Qt.createComponent("WirelessDetailsDialog.qml")
            if (component.status === Component.Ready) {
                var dialog = component.createObject(rootWindow, { "parentWindow": rootWindow })
                dialog.show()
                dialog.raise()
                dialog.requestActivate()
            }
        }
    }

    details: [
        RowLayout {
            visible: predictableWirelessPassword
            spacing: FishUI.Units.largeSpacing

            Label {
                text: qsTr("Password")
            }

            TextField {
                id: passwordField
                focus: true
                echoMode: TextInput.Password
                selectByMouse: true
                placeholderText: qsTr("Password")
                validator: RegularExpressionValidator {
                    regularExpression: {
                        if (model.securityType === NM.Enums.StaticWep)
                            return /^(?:[\x20-\x7F]{5}|[0-9a-fA-F]{10}|[\x20-\x7F]{13}|[0-9a-fA-F]{26}){1}$/;
                        return /^(?:[\x20-\x7F]{8,64}){1}$/;
                    }
                }
                onAccepted: control.connectWithPassword()
                Keys.onEscapePressed: control.expanded = false
                Layout.fillWidth: true
            }

            Button {
                text: qsTr("Cancel")
                onClicked: control.expanded = false
            }

            Button {
                text: qsTr("Connect")
                flat: true
                enabled: passwordField.acceptableInput
                onClicked: control.connectWithPassword()
            }
        },

        RowLayout {
            visible: !predictableWirelessPassword

            Button {
                visible: (model.uuid || !predictableWirelessPassword) && connectionState === NM.Enums.Deactivated
                text: qsTr("Connect")
                flat: true

                onClicked: {
                    if (!predictableWirelessPassword && !model.uuid) {
                        handler.addAndActivateConnection(model.devicePath, model.specificPath)
                    } else {
                        handler.activateConnection(model.connectionPath, model.devicePath, model.specificPath)
                    }
                }
            }

            Button {
                visible: connectionState === NM.Enums.Activated
                text: qsTr("Disconnect")

                onClicked: {
                    handler.deactivateConnection(model.connectionPath, model.devicePath)
                }
            }
        },

        Item {
            height: FishUI.Units.largeSpacing
        }
    ]

    function connectWithPassword() {
        handler.addAndActivateConnection(model.devicePath, model.specificPath, passwordField.text)
    }
}
