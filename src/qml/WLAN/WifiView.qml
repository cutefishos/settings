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
import FishUI 1.0 as FishUI
import Cutefish.NetworkManagement 1.0 as NM

ColumnLayout {
    id: _contentLayout
    spacing: FishUI.Units.largeSpacing

    RowLayout {
        spacing: FishUI.Units.smallSpacing * 1.5

        Label {
            id: wlanLabel
            text: qsTr("WLAN")
            color: FishUI.Theme.disabledTextColor
        }

        FishUI.BusyIndicator {
            id: wlanBusyIndicator
            width: wirelessSwitch.height
            height: width
            visible: enabledConnections.wirelessEnabled && wirelessView.count === 0
            running: wlanBusyIndicator.visible
        }

        Item {
            Layout.fillWidth: true
        }

        Switch {
            id: wirelessSwitch
            height: wlanLabel.implicitHeight
            leftPadding: 0
            rightPadding: 0

            checked: enabledConnections.wirelessEnabled
            onCheckedChanged: {
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

    ListView {
        id: wirelessView
        Layout.fillWidth: true
        Layout.preferredHeight: itemHeight * count + ((count - 1) * spacing)
        clip: true

        model: NM.AppletProxyModel {
            type: NM.AppletProxyModel.WirelessType
            sourceModel: connectionModel
        }

        spacing: FishUI.Units.largeSpacing
        interactive: false
        visible: count > 0

        property var itemHeight: 38

        delegate: WifiItem {
            width: ListView.view.width
            height: ListView.view.itemHeight
        }
    }
}
