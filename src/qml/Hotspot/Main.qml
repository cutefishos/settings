// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Cutefish.Settings
import FishUI as FishUI
import Cutefish.Network as NM

import "../"

// Shares this computer's internet connection over a Wi-Fi network of its own. The name
// and password are set while the hotspot is off and take effect when it starts.
ItemPage {
    id: control
    headerTitle: qsTr("Hotspot")

    property string errorMessage
    readonly property string description: qsTr("Share this computer's internet connection by creating a Wi-Fi network other devices can join. The connection to share must come from a source other than Wi-Fi.")

    readonly property bool nameValid: configuration.hotspotNameValid(nameField.text)
    readonly property bool passwordValid: configuration.hotspotPasswordValid(passwordField.text)

    NM.Handler {
        id: handler

        onHotspotFailed: (message) => control.errorMessage = message
        onHotspotActiveChanged: {
            if (hotspotActive)
                control.errorMessage = ""
        }
    }

    NM.Configuration {
        id: configuration
    }

    NM.EnabledConnections {
        id: enabledConnections
    }

    function start() {
        configuration.hotspotName = nameField.text
        configuration.hotspotPassword = passwordField.text
        errorMessage = ""
        handler.createHotspot()
    }

    function requestStart() {
        if (handler.hotspotReplaces.length > 0)
            confirmDialog.createObject(control)
        else
            start()
    }

    Component {
        id: confirmDialog

        FishUI.ConfirmDialog {
            title: qsTr("Turn On Wi-Fi Hotspot?")
            text: qsTr("Turning on the hotspot will disconnect from “%1”, and you will not have access to the internet it provides.")
                  .arg(handler.hotspotReplaces)
            cancelText: qsTr("Cancel")
            confirmText: qsTr("Turn on")
            onAccepted: control.start()
            onClosing: destroy()
        }
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent
            spacing: Metrics.sectionLabelSpacing

            ListCard {
                ListRow {
                    interactive: false
                    text: qsTr("Wi-Fi hotspot")

                    FishUI.Switch {
                        id: hotspotSwitch
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                        rightPadding: 0
                        rightInset: 0
                        enabled: handler.hotspotActive
                                 || (handler.hotspotSupported && control.nameValid && control.passwordValid)
                        checked: handler.hotspotActive
                        Accessible.name: qsTr("Wi-Fi hotspot")
                        onToggled: {
                            if (checked)
                                control.requestStart()
                            else
                                handler.stopHotspot()
                            // The switch shows what NetworkManager reports, not what was asked for.
                            checked = Qt.binding(function() { return handler.hotspotActive })
                        }
                    }
                }
            }

            FishUI.Label {
                Layout.fillWidth: true
                Layout.leftMargin: Metrics.cardHorizontalPadding
                Layout.rightMargin: Metrics.cardHorizontalPadding
                text: control.errorMessage.length > 0 ? control.errorMessage
                      : !enabledConnections.ready ? control.description
                      : !enabledConnections.wirelessEnabled ? qsTr("Turn on Wi-Fi to use the hotspot.")
                      : !handler.hotspotAllowed ? qsTr("System policy prohibits use as a hotspot.")
                      : handler.hotspotActive ? qsTr("Other devices can join “%1” with the password below. Turn off the hotspot to use Wi-Fi on this computer.")
                                                .arg(configuration.hotspotName)
                      : control.description
                textFormat: Text.PlainText
                color: control.errorMessage.length > 0 ? FishUI.Theme.redColor : FishUI.Theme.disabledTextColor
                wrapMode: Text.Wrap
            }

            ListCard {
                Layout.topMargin: Metrics.sectionSpacing - Metrics.sectionLabelSpacing

                ListRow {
                    interactive: false
                    minimumRowHeight: Metrics.controlRowHeight
                    text: qsTr("Network name")

                    FishUI.TextField {
                        id: nameField
                        Layout.preferredWidth: 220
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                        enabled: !handler.hotspotActive
                        text: configuration.hotspotName
                        selectByMouse: true
                        onEditingFinished: {
                            if (control.nameValid)
                                configuration.hotspotName = text
                        }
                    }
                }

                ListRow {
                    interactive: false
                    minimumRowHeight: Metrics.controlRowHeight
                    text: qsTr("Password")

                    FishUI.TextField {
                        id: passwordField
                        Layout.preferredWidth: 220
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                        rightPadding: FishUI.Units.largeSpacing + generateButton.width + FishUI.Units.smallSpacing
                        enabled: !handler.hotspotActive
                        text: configuration.hotspotPassword
                        maximumLength: 64
                        selectByMouse: true
                        onEditingFinished: {
                            if (control.passwordValid)
                                configuration.hotspotPassword = text
                        }

                        // Inside the field, its ink inset from the edge as far as the text is.
                        IconButton {
                            id: generateButton
                            width: 18
                            anchors.right: parent.right
                            anchors.rightMargin: FishUI.Units.largeSpacing + Metrics.trailingGlyphMargin(iconName, width)
                            anchors.verticalCenter: parent.verticalCenter
                            iconName: "refresh"
                            iconColor: FishUI.Theme.highlightColor
                            Accessible.name: qsTr("Generate password")
                            onClicked: {
                                passwordField.text = configuration.generatePassword()
                                configuration.hotspotPassword = passwordField.text
                            }
                        }
                    }
                }
            }

            FishUI.Label {
                Layout.fillWidth: true
                Layout.leftMargin: Metrics.cardHorizontalPadding
                Layout.rightMargin: Metrics.cardHorizontalPadding
                visible: text.length > 0
                text: !control.nameValid
                      ? (nameField.text.length === 0 ? qsTr("Network name cannot be empty.")
                                                     : qsTr("Network name is too long."))
                      : !control.passwordValid
                      ? (passwordField.text.length < 8 ? qsTr("The password must have at least 8 characters.")
                                                       : qsTr("The password must have at most 63 characters."))
                      : ""
                textFormat: Text.PlainText
                color: FishUI.Theme.redColor
                wrapMode: Text.Wrap
            }
        }
    }
}
