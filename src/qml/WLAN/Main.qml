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

        onConnect: function(ssid, username, pwd, type) {
            wifiSettings.addOtherConnection(ssid, username, pwd, type)
        }
        onJoinNetwork: function(device, accessPoint, username, pwd) {
            handler.addAndActivateEnterprise(device, accessPoint, username, pwd)
        }
    }

    Component.onCompleted: {
        if (!rootWindow.mockNetworkData)
            handler.requestScan()
        joinRequestedNetwork()
    }

    // The status bar sends enterprise networks here. The window and NetworkManager's
    // objects may not be there yet when the request arrives.
    function joinRequestedNetwork() {
        if (!rootWindow.pendingWifiJoin || !rootWindow.revealed || !enabledConnections.ready)
            return
        var accessPoint = rootWindow.pendingWifiJoin
        rootWindow.pendingWifiJoin = ""
        newNetworkDialog.openNetwork(accessPoint, handler.wirelessNetwork(accessPoint))
    }

    Connections {
        target: rootWindow
        function onPendingWifiJoinChanged() { control.joinRequestedNetwork() }
        function onRevealedChanged() { control.joinRequestedNetwork() }
    }

    Connections {
        target: enabledConnections
        function onReadyChanged() { control.joinRequestedNetwork() }
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
        }
    }
}
