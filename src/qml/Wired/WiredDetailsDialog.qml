// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import Cutefish.Settings
import FishUI as FishUI
import Cutefish.Network as NM
import "../"
import "../Network"

// One wired profile: whether it connects by itself, its addresses, and its IP settings.
FishUI.DialogWindow {
    id: control
    title: ipSettingsShown ? qsTr("IP Settings") : (row.itemUniqueName || "")

    parentWindow: rootWindow
    width: 400
    contentHeight: (ipSettingsShown ? ipSettingsLoader.implicitHeight : detailsLayout.implicitHeight) + 32
    // The page colour, so the cards stand out as they do on a page.
    background.color: FishUI.Theme.backgroundColor
    // The background item reaches a grown window a frame or two after its buffer does.
    color: background.color

    // The IP settings take the details' place, with a way back in the title bar.
    property bool ipSettingsShown: false
    backButtonVisible: ipSettingsShown
    onBackClicked: ipSettingsShown = false

    // The profile shown, found by its connection while the list re-sorts.
    property alias listModel: tracker.model
    property alias connectionPath: tracker.value
    readonly property var row: tracker.row || ({})

    TrackedRow {
        id: tracker
        role: "connectionPath"
        property bool found: false

        onRowChanged: {
            if (row)
                found = true
            else if (found)
                control.close()
        }
    }

    NM.WirelessItemSettings {
        id: settings
    }

    Component.onCompleted: {
        // The sample networks have no NetworkManager connection behind them.
        if (!rootWindow.mockNetworkData)
            settings.path = connectionPath
        autoConnectSwitch.checked = rootWindow.mockNetworkData || settings.autoConnect
    }

    // Created per click, so each one goes away with its window.
    onClosing: destroy()

    Shortcut {
        sequence: "Escape"
        onActivated: {
            if (control.ipSettingsShown)
                control.ipSettingsShown = false
            else
                control.close()
        }
    }

    ColumnLayout {
        id: detailsLayout
        visible: !control.ipSettingsShown
        // Not filled to the window: the window height follows this layout's implicit
        // height a polish later, and a filled layout would be spread over the old height.
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: Metrics.dialogPadding
        spacing: Metrics.dialogSpacing

        ListCard {
            ListRow {
                interactive: false
                text: qsTr("Connect Automatically")

                FishUI.Switch {
                    id: autoConnectSwitch
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                    rightPadding: 0
                    rightInset: 0
                    onCheckedChanged: {
                        if (!rootWindow.mockNetworkData)
                            settings.autoConnect = checked
                    }
                }
            }

            NavigationItem {
                text: qsTr("IP Settings")
                enabled: !rootWindow.mockNetworkData
                onClicked: control.ipSettingsShown = true
            }
        }

        ListCard {
            // Only a connection that is up has addresses.
            visible: !!row.ipAddress || !!row.ipV6Address

            InfoRow {
                text: qsTr("IPv4 address")
                value: row.ipAddress
            }

            InfoRow {
                text: qsTr("IPv6 address")
                value: row.ipV6Address
            }

            InfoRow {
                text: qsTr("MAC address")
                value: row.macAddress
            }

            InfoRow {
                text: qsTr("Gateway")
                value: row.gateWay
            }

            InfoRow {
                text: qsTr("DNS")
                value: row.nameServer
            }
        }

        FishUI.ConfirmDialogButton {
            Layout.alignment: Qt.AlignRight
            text: qsTr("Close")
            onClicked: control.close()
        }
    }

    // Made each time it is shown, so it starts from the saved profile.
    Loader {
        id: ipSettingsLoader
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: Metrics.dialogPadding
        active: control.ipSettingsShown

        sourceComponent: IpSettingsView {
            connectionPath: control.connectionPath
            onFinished: control.ipSettingsShown = false
        }
    }
}
