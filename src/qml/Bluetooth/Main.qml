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

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FishUI 1.0 as FishUI
import Cutefish.Bluetooth 1.0 as Bluetooth
import "../"

ItemPage {
    id: control
    headerTitle: qsTr("Bluetooth")
    readonly property bool bluetoothEnabled: Bluetooth.Manager.enabled

    function updateActivity() {
        Bluetooth.Manager.agentEnabled = visible
        Bluetooth.Manager.discoveryRequested = visible && Bluetooth.Manager.enabled
    }

    onVisibleChanged: updateActivity()
    onBluetoothEnabledChanged: {
        Bluetooth.Manager.discoveryRequested = visible && bluetoothEnabled
    }
    Component.onCompleted: updateActivity()
    Component.onDestruction: {
        Bluetooth.Manager.discoveryRequested = false
        Bluetooth.Manager.agentEnabled = false
    }

    Bluetooth.DevicesModel { id: devicesModel }
    PairDialog { }

    Dialog {
        id: forgetDialog
        property var device: null
        title: qsTr("Forget this device?")
        anchors.centerIn: parent
        modal: true
        standardButtons: Dialog.Cancel | Dialog.Ok
        Label {
            text: forgetDialog.device ? forgetDialog.device.name : ""
            textFormat: Text.PlainText
        }
        onAccepted: {
            if (device)
                device.forget()
            device = null
        }
        onRejected: device = null
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            width: parent.contentWidth
            spacing: FishUI.Units.largeSpacing

            RoundedItem {
                RowLayout {
                    Label {
                        text: qsTr("Bluetooth")
                        Layout.fillWidth: true
                    }
                    Switch {
                        checked: Bluetooth.Manager.enabled
                        enabled: Bluetooth.Manager.operational && Bluetooth.Manager.adapters.length > 0
                        onClicked: Bluetooth.Manager.enabled = checked
                    }
                }
                Label {
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                    visible: !Bluetooth.Manager.operational || !Bluetooth.Manager.adapters.length
                             || Bluetooth.Manager.hardwareBlocked || !Bluetooth.Manager.enabled
                    text: !Bluetooth.Manager.operational ? qsTr("Bluetooth service is unavailable")
                          : Bluetooth.Manager.hardwareBlocked ? qsTr("Bluetooth is blocked by a hardware switch")
                          : !Bluetooth.Manager.adapters.length ? qsTr("No Bluetooth adapters found")
                          : qsTr("Bluetooth is disabled")
                }
                RowLayout {
                    visible: Bluetooth.Manager.enabled
                    Label {
                        Layout.fillWidth: true
                        text: Bluetooth.Manager.scanning ? qsTr("Searching for devices…") : qsTr("Devices")
                    }
                    BusyIndicator {
                        running: Bluetooth.Manager.scanning
                        implicitWidth: 28
                        implicitHeight: 28
                    }
                    Button {
                        text: Bluetooth.Manager.discoveryRequested ? qsTr("Stop searching") : qsTr("Search")
                        onClicked: {
                            Bluetooth.Manager.clearError()
                            Bluetooth.Manager.discoveryRequested = !Bluetooth.Manager.discoveryRequested
                        }
                    }
                }
            }

            RoundedItem {
                visible: Bluetooth.Manager.errorText.length > 0
                Label {
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                    textFormat: Text.PlainText
                    text: Bluetooth.Manager.errorText
                }
                Button {
                    text: qsTr("Dismiss")
                    onClicked: Bluetooth.Manager.clearError()
                }
            }

            Repeater {
                model: Bluetooth.Manager.adapters
                delegate: RoundedItem {
                    required property var modelData
                    Label {
                        Layout.fillWidth: true
                        text: modelData.name + " · " + modelData.address
                        textFormat: Text.PlainText
                        elide: Text.ElideRight
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        TextField {
                            id: adapterName
                            Layout.fillWidth: true
                            placeholderText: modelData.name
                            maximumLength: 248
                        }
                        Button {
                            text: qsTr("Rename")
                            enabled: adapterName.text.length > 0
                            onClicked: {
                                modelData.name = adapterName.text
                                adapterName.clear()
                            }
                        }
                    }
                    RowLayout {
                        Switch {
                            text: qsTr("Powered")
                            checked: modelData.powered
                            onClicked: modelData.powered = checked
                        }
                        Switch {
                            text: qsTr("Visible to other devices")
                            enabled: modelData.powered
                            checked: modelData.discoverable
                            onClicked: modelData.discoverable = checked
                        }
                    }
                    Switch {
                        text: qsTr("Allow incoming pairing")
                        enabled: modelData.powered
                        checked: modelData.pairable
                        onClicked: modelData.pairable = checked
                    }
                }
            }

            Label {
                visible: Bluetooth.Manager.enabled && devicesModel.count === 0
                text: qsTr("No devices found. Make your device discoverable and search again.")
                wrapMode: Text.Wrap
                Layout.fillWidth: true
            }

            Repeater {
                model: devicesModel
                delegate: RoundedItem {
                    required property var device
                    visible: Bluetooth.Manager.enabled
                    RowLayout {
                        ColumnLayout {
                            Layout.fillWidth: true
                            Label {
                                Layout.fillWidth: true
                                text: device.name
                                textFormat: Text.PlainText
                                elide: Text.ElideRight
                            }
                            Label {
                                text: device.address + " · " + device.adapterPath.split("/").pop()
                                color: FishUI.Theme.disabledTextColor
                            }
                            Label {
                                text: device.connected ? qsTr("Connected")
                                      : device.paired ? qsTr("Paired") : qsTr("Not paired")
                            }
                        }
                        Label {
                            visible: device.batteryPercentage >= 0
                            text: qsTr("%1% battery").arg(device.batteryPercentage)
                        }
                        BusyIndicator {
                            running: device.busy
                            visible: running
                            implicitWidth: 28
                            implicitHeight: 28
                        }
                    }
                    RowLayout {
                        Button {
                            text: device.connected ? qsTr("Disconnect")
                                  : device.paired ? qsTr("Connect") : qsTr("Pair")
                            enabled: !device.busy && (device.paired || Bluetooth.Manager.agentReady)
                            onClicked: {
                                if (device.connected)
                                    device.disconnectFromDevice()
                                else if (device.paired)
                                    device.connectToDevice()
                                else
                                    device.pair()
                            }
                        }
                        Button {
                            visible: device.busy && !device.paired
                            text: qsTr("Cancel pairing")
                            onClicked: device.cancelPairing()
                        }
                        Button {
                            visible: device.paired
                            enabled: !device.busy
                            text: qsTr("Forget")
                            onClicked: {
                                forgetDialog.device = device
                                forgetDialog.open()
                            }
                        }
                        CheckBox {
                            visible: device.paired
                            text: qsTr("Trusted")
                            checked: device.trusted
                            enabled: !device.busy
                            onClicked: device.trusted = checked
                        }
                        CheckBox {
                            text: qsTr("Blocked")
                            checked: device.blocked
                            enabled: !device.busy
                            onClicked: device.blocked = checked
                        }
                    }
                }
            }
        }
    }
}
