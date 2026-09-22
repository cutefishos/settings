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

import QtQuick 2.4
import QtQuick.Window 2.3
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import FishUI 1.0 as FishUI
import Cutefish.Network 1.0 as NM

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
    background.color: FishUI.Theme.secondBackgroundColor

    headerItem: FishUI.Label {
        anchors.left: parent.left
        anchors.leftMargin: 16
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width - 64
        text: control.title
        textFormat: Text.PlainText
        font.pixelSize: 14
        font.weight: Font.DemiBold
        color: FishUI.Theme.textColor
        elide: Text.ElideRight
    }

    signal forgetBtnClicked()

    NM.WirelessItemSettings {
        id: settings
    }

    Component.onCompleted: {
        if (model.connectionPath) {
            settings.path = model.connectionPath
            autoJoinSwitch.checked = settings.autoConnect
            autoJoinSwitch.visible = true
            autoJoinLabel.visible = true
        }
    }

    ColumnLayout {
        id: detailsLayout
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16

        GridLayout {
            Layout.fillWidth: true
            id: gridLayout
            columns: 2
            columnSpacing: 16
            rowSpacing: 8

            FishUI.Label {
                id: autoJoinLabel
                text: qsTr("Auto-Join")
                visible: false
                Layout.alignment: Qt.AlignRight
                color: FishUI.Theme.disabledTextColor
            }

            FishUI.Switch {
                id: autoJoinSwitch
                rightPadding: 0
                rightInset: 0
                Layout.fillHeight: true
                visible: false
                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                onCheckedChanged: settings.autoConnect = checked
            }

            FishUI.Label {
                text: qsTr("Security")
                color: FishUI.Theme.disabledTextColor
                Layout.alignment: Qt.AlignRight
            }

            FishUI.Label {
                id: securityLabel
                text: model.securityTypeString
                Layout.alignment: Qt.AlignRight
            }

            FishUI.Label {
                text: qsTr("Signal")
                color: FishUI.Theme.disabledTextColor
                Layout.alignment: Qt.AlignRight
            }

            FishUI.Label {
                id: signalLabel
                text: model.signal
                Layout.alignment: Qt.AlignRight
            }

            FishUI.Label {
                text: qsTr("IPv4 Address")
                color: FishUI.Theme.disabledTextColor
                Layout.alignment: Qt.AlignRight
            }

            FishUI.Label {
                id: ipv4AddressLabel
                text: model.ipAddress
                Layout.alignment: Qt.AlignRight
                Layout.fillWidth: true
            }

            FishUI.Label {
                text: qsTr("IPv6 Address")
                color: FishUI.Theme.disabledTextColor
                Layout.alignment: Qt.AlignRight
            }

            FishUI.Label {
                id: ipV6AddressLabel
                // text: model.ipV6Address
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignRight
            }

            FishUI.Label {
                text: qsTr("MAC Address")
                color: FishUI.Theme.disabledTextColor
                Layout.alignment: Qt.AlignRight
            }

            FishUI.Label {
                id: macAddressLabel
                // text: model.macAddress
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignRight
            }

            FishUI.Label {
                text: qsTr("Gateway")
                color: FishUI.Theme.disabledTextColor
                Layout.alignment: Qt.AlignRight
            }

            FishUI.Label {
                id: routerLabel
                text: model.gateWay
                Layout.alignment: Qt.AlignRight
            }

            FishUI.Label {
                text: qsTr("DNS")
                color: FishUI.Theme.disabledTextColor
                Layout.alignment: Qt.AlignRight
            }

            FishUI.Label {
                id: dnsLabel
                // text: model.nameServer
                Layout.alignment: Qt.AlignRight
            }
        }

        RowLayout {
            id: footerLayout
            spacing: 8
            Layout.alignment: Qt.AlignRight

            FishUI.ConfirmDialogButton {
                text: qsTr("Close")
                onClicked: control.close()
            }

            FishUI.ConfirmDialogButton {
                text: qsTr("Forget this network")
                primary: true
                destructive: true
                onClicked: {
                    handler.removeConnection(model.connectionPath)
                    control.close()
                }
            }
        }
    }
}
