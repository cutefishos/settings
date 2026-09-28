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

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Cutefish.Settings 1.0
import FishUI 1.0 as FishUI
import Cutefish.Network 1.0 as NM

FishUI.Window {
    id: control

    windowButtonsTopMargin: Metrics.dialogWindowButtonsMargin
    windowButtonsRightMargin: Metrics.dialogWindowButtonsMargin
    header.height: 44
    contentTopMargin: 0

    width: contentWidth
    height: contentHeight
    minimumWidth: contentWidth
    minimumHeight: contentHeight
    maximumWidth: contentWidth
    maximumHeight: contentHeight
    modality: Qt.WindowModal

    property int contentWidth: 400
    property int contentHeight: _mainLayout.implicitHeight + 32

    title: qsTr("Add Other Network")
    visible: false
    minimizeButtonVisible: false

    background.color: FishUI.Theme.secondBackgroundColor
    flags: Qt.Dialog | Qt.FramelessWindowHint

    readonly property string security: securitybox.currentValue ?? ""
    readonly property bool enterprise: security.endsWith("-eap")
    readonly property bool canJoin: _nameTextField.text.length > 0
                                    && (!enterprise || _userNameTextField.text.length > 0)
                                    && passwordValid(_passwordTextField.text)

    signal connect(var ssid, var username, var pwd, var type)

    // What NetworkManager accepts for each kind of key.
    function passwordValid(password) {
        switch (security) {
        case "none":
            return true
        case "wep":
            return [5, 10, 13, 26].indexOf(password.length) >= 0
        case "wpa":
        case "wpa2-wpa3":
        case "wpa3":
            return password.length >= 8 && password.length <= 64
        default:
            return password.length > 0
        }
    }

    function join() {
        if (!canJoin)
            return

        control.connect(_nameTextField.text,
                        enterprise ? _userNameTextField.text : "",
                        security === "none" ? "" : _passwordTextField.text,
                        security)
        control.close()
    }

    onVisibleChanged: {
        if (visible) {
            _nameTextField.forceActiveFocus()
        } else {
            _nameTextField.clear()
            _userNameTextField.clear()
            _passwordTextField.clear()
            securitybox.currentIndex = 3
        }
    }

    ColumnLayout {
        id: _mainLayout
        anchors.fill: parent
        anchors.margins: Metrics.dialogPadding
        spacing: Metrics.dialogSpacing

        FishUI.Label {
            Layout.fillWidth: true
            Layout.rightMargin: Metrics.xxl
            text: control.title
            textFormat: Text.PlainText
            font.pixelSize: 14
            font.weight: Font.DemiBold
            color: FishUI.Theme.textColor
            wrapMode: Text.Wrap
        }

        GridLayout {
            Layout.fillWidth: true
            columns: 2
            columnSpacing: Metrics.dialogSpacing
            rowSpacing: Metrics.formRowSpacing

            FishUI.Label {
                text: qsTr("Name")
            }

            FishUI.TextField {
                id: _nameTextField
                placeholderText: qsTr("Network Name")
                Layout.fillWidth: true
                Keys.onEscapePressed: control.close()
                Keys.onReturnPressed: control.join()
                Keys.onEnterPressed: control.join()
            }

            FishUI.Label {
                text: qsTr("Security")
            }

            FishUI.ComboBox {
                id: securitybox
                Layout.fillWidth: true
                currentIndex: 3
                textRole: "text"
                valueRole: "value"
                model: [
                    { text: qsTr("Open"), value: "none" },
                    { text: "WEP", value: "wep" },
                    { text: "WPA", value: "wpa" },
                    { text: "WPA2/WPA3", value: "wpa2-wpa3" },
                    { text: "WPA3", value: "wpa3" },
                    { text: qsTr("WPA Enterprise"), value: "wpa-eap" },
                    { text: qsTr("WPA2 Enterprise"), value: "wpa2-eap" },
                    { text: qsTr("WPA3 Enterprise"), value: "wpa3-eap" }
                ]
            }

            FishUI.Label {
                visible: control.enterprise
                text: qsTr("Username")
            }

            FishUI.TextField {
                id: _userNameTextField
                visible: control.enterprise
                placeholderText: qsTr("Username")
                Layout.fillWidth: true
                Keys.onEscapePressed: control.close()
                Keys.onReturnPressed: control.join()
                Keys.onEnterPressed: control.join()
            }

            FishUI.Label {
                visible: control.security !== "none"
                text: qsTr("Password")
            }

            FishUI.TextField {
                id: _passwordTextField
                visible: control.security !== "none"
                placeholderText: qsTr("Password")
                echoMode: TextInput.Password
                Layout.fillWidth: true
                Keys.onEscapePressed: control.close()
                Keys.onReturnPressed: control.join()
                Keys.onEnterPressed: control.join()
            }
        }

        RowLayout {
            spacing: Metrics.dialogButtonSpacing
            Layout.alignment: Qt.AlignRight

            FishUI.ConfirmDialogButton {
                text: qsTr("Cancel")
                onClicked: control.close()
            }

            FishUI.ConfirmDialogButton {
                enabled: control.canJoin
                primary: true
                text: qsTr("Join")
                onClicked: control.join()
            }
        }
    }
}
