// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.4
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3

import FishUI 1.0 as FishUI
import Cutefish.Settings 1.0
import Cutefish.Network 1.0 as NM

import "../"

ItemPage {
    id: control
    headerTitle: qsTr("Wi-Fi")

    property var itemHeight: 45
    property var settingsMap: ({})

    NM.Handler {
        id: handler
    }

    NM.WifiSettings {
        id: wifiSettings
    }

    NetworkMockModel {
        id: mockNetworkModel
    }

    NM.EnabledConnections {
        id: enabledConnections
    }

    NM.NetworkModel {
        id: connectionModel
    }

    NM.Configuration {
        id: configuration
    }

    NewNetworkDialog {
        id: newNetworkDialog

        onConnect: {
            wifiSettings.addOtherConnection(ssid, username, pwd, type)
        }
    }

    Component.onCompleted: {
        if (!rootWindow.mockNetworkData)
            handler.requestScan()
    }

    Timer {
        id: scanTimer
        interval: 10200
        repeat: true
        running: control.visible && !rootWindow.mockNetworkData
        onTriggered: handler.requestScan()
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: mainLayout.implicitHeight
        visible: enabledConnections.wirelessHwEnabled || rootWindow.mockNetworkData

        ColumnLayout {
            id: mainLayout
            anchors.fill: parent
            spacing: Metrics.sectionSpacing

            WifiView {
                Layout.fillWidth: true
                visible: enabledConnections.wirelessHwEnabled || rootWindow.mockNetworkData
                sourceModel: rootWindow.mockNetworkData ? mockNetworkModel : connectionModel
            }

            // Hotspot
            // Not finished yet.
//            RoundedItem {
//                id: hotspotItem
//                visible: handler.hotspotSupported

//                RowLayout {
//                    Label {
//                        text: qsTr("Hotspot")
//                        color: FishUI.Theme.disabledTextColor
//                    }

//                    Item {
//                        Layout.fillWidth: true
//                    }

//                    Switch {
//                        Layout.fillHeight: true
//                        rightPadding: 0

//                        onToggled: {
//                            if (checked) {
//                                handler.createHotspot()
//                            } else {
//                                handler.stopHotspot()
//                            }
//                        }
//                    }
//                }

//                Item {
//                    height: FishUI.Units.largeSpacing
//                }

//                TextField {
//                    id: ssidName
//                    text: configuration.hotspotName
//                    placeholderText: qsTr("SSID")
//                }

//                TextField {
//                    id: hotspotPassword
//                    placeholderText: qsTr("Password")
//                    text: configuration.hotspotPassword
//                }
//            }

        }
    }
}
