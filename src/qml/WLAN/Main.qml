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
        }
    }
}
