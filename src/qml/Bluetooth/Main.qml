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
import Cutefish.Settings 1.0
import FishUI 1.0 as FishUI
import Cutefish.Bluetooth 1.0 as Bluetooth
import "../"

ItemPage {
    id: control
    headerTitle: qsTr("Bluetooth")
    readonly property bool bluetoothEnabled: Bluetooth.Manager.enabled

    // Pairing prompts come from the shell's agent, which BlueZ falls back to.
    function updateActivity() {
        Bluetooth.Manager.discoveryRequested = visible && Bluetooth.Manager.enabled
    }

    onVisibleChanged: updateActivity()
    onBluetoothEnabledChanged: {
        Bluetooth.Manager.discoveryRequested = visible && bluetoothEnabled
    }
    Component.onCompleted: updateActivity()
    Component.onDestruction: {
        Bluetooth.Manager.discoveryRequested = false
    }

    Bluetooth.DevicesModel {
        id: pairedDevices
        filter: Bluetooth.DevicesModelPrivate.PairedDevices
    }

    Bluetooth.DevicesModel {
        id: otherDevices
        filter: Bluetooth.DevicesModelPrivate.UnpairedDevices
        namedOnly: true
    }

    function openAdapterDetails(adapter) {
        var component = Qt.createComponent("AdapterDetailsDialog.qml")
        if (component.status === Component.Ready) {
            var dialog = component.createObject(rootWindow, {
                "parentWindow": rootWindow,
                "adapter": adapter
            })
            dialog.show()
            dialog.raise()
            dialog.requestActivate()
        }
    }

    Scrollable {
        id: scroll
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            width: scroll.contentWidth
            spacing: Metrics.sectionSpacing

            // The switch and the adapters share the first card; the devices
            // are split into the paired ones and those nearby.
            ColumnLayout {
                Layout.fillWidth: true
                spacing: Metrics.sectionLabelSpacing

                ListCard {
                    ListRow {
                        interactive: false
                        text: qsTr("Bluetooth")

                        FishUI.Switch {
                            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                            rightPadding: 0
                            rightInset: 0
                            checked: Bluetooth.Manager.enabled
                            enabled: Bluetooth.Manager.operational && Bluetooth.Manager.adapters.length > 0
                            onClicked: Bluetooth.Manager.enabled = checked
                        }
                    }

                    Repeater {
                        model: Bluetooth.Manager.adapters

                        delegate: ListRow {
                            required property var modelData

                            visible: control.bluetoothEnabled
                            text: modelData.name
                            onClicked: control.openAdapterDetails(modelData)

                            FishUI.Label {
                                color: FishUI.Theme.disabledTextColor
                                text: modelData.discoverable ? qsTr("Discoverable") : qsTr("Not Discoverable")
                            }

                            IconButton {
                                width: 20
                                iconName: "info"
                                iconColor: FishUI.Theme.highlightColor
                                onClicked: control.openAdapterDetails(modelData)
                            }
                        }
                    }
                }

                SectionLabel {
                    visible: text.length > 0
                    wrapMode: Text.Wrap
                    textFormat: Text.PlainText
                    text: Bluetooth.Manager.errorText.length > 0 ? Bluetooth.Manager.errorText
                          : !Bluetooth.Manager.operational ? qsTr("Bluetooth service is unavailable")
                          : Bluetooth.Manager.hardwareBlocked ? qsTr("Bluetooth is blocked by a hardware switch")
                          : !Bluetooth.Manager.adapters.length ? qsTr("No Bluetooth adapters found")
                          : ""
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: Metrics.sectionSpacing
                visible: control.bluetoothEnabled

                DeviceSection {
                    title: qsTr("My Devices")
                    model: pairedDevices
                }

                DeviceSection {
                    title: qsTr("Other Devices")
                    model: otherDevices
                    busy: Bluetooth.Manager.scanning
                    placeholderText: Bluetooth.Manager.scanning ? qsTr("Searching for devices…")
                                     : qsTr("No devices found. Make sure your device is discoverable.")
                }
            }
        }
    }
}
