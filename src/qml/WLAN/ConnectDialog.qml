/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     Kate Leet <kate@cutefishos.com>
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

import QtQuick 2.15
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import FishUI 1.0 as FishUI
import Cutefish.Network 1.0 as NM

FishUI.Window {
    id: control

    windowButtonsTopMargin: 8
    windowButtonsRightMargin: 8
    header.height: 44
    contentTopMargin: 0

    width: contentWidth
    height: contentHeight
    minimumWidth: contentWidth
    minimumHeight: contentHeight
    maximumWidth: contentWidth
    maximumHeight: contentHeight

    property int contentWidth: 400
    property int contentHeight: _mainLayout.implicitHeight + 32

    title: "Enter Password"
    visible: false
    minimizeButtonVisible: false
    modality: Qt.WindowModal

    background.color: FishUI.Theme.secondBackgroundColor
    flags: Qt.Dialog | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint

    signal connect(var devicePath, var specificPath, var password)

    property var name: ""
    property var devicePath: ""
    property var specificPath: ""
    property var securityType: ""

    onVisibleChanged: {
        if (visible) {
            passwordField.clear()
            passwordField.forceActiveFocus()
        }
    }

    ColumnLayout {
        id: _mainLayout
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16

        Label {
            Layout.fillWidth: true
            Layout.rightMargin: 32
            text: control.title
            textFormat: Text.PlainText
            font.pixelSize: 14
            font.weight: Font.DemiBold
            color: FishUI.Theme.textColor
            wrapMode: Text.Wrap
        }

        Label {
            Layout.fillWidth: true
            textFormat: Text.PlainText
            font.pixelSize: 14
            text: qsTr("Enter the password for %1").arg(control.name)
            color: FishUI.Theme.disabledTextColor
            wrapMode: Text.WordWrap
        }

        TextField {
            id: passwordField
            focus: true
            echoMode: TextInput.Password
            selectByMouse: true
            placeholderText: qsTr("Password")

            validator: RegularExpressionValidator {
                regularExpression: {
                    if (control.securityType === NM.Enums.StaticWep)
                        return /^(?:[\x20-\x7F]{5}|[0-9a-fA-F]{10}|[\x20-\x7F]{13}|[0-9a-fA-F]{26}){1}$/;
                    return /^(?:[\x20-\x7F]{8,64}){1}$/;
                }
            }

            onAccepted: {
                control.emitSignal()
            }

            Keys.onEscapePressed: {
                control.visible = false
            }

            Layout.fillWidth: true
        }

        RowLayout {
            spacing: 8
            Layout.alignment: Qt.AlignRight

            FishUI.ConfirmDialogButton {
                text: qsTr("Cancel")
                onClicked: control.visible = false
            }

            FishUI.ConfirmDialogButton {
                text: qsTr("Connect")
                primary: true
                enabled: passwordField.acceptableInput
                onClicked: control.emitSignal()
            }
        }
    }

    function emitSignal() {
        control.connect(control.devicePath, control.specificPath, passwordField.text)
        control.visible = false
    }
}
