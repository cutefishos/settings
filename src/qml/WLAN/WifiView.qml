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

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import "../"

import Cutefish.Settings 1.0
import FishUI 1.0 as FishUI
import Cutefish.Network 1.0 as NM

ColumnLayout {
    id: _contentLayout
    spacing: Metrics.sectionSpacing
    property var sourceModel: connectionModel

    // The switch and the network in use share the first card; the rest is
    // split into the networks with a saved connection and the others.
    ListCard {
        ListRow {
            interactive: false
            text: qsTr("Wi-Fi")

            FishUI.Switch {
                id: wirelessSwitch
                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                rightPadding: 0
                rightInset: 0

                checked: rootWindow.mockNetworkData || enabledConnections.wirelessEnabled
                onCheckedChanged: {
                    if (rootWindow.mockNetworkData)
                        return

                    if (checked) {
                        if (!enabledConnections.wirelessEnabled) {
                            handler.enableWireless(checked)
                            handler.requestScan()
                        }
                    } else {
                        if (enabledConnections.wirelessEnabled) {
                            handler.enableWireless(checked)
                        }
                    }
                }
            }
        }

        Repeater {
            model: wirelessModel

            delegate: WifiItem {
                visible: model.connectionState !== NM.Enums.Deactivated
            }
        }
    }

    NM.AppletProxyModel {
        id: wirelessModel
        type: NM.AppletProxyModel.WirelessType
        sourceModel: _contentLayout.sourceModel
    }

    WifiSection {
        title: qsTr("Known Networks")
        model: wirelessModel
        // A saved connection that is not the one in use.
        includes: function(item) {
            return !!item.uuid && item.connectionState === NM.Enums.Deactivated
        }
    }

    WifiSection {
        title: qsTr("Other Networks")
        model: wirelessModel
        includes: function(item) {
            return !item.uuid && item.connectionState === NM.Enums.Deactivated
        }
    }
}
