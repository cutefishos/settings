// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import FishUI as FishUI
import Cutefish.Settings
import Cutefish.Network as NM

import "../"

ItemPage {
    id: control
    headerTitle: qsTr("Ethernet")

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

    Scrollable {
        anchors.fill: parent
        contentHeight: mainLayout.implicitHeight

        ColumnLayout {
            id: mainLayout
            anchors.fill: parent
            spacing: Metrics.sectionSpacing

            // The switch and the wired connections share one card.
            ListCard {
                visible: rootWindow.mockNetworkData || enabledConnections.wiredAvailable

                ListRow {
                    interactive: false
                    text: qsTr("Ethernet")

                    FishUI.Switch {
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                        rightPadding: 0
                        rightInset: 0
                        checked: rootWindow.mockNetworkData || enabledConnections.wiredEnabled
                        onToggled: {
                            if (!rootWindow.mockNetworkData)
                                handler.enableWired(checked)
                        }
                    }
                }

                Repeater {
                    model: NM.AppletProxyModel {
                        type: NM.AppletProxyModel.WiredType
                        sourceModel: rootWindow.mockNetworkData ? mockNetworkModel : connectionModel
                    }

                    delegate: WiredItem {
                        visible: rootWindow.mockNetworkData || enabledConnections.wiredEnabled
                    }
                }
            }

        }
    }
}
