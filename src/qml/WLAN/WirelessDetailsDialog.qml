// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.4
import QtQuick.Window 2.3
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import Cutefish.Settings 1.0
import FishUI 1.0 as FishUI
import Cutefish.Network 1.0 as NM
import "../"

FishUI.Window {
    id: control
    title: model.itemUniqueName

    parentWindow: rootWindow
    x: parentWindow ? parentWindow.x + Math.round((parentWindow.width - width) / 2) : 0
    y: parentWindow ? parentWindow.y + Math.round((parentWindow.height - height) / 2) : 0
    width: 400
    property int contentHeight: detailsLayout.implicitHeight + header.height + 32
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

    headerItem: FishUI.Label {
        anchors.left: parent.left
        anchors.leftMargin: Metrics.dialogPadding
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width - 64
        text: control.title
        textFormat: Text.PlainText
        font.pixelSize: 14
        font.weight: Font.DemiBold
        color: FishUI.Theme.textColor
        elide: Text.ElideRight
    }

    // Only a saved network has settings to auto-join with or forget.
    readonly property bool saved: !!model.connectionPath

    signal forgetBtnClicked()

    NM.WirelessItemSettings {
        id: settings
    }

    Component.onCompleted: {
        if (!control.saved)
            return

        // The sample networks have no NetworkManager connection behind them.
        if (!rootWindow.mockNetworkData)
            settings.path = model.connectionPath
        autoJoinSwitch.checked = rootWindow.mockNetworkData || settings.autoConnect
    }

    // Created per click, so each one goes away with its window.
    onClosing: destroy()

    ColumnLayout {
        id: detailsLayout
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
        }

        ListCard {
            InfoRow {
                text: qsTr("Security")
                value: model.securityTypeString
            }

            InfoRow {
                text: qsTr("Signal")
                value: qsTr("%1%").arg(model.signal)
            }

            InfoRow {
                text: qsTr("IPv4 Address")
                value: model.ipAddress
            }

            InfoRow {
                text: qsTr("IPv6 Address")
                value: model.ipV6Address
            }

            InfoRow {
                text: qsTr("MAC Address")
                value: model.macAddress
            }

            InfoRow {
                text: qsTr("Gateway")
                value: model.gateWay
            }

            InfoRow {
                text: qsTr("DNS")
                value: model.nameServer
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
                        handler.removeConnection(model.connectionPath)
                    control.close()
                }
            }
        }
    }
}
