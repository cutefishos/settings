// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cutefish.Settings
import FishUI as FishUI
import Cutefish.Bluetooth as Bluetooth
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
                                text: modelData.discoverable ? qsTr("Discoverable") : qsTr("Not discoverable")
                            }

                            IconButton {
                                width: 20
                                Layout.rightMargin: Metrics.trailingGlyphMargin(iconName, width)
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
                    title: qsTr("My devices")
                    model: pairedDevices
                }

                DeviceSection {
                    title: qsTr("Other devices")
                    model: otherDevices
                    busy: Bluetooth.Manager.scanning
                    placeholderText: Bluetooth.Manager.scanning ? ""
                                     : qsTr("No devices found. Make sure your device is discoverable.")
                }
            }
        }
    }
}
