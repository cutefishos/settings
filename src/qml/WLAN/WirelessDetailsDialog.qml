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

Dialog {
    id: control
    title: model.itemUniqueName

    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    width: 400
    padding: 16
    spacing: 16
    modal: true

    background: Rectangle {
        radius: 16
        color: FishUI.Theme.secondBackgroundColor
    }

    header: Label {
        text: control.title
        textFormat: Text.PlainText
        padding: 16
        bottomPadding: 0
        font.pixelSize: 14
        font.weight: Font.DemiBold
        color: FishUI.Theme.textColor
        wrapMode: Text.Wrap
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

    contentItem: ColumnLayout {
        id: detailsLayout
        spacing: 16

        GridLayout {
            Layout.fillWidth: true
            id: gridLayout
            columns: 2
            columnSpacing: 16
            rowSpacing: 8

            Label {
                id: autoJoinLabel
                text: qsTr("Auto-Join")
                visible: false
                Layout.alignment: Qt.AlignRight
                color: FishUI.Theme.disabledTextColor
            }

            Switch {
                id: autoJoinSwitch
                rightPadding: 0
                Layout.fillHeight: true
                visible: false
                Layout.alignment: Qt.AlignRight
                onCheckedChanged: settings.autoConnect = checked
            }

            Label {
                text: qsTr("Security")
                color: FishUI.Theme.disabledTextColor
                Layout.alignment: Qt.AlignRight
            }

            Label {
                id: securityLabel
                text: model.securityTypeString
                Layout.alignment: Qt.AlignRight
            }

            Label {
                text: qsTr("Signal")
                color: FishUI.Theme.disabledTextColor
                Layout.alignment: Qt.AlignRight
            }

            Label {
                id: signalLabel
                text: model.signal
                Layout.alignment: Qt.AlignRight
            }

            Label {
                text: qsTr("IPv4 Address")
                color: FishUI.Theme.disabledTextColor
                Layout.alignment: Qt.AlignRight
            }

            Label {
                id: ipv4AddressLabel
                // text: model.ipV4Address
                Layout.alignment: Qt.AlignRight
                Layout.fillWidth: true
            }

            Label {
                text: qsTr("IPv6 Address")
                color: FishUI.Theme.disabledTextColor
                Layout.alignment: Qt.AlignRight
            }

            Label {
                id: ipV6AddressLabel
                // text: model.ipV6Address
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignRight
            }

            Label {
                text: qsTr("MAC Address")
                color: FishUI.Theme.disabledTextColor
                Layout.alignment: Qt.AlignRight
            }

            Label {
                id: macAddressLabel
                // text: model.macAddress
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignRight
            }

            Label {
                text: qsTr("Gateway")
                color: FishUI.Theme.disabledTextColor
                Layout.alignment: Qt.AlignRight
            }

            Label {
                id: routerLabel
                // text: model.gateway
                Layout.alignment: Qt.AlignRight
            }

            Label {
                text: qsTr("DNS")
                color: FishUI.Theme.disabledTextColor
                Layout.alignment: Qt.AlignRight
            }

            Label {
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
                onClicked: control.reject()
            }

            FishUI.ConfirmDialogButton {
                text: qsTr("Forget this network")
                primary: true
                destructive: true
                onClicked: {
                    handler.removeConnection(model.connectionPath)
                    control.reject()
                }
            }
        }
    }
}
