// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQml
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

import FishUI as FishUI
import Cutefish.Settings
import "../"

ItemPage {
    headerTitle: qsTr("Proxy")

    Binding {
        target: ftpProxyTextField
        property: "text"
        value: httpProxyField.text
        when: forFtpCheckBox.checked
        restoreMode: Binding.RestoreValue
    }

    NetworkProxy {
        id: networkProxy
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
            spacing: Metrics.sectionSpacing

            RoundedItem {
                id: mainItem

                FishUI.RadioButton {
                    id: noProxyRadioButton
                    // The circle, not the button's padding, starts on the text column.
                    horizontalPadding: 0
                    checked: networkProxy.flag === 0
                    text: qsTr("No Proxy")
                    onClicked: {
                        networkProxy.flag = 0
                    }
                }

                FishUI.RadioButton {
                    id: autoScriptProxyRadioButton
                    horizontalPadding: 0
                    checked: networkProxy.flag === 1
                    text: qsTr("Use proxy auto configuration URL")
                    visible: false
                    onClicked: {
                        networkProxy.flag = 1
                    }
                }

                FishUI.RadioButton {
                    id: manualProxyRadioButton
                    horizontalPadding: 0
                    checked: networkProxy.flag === 2
                    text: qsTr("Use manually specified proxy configuration")
                    onClicked: {
                        networkProxy.flag = 2
                    }
                }
            }

            RoundedItem {
                id: proxyItem
                visible: !noProxyRadioButton.checked

                property bool needSave: false

                Component.onCompleted: needSave = false

                RowLayout {
                    id: autoScriptProxyLayout
                    spacing: Metrics.controlSpacing
                    visible: autoScriptProxyRadioButton.checked

                    FishUI.TextField {
                        id: autoScriptField
                        Layout.fillWidth: true
                        text: networkProxy.scriptProxy
                        height: 40

                        onTextChanged: {
                            proxyItem.needSave = true
                        }
                    }

                    FishUI.Button {
                        text: qsTr("Select file")
                        flat: true
                        onClicked: fileDialog.open()
                    }
                }

                // ----------------------
                GridLayout {
                    visible: manualProxyRadioButton.checked
                    columns: 4
                    columnSpacing: Metrics.rowSpacing
                    rowSpacing: Metrics.formRowSpacing

                    FishUI.Label {
                        text: qsTr("HTTP Proxy")
                    }

                    FishUI.TextField {
                        id: httpProxyField
                        Layout.fillWidth: true
                        height: 40
                        text: networkProxy.httpProxy
                        onTextChanged: {
                            proxyItem.needSave = true
                        }
                    }

                    FishUI.Label {
                        text: qsTr("Port")
                    }

                    FishUI.TextField {
                        id: httpProxyPortField
                        height: 40
                        Layout.preferredWidth: 80

                        text: networkProxy.httpProxyPort

                        validator: IntValidator { bottom: 0; top: 999999}

                        onTextChanged: {
                            proxyItem.needSave = true
                        }
                    }

                    Item {
                        width: 1
                    }

                    FishUI.CheckBox {
                        id: forFtpCheckBox
                        text: qsTr("Also use this proxy for FTP")
                        Layout.fillWidth: true
                        checked: networkProxy.useSameProxy
                        onClicked: {
                            proxyItem.needSave = true
                        }
                    }

                    Item {
                        width: 1
                    }

                    Item {
                        width: 1
                    }

                    // FTP
                    FishUI.Label {
                        text: qsTr("FTP Proxy")
                    }

                    FishUI.TextField {
                        id: ftpProxyTextField
                        Layout.fillWidth: true
                        height: 40
                        enabled: !forFtpCheckBox.checked
                        text: networkProxy.ftpProxy
                        onTextChanged: {
                            proxyItem.needSave = true
                        }
                    }

                    FishUI.Label {
                        text: qsTr("Port")
                    }

                    FishUI.TextField {
                        id: ftpProxyPortField
                        height: 40
                        Layout.preferredWidth: 80
                        enabled: !forFtpCheckBox.checked

                        text: networkProxy.ftpProxyPort
                        validator: IntValidator { bottom: 0; top: 999999}

                        onTextChanged: {
                            proxyItem.needSave = true
                        }
                    }

                    // SOCKS
                    FishUI.Label {
                        text: qsTr("SOCKS Proxy")
                    }

                    FishUI.TextField {
                        id: socksProxyField
                        Layout.fillWidth: true
                        height: 40
                        text: networkProxy.socksProxy

                        onTextChanged: {
                            proxyItem.needSave = true
                        }
                    }

                    FishUI.Label {
                        text: qsTr("Port")
                    }

                    FishUI.TextField {
                        id: socksProxyPortField
                        height: 40
                        Layout.preferredWidth: 80

                        text: networkProxy.socksProxyPort
                        validator: IntValidator { bottom: 0; top: 999999}

                        onTextChanged: {
                            proxyItem.needSave = true
                        }
                    }
                }

                HorizontalDivider {
                    visible: proxyItem.needSave
                }

                RowLayout {
                    spacing: Metrics.rowSpacing
                    visible: proxyItem.needSave

                    Item {
                        Layout.fillWidth: true
                    }

                    FishUI.Button {
                        text: qsTr("Save")

                        onClicked: {
                            proxyItem.needSave = false
                            networkProxy.scriptProxy = autoScriptField.text
                            networkProxy.httpProxy = httpProxyField.text
                            networkProxy.httpProxyPort = httpProxyPortField.text
                            networkProxy.useSameProxy = forFtpCheckBox.checked
                            networkProxy.ftpProxy = ftpProxyTextField.text
                            networkProxy.ftpProxyPort = ftpProxyPortField.text
                            networkProxy.socksProxy = socksProxyField.text
                            networkProxy.socksProxyPort = socksProxyPortField.text
                        }
                    }
                }
            }

        }
    }

    FileDialog {
        id: fileDialog
        onAccepted: {
            autoScriptField.text = decodeURIComponent(fileDialog.selectedFile.toString().replace("file://", ""))
            networkProxy.scriptProxy = autoScriptField.text
        }
    }
}
