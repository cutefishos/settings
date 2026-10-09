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

FishUI.Window {
    id: control
    title: ipSettingsShown ? qsTr("IP Settings") : (row.itemUniqueName || "")

    parentWindow: rootWindow
    x: parentWindow ? parentWindow.x + Math.round((parentWindow.width - width) / 2) : 0
    y: parentWindow ? parentWindow.y + Math.round((parentWindow.height - height) / 2) : 0
    width: 400
    property int contentHeight: (ipSettingsShown ? ipSettingsLoader.implicitHeight : detailsLayout.implicitHeight)
                                + header.height + 32
    height: contentHeight
    minimumWidth: width
    maximumWidth: width
    minimumHeight: contentHeight
    maximumHeight: contentHeight
    flags: Qt.Dialog | Qt.FramelessWindowHint
    modality: Qt.WindowModal
    visible: false
    minimizeButtonVisible: false
    header.height: 44
    contentTopMargin: header.height
    // The page colour, so the cards stand out as they do on a page.
    background.color: FishUI.Theme.backgroundColor
    // The background item reaches a grown window a frame or two after its buffer does.
    color: background.color

    // The IP settings take the details' place, with a way back in the title bar.
    property bool ipSettingsShown: false

    headerItem: Item {
        // Mirrors the close button: the same size and the same inset from its corner.
        NavigationButton {
            id: backButton
            x: control.windowButtonsRightMargin
            y: control.windowButtonsTopMargin
            width: control.header.buttonSize
            height: width
            visible: control.ipSettingsShown
            iconName: "chevron-left"
            onClicked: control.ipSettingsShown = false
        }

        // Placed by its capitals, centred on the window buttons: a centred line box sits low.
        FishUI.AlignedLabel {
            anchors.left: backButton.visible ? backButton.right : parent.left
            anchors.leftMargin: backButton.visible ? FishUI.Units.smallSpacing : Metrics.dialogPadding
            anchors.right: parent.right
            y: snap(textBaseline - baselineOffset)
            textBaseline: backButton.y + backButton.height / 2 + capHeight / 2
            text: control.title
            textFormat: Text.PlainText
            font.pixelSize: 14
            font.weight: Font.DemiBold
            color: FishUI.Theme.textColor
            elide: Text.ElideRight
        }
    }

    // The network shown, found by a role that its row keeps while the list re-sorts.
    property alias listModel: tracker.model
    property alias trackedRole: tracker.role
    property alias trackedValue: tracker.value
    readonly property var row: tracker.row || ({})

    // Only a saved network has settings to auto-join with or forget.
    readonly property bool saved: !!row.connectionPath

    signal forgetBtnClicked()

    TrackedRow {
        id: tracker
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

    onSavedChanged: {
        if (!control.saved)
            return

        // The sample networks have no NetworkManager connection behind them.
        if (!rootWindow.mockNetworkData)
            settings.path = row.connectionPath
        autoJoinSwitch.checked = rootWindow.mockNetworkData || settings.autoConnect
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
            visible: control.saved

            ListRow {
                interactive: false
                text: qsTr("Auto-Join")

                FishUI.Switch {
                    id: autoJoinSwitch
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
            InfoRow {
                text: qsTr("Security")
                value: row.securityTypeString
            }

            InfoRow {
                text: qsTr("Signal")
                value: row.signal !== undefined ? qsTr("%1%").arg(row.signal) : ""
            }

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

        RowLayout {
            id: footerLayout
            spacing: Metrics.dialogButtonSpacing
            Layout.alignment: Qt.AlignRight

            FishUI.ConfirmDialogButton {
                text: qsTr("Close")
                onClicked: control.close()
            }

            FishUI.ConfirmDialogButton {
                text: qsTr("Forget this network")
                visible: control.saved
                primary: true
                destructive: true
                onClicked: {
                    if (!rootWindow.mockNetworkData)
                        handler.removeConnection(row.connectionPath)
                    control.close()
                }
            }
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
            connectionPath: row.connectionPath
            onFinished: control.ipSettingsShown = false
        }
    }
}
