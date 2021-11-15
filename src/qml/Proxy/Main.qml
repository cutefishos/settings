/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     Reion Wong <reion@cutefishos.com>
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

import QtQuick 2.12
import QtQml 2.15
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.2

import FishUI 1.0 as FishUI
import Cutefish.Settings 1.0
import "../"

ItemPage {
    headerTitle: qsTr("Proxy")

    Binding {
        target: ftpProxyTextField
        property: "text"
        value: httpProxyField.text
        when: forFtpCheckBox.checked
        restoreMode: Binding.RestoreNone
    }

    Binding {
        target: ftpProxyPortField
        property: "text"
        value: httpProxyPortField.text
        when: forFtpCheckBox.checked
        restoreMode: Binding.RestoreNone
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent
            spacing: FishUI.Units.largeSpacing * 2

            RoundedItem {
                id: mainItem
                spacing: FishUI.Units.largeSpacing

                RadioButton {
                    id: noProxyRadioButton
                    checked: true
                    text: qsTr("No Proxy")
                }

                RadioButton {
                    id: autoDiscoverProxyRadioButton
                    text: qsTr("Detect proxy configuration automatically")
                }

                RadioButton {
                    id: autoScriptProxyRadioButton
                    text: qsTr("Use proxy auto configuration URL")
                }

                RadioButton {
                    id: manualProxyRadioButton
                    text: qsTr("Use manually specified proxy configuration")
                }
            }

            RoundedItem {
                id: proxyItem
                visible: !noProxyRadioButton.checked && !autoDiscoverProxyRadioButton.checked

                RowLayout {
                    id: autoScriptProxyLayout
                    spacing: FishUI.Units.largeSpacing
                    visible: autoScriptProxyRadioButton.checked

                    TextField {
                        id: autoScriptField
                        Layout.fillWidth: true
                        height: 40
                    }

                    Button {
                        text: qsTr("Select file")
                        flat: true
                        onClicked: fileDialog.open()
                    }
                }

                // ----------------------
                GridLayout {
                    visible: manualProxyRadioButton.checked
                    columns: 4
                    columnSpacing: FishUI.Units.largeSpacing
                    rowSpacing: FishUI.Units.largeSpacing

                    Label {
                        text: qsTr("HTTP Proxy")
                    }

                    TextField {
                        id: httpProxyField
                        Layout.fillWidth: true
                        height: 40
                    }

                    Label {
                        text: qsTr("Port")
                    }

                    TextField {
                        id: httpProxyPortField
                        height: 40
                        Layout.preferredWidth: 80
                    }

                    Item {
                        width: 1
                    }

                    CheckBox {
                        id: forFtpCheckBox
                        text: qsTr("Also use this proxy for FTP")
                        Layout.fillWidth: true
                        checked: true
                    }

                    Item {
                        width: 1
                    }

                    Item {
                        width: 1
                    }

                    // FTP
                    Label {
                        text: qsTr("FTP Proxy")
                    }

                    TextField {
                        id: ftpProxyTextField
                        Layout.fillWidth: true
                        height: 40
                        enabled: !forFtpCheckBox.checked
                    }

                    Label {
                        text: qsTr("Port")
                    }

                    TextField {
                        id: ftpProxyPortField
                        height: 40
                        Layout.preferredWidth: 80
                        enabled: !forFtpCheckBox.checked
                    }

                    // SOCKS
                    Label {
                        text: qsTr("SOCKS Proxy")
                    }

                    TextField {
                        Layout.fillWidth: true
                        height: 40
                    }

                    Label {
                        text: qsTr("Port")
                    }

                    TextField {
                        height: 40
                        Layout.preferredWidth: 80
                    }
                }
            }

            Item {
                height: FishUI.Units.smallSpacing
            }
        }
    }

    FileDialog {
        id: fileDialog
        onAccepted: {
            autoScriptField.text = fileDialog.fileUrl.toString().replace("file://", "")
        }
    }
}
