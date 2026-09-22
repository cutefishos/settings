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
    PairDialog {
        hostWindow: rootWindow
    }

    FishUI.Window {
        id: forgetDialog
        property var device: null
        parentWindow: rootWindow
        x: rootWindow.x + Math.round((rootWindow.width - width) / 2)
        y: rootWindow.y + Math.round((rootWindow.height - height) / 2)
        width: 420
        property int contentHeight: forgetLayout.implicitHeight + header.height + 32
        height: contentHeight
        minimumWidth: width
        maximumWidth: width
        minimumHeight: contentHeight
        maximumHeight: contentHeight
        flags: Qt.Dialog | Qt.FramelessWindowHint
        modality: Qt.WindowModal
        visible: false
        minimizeButtonVisible: false
        header.height: 44
        contentTopMargin: header.height
        background.color: FishUI.Theme.secondBackgroundColor
        title: qsTr("Forget this device?")

        function open() {
            show()
            raise()
            requestActivate()
        }

        headerItem: FishUI.Label {
            anchors.left: parent.left
            anchors.leftMargin: 16
            anchors.verticalCenter: parent.verticalCenter
            width: parent.width - 64
            text: forgetDialog.title
            textFormat: Text.PlainText
            font.pixelSize: 14
            font.weight: Font.DemiBold
            color: FishUI.Theme.textColor
            elide: Text.ElideRight
        }

        ColumnLayout {
            id: forgetLayout
            anchors.fill: parent
            anchors.margins: 16
            spacing: 16

            FishUI.Label {
                Layout.fillWidth: true
                text: forgetDialog.device ? forgetDialog.device.name : ""
                textFormat: Text.PlainText
                wrapMode: Text.Wrap
            }

            RowLayout {
                Layout.alignment: Qt.AlignRight
                spacing: 8

                FishUI.ConfirmDialogButton {
                    text: qsTr("Cancel")
                    onClicked: forgetDialog.close()
                }

                FishUI.ConfirmDialogButton {
                    text: qsTr("Forget")
                    primary: true
                    onClicked: {
                        if (forgetDialog.device)
                            forgetDialog.device.forget()
                        forgetDialog.close()
                    }
                }
            }
        }

        onClosing: device = null
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
                    FishUI.Label {
                        text: qsTr("Bluetooth")
                        Layout.fillWidth: true
                    }
                    FishUI.Switch {
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                        checked: Bluetooth.Manager.enabled
                        enabled: Bluetooth.Manager.operational && Bluetooth.Manager.adapters.length > 0
                        rightPadding: 0
                        rightInset: 0
                        onClicked: Bluetooth.Manager.enabled = checked
                    }
                }
                FishUI.Label {
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
                    FishUI.Label {
                        Layout.fillWidth: true
                        text: Bluetooth.Manager.scanning ? qsTr("Searching for devices…") : qsTr("Devices")
                    }
                    BusyIndicator {
                        running: Bluetooth.Manager.scanning
                        implicitWidth: 28
                        implicitHeight: 28
                    }
                    FishUI.Button {
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
                FishUI.Label {
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                    textFormat: Text.PlainText
                    text: Bluetooth.Manager.errorText
                }
                FishUI.Button {
                    text: qsTr("Dismiss")
                    onClicked: Bluetooth.Manager.clearError()
                }
            }

            Repeater {
                model: Bluetooth.Manager.adapters
                delegate: RoundedItem {
                    required property var modelData
                    FishUI.Label {
                        Layout.fillWidth: true
                        text: modelData.name + " · " + modelData.address
                        textFormat: Text.PlainText
                        elide: Text.ElideRight
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        FishUI.TextField {
                            id: adapterName
                            Layout.fillWidth: true
                            placeholderText: modelData.name
                            maximumLength: 248
                        }
                        FishUI.Button {
                            text: qsTr("Rename")
                            enabled: adapterName.text.length > 0
                            onClicked: {
                                modelData.name = adapterName.text
                                adapterName.clear()
                            }
                        }
                    }
                    RowLayout {
                        FishUI.Switch {
                            text: qsTr("Powered")
                            checked: modelData.powered
                            onClicked: modelData.powered = checked
                        }
                        FishUI.Switch {
                            text: qsTr("Visible to other devices")
                            enabled: modelData.powered
                            checked: modelData.discoverable
                            onClicked: modelData.discoverable = checked
                        }
                    }
                    FishUI.Switch {
                        text: qsTr("Allow incoming pairing")
                        enabled: modelData.powered
                        checked: modelData.pairable
                        onClicked: modelData.pairable = checked
                    }
                }
            }

            FishUI.Label {
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
                            FishUI.Label {
                                Layout.fillWidth: true
                                text: device.name
                                textFormat: Text.PlainText
                                elide: Text.ElideRight
                            }
                            FishUI.Label {
                                text: device.address + " · " + device.adapterPath.split("/").pop()
                                color: FishUI.Theme.disabledTextColor
                            }
                            FishUI.Label {
                                text: device.connected ? qsTr("Connected")
                                      : device.paired ? qsTr("Paired") : qsTr("Not paired")
                            }
                        }
                        FishUI.Label {
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
                        FishUI.Button {
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
                        FishUI.Button {
                            visible: device.busy && !device.paired
                            text: qsTr("Cancel pairing")
                            onClicked: device.cancelPairing()
                        }
                        FishUI.Button {
                            visible: device.paired
                            enabled: !device.busy
                            text: qsTr("Forget")
                            onClicked: {
                                forgetDialog.device = device
                                forgetDialog.open()
                            }
                        }
                        FishUI.CheckBox {
                            visible: device.paired
                            text: qsTr("Trusted")
                            checked: device.trusted
                            enabled: !device.busy
                            onClicked: device.trusted = checked
                        }
                        FishUI.CheckBox {
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
