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
        if (control.saved) {
            settings.path = model.connectionPath
            autoJoinSwitch.checked = settings.autoConnect
        }
    }

    // Created per click, so each one goes away with its window.
    onClosing: destroy()

    ColumnLayout {
        id: detailsLayout
        anchors.fill: parent
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
                    onCheckedChanged: settings.autoConnect = checked
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
                    handler.removeConnection(model.connectionPath)
                    control.close()
                }
            }
        }
    }
}
