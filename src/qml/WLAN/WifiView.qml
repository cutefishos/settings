// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

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

        // The adapter serves the hotspot instead of joining networks, so there is nothing to list.
        ListRow {
            visible: handler.hotspotActive
            text: configuration.hotspotName
            onClicked: rootWindow.switchPageFromName("hotspot")

            leading: FishUI.Icon {
                width: 18
                height: width
                name: "network-wireless-hotspot"
            }

            FishUI.Label {
                text: qsTr("Hotspot")
                color: FishUI.Theme.disabledTextColor
            }

            FishUI.Icon {
                width: 16
                height: width
                Layout.rightMargin: Metrics.trailingGlyphMargin(name, width)
                name: "chevron-right"
                color: FishUI.Theme.disabledTextColor
            }
        }
    }

    NM.AppletProxyModel {
        id: wirelessModel
        type: NM.AppletProxyModel.WirelessType
        sourceModel: _contentLayout.sourceModel
    }

    WifiSection {
        visible: !handler.hotspotActive && (shown > 0 || footerText.length > 0)
        title: qsTr("Known Networks")
        model: wirelessModel
        displayedState: _contentLayout.displayedState
        // A saved connection that is not the one in use.
        includes: function(item) {
            return !!item.uuid && _contentLayout.displayedState(item.connectionState) === NM.Enums.Deactivated
        }
    }

    WifiSection {
        visible: !handler.hotspotActive && (shown > 0 || footerText.length > 0)
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
