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
    property int activatingCount: 0

    // Switching networks leaves the old one Deactivating until the new one is
    // up; show it as already gone so only the network being joined spins.
    function displayedState(state) {
        if (state === NM.Enums.Deactivating && activatingCount > 0)
            return NM.Enums.Deactivated

        return state
    }

    function recountActivating() {
        var count = 0
        for (var i = 0; i < currentRepeater.count; ++i) {
            var item = currentRepeater.itemAt(i)
            if (item && item.activating)
                ++count
        }
        activatingCount = count
    }

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
            id: currentRepeater
            model: wirelessModel

            delegate: WifiItem {
                readonly property bool activating: model.connectionState === NM.Enums.Activating
                displayedState: _contentLayout.displayedState(model.connectionState)
                visible: displayedState !== NM.Enums.Deactivated
                onActivatingChanged: _contentLayout.recountActivating()
                Component.onCompleted: _contentLayout.recountActivating()
                Component.onDestruction: _contentLayout.recountActivating()
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
        displayedState: _contentLayout.displayedState
        // A saved connection that is not the one in use.
        includes: function(item) {
            return !!item.uuid && _contentLayout.displayedState(item.connectionState) === NM.Enums.Deactivated
        }
    }

    WifiSection {
        title: qsTr("Other Networks")
        model: wirelessModel
        displayedState: _contentLayout.displayedState
        includes: function(item) {
            return !item.uuid && _contentLayout.displayedState(item.connectionState) === NM.Enums.Deactivated
        }
        footerText: wirelessSwitch.checked ? qsTr("Add other...") : ""
        onFooterClicked: newNetworkDialog.show()
    }
}
