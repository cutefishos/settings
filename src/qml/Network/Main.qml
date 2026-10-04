// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts

import Cutefish.Settings
import FishUI as FishUI
import Cutefish.Network as NM
import "../"

// Network services that aren't a device of their own, each on a page of its own.
ItemPage {
    headerTitle: qsTr("Network")

    NM.Handler {
        id: handler
    }

    NM.EnabledConnections {
        id: enabledConnections
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent
            spacing: Metrics.sectionSpacing

            ListCard {
                NavigationItem {
                    text: qsTr("VPN")
                    leading: RowIcon {
                        source: "qrc:/fishui/kit/images/icons/network-vpn.svg"
                        tint: "#48484C"
                    }
                    onClicked: rootWindow.pushPage("qrc:/qml/VPN/Main.qml")
                }

                NavigationItem {
                    // Shown until NetworkManager has been read, so the row doesn't pop in late.
                    visible: !enabledConnections.ready || handler.hotspotCapable
                    text: qsTr("Hotspot")
                    leading: RowIcon {
                        source: "qrc:/fishui/kit/images/icons/network-wireless-hotspot.svg"
                        tint: "#2EBC6B"
                    }
                    onClicked: rootWindow.pushPage("qrc:/qml/Hotspot/Main.qml")
                }
            }
        }
    }
}
