/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     rekols <aj@cutefishos.com>
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

import QtQuick 2.4
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3

import FishUI 1.0 as FishUI
import Cutefish.Settings 1.0
import Cutefish.Network 1.0 as NM

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

    NM.NetworkModel {
        id: networkModel
    }

    NetworkMockModel {
        id: mockNetworkModel
    }

    NM.EnabledConnections {
        id: enabledConnections
    }

    NM.IdentityModel {
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

            // Wired connection
            ColumnLayout {
                Layout.fillWidth: true
                spacing: Metrics.sectionLabelSpacing

                SectionLabel {
                    text: qsTr("Ethernet")
                    visible: wiredView.count > 0
                }

                ListCard {
                    visible: wiredView.count > 0

                    Repeater {
                        id: wiredView

                        model: NM.AppletProxyModel {
                            type: NM.AppletProxyModel.WiredType
                            sourceModel: rootWindow.mockNetworkData ? mockNetworkModel : connectionModel
                        }

                        delegate: WiredItem {}
                    }
                }
            }

        }
    }
}
