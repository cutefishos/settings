// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cutefish.Settings
import FishUI as FishUI
import Cutefish.Network as NM
import "../"

// How a saved network gets its addresses and name servers, shown in place of its details.
// Save writes the profile and applies it to the connection at once; finished() follows
// a save or Cancel.
ColumnLayout {
    id: control

    property string connectionPath

    property var form: ({})
    property bool saving: false
    property string error
    // Bumped on each edit, so the check runs again.
    property int revision: 0
    // One label column for every card, measured as the labels appear.
    property real labelWidth: 0

    readonly property bool ipv4Manual: _ipv4Method.currentValue === "manual"
    readonly property bool ipv6Manual: _ipv6Method.currentValue === "manual"
    // Only a family that asks the network for its addresses can be told its name servers too.
    readonly property bool automaticFamily: ["auto", "dhcp"].indexOf(_ipv4Method.currentValue) >= 0
                                            || ["auto", "dhcp"].indexOf(_ipv6Method.currentValue) >= 0
    readonly property var result: {
        revision
        return handler.checkIpForm(collect())
    }

    signal finished()

    spacing: Metrics.dialogSpacing

    NM.Handler {
        id: handler

        onIpSaved: (connection) => {
            if (connection !== control.connectionPath || !control.saving)
                return
            control.saving = false
            control.finished()
        }

        onIpSaveFailed: (connection, message) => {
            if (connection !== control.connectionPath || !control.saving)
                return
            control.saving = false
            control.error = message
        }
    }

    Component.onCompleted: load()

    function measure(width) {
        labelWidth = Math.max(labelWidth, Math.min(width, 140))
    }

    function load() {
        form = handler.ipForm(connectionPath)
        _ipv4Method.currentIndex = Math.max(0, _ipv4Method.indexOfValue(form.ipv4Method))
        _ipv4Address.text = form.ipv4Address
        _ipv4Netmask.text = form.ipv4Netmask
        _ipv4Gateway.text = form.ipv4Gateway
        _ipv6Method.currentIndex = Math.max(0, _ipv6Method.indexOfValue(form.ipv6Method))
        _ipv6Address.text = form.ipv6Address
        _ipv6Prefix.text = form.ipv6Prefix
        _ipv6Gateway.text = form.ipv6Gateway
        _automaticDns.checked = form.automaticDns
        _dns.text = form.dns
        revision++
    }

    function collect() {
        return {
            ipv4Method: _ipv4Method.currentValue ?? "",
            ipv4Address: _ipv4Address.text,
            ipv4Netmask: _ipv4Netmask.text,
            ipv4Gateway: _ipv4Gateway.text,
            ipv6Method: _ipv6Method.currentValue ?? "",
            ipv6Address: _ipv6Address.text,
            ipv6Prefix: _ipv6Prefix.text,
            ipv6Gateway: _ipv6Gateway.text,
            automaticDns: _automaticDns.checked,
            dns: _dns.text
        }
    }

    // A manual setup starts from the addresses the connection has now, as they usually
    // are what is to be kept.
    function fillFromCurrent(family) {
        const current = form.current || {}
        const fields = family === "ipv4" ? [[_ipv4Address, "ipv4Address"], [_ipv4Netmask, "ipv4Netmask"], [_ipv4Gateway, "ipv4Gateway"]]
                                         : [[_ipv6Address, "ipv6Address"], [_ipv6Prefix, "ipv6Prefix"], [_ipv6Gateway, "ipv6Gateway"]]
        if (fields[0][0].text.length > 0 || !current[fields[0][1]])
            return
        for (const field of fields)
            field[0].text = current[field[1]] || ""
    }

    function save() {
        if (saving || !result.complete)
            return
        saving = true
        error = ""
        handler.saveIp(connectionPath, collect())
    }

    component FieldLabel: FishUI.Label {
        Layout.preferredWidth: control.labelWidth
        Layout.alignment: Qt.AlignVCenter
        textFormat: Text.PlainText
        wrapMode: Text.Wrap
        Component.onCompleted: control.measure(implicitWidth)
    }

    component AddressField: FishUI.TextField {
        Layout.fillWidth: true
        inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase
        onTextEdited: control.revision++
        onAccepted: control.save()
    }

    component FormRow: RowLayout {
        Layout.fillWidth: true
        spacing: Metrics.dialogSpacing
    }

    component FormCard: RoundedItem {
        verticalPadding: Metrics.cardHorizontalPadding
        spacing: Metrics.formRowSpacing
    }

    FormCard {
        FormRow {
            FieldLabel { text: qsTr("Configure IPv4") }

            FishUI.ComboBox {
                id: _ipv4Method
                Layout.fillWidth: true
                textRole: "label"
                valueRole: "value"
                model: control.form.ipv4Methods ?? []
                onActivated: {
                    if (currentValue === "manual")
                        control.fillFromCurrent("ipv4")
                    control.revision++
                }
            }
        }

        FormRow {
            visible: control.ipv4Manual

            FieldLabel { text: qsTr("IP Address") }

            AddressField {
                id: _ipv4Address
                placeholderText: qsTr("Required")
            }
        }

        FormRow {
            visible: control.ipv4Manual

            FieldLabel { text: qsTr("Subnet Mask") }

            AddressField {
                id: _ipv4Netmask
                placeholderText: "255.255.255.0"
            }
        }

        FormRow {
            visible: control.ipv4Manual

            FieldLabel { text: qsTr("Router") }

            AddressField {
                id: _ipv4Gateway
                placeholderText: qsTr("Optional")
            }
        }
    }

    FormCard {
        FormRow {
            FieldLabel { text: qsTr("Configure IPv6") }

            FishUI.ComboBox {
                id: _ipv6Method
                Layout.fillWidth: true
                textRole: "label"
                valueRole: "value"
                model: control.form.ipv6Methods ?? []
                onActivated: {
                    if (currentValue === "manual")
                        control.fillFromCurrent("ipv6")
                    control.revision++
                }
            }
        }

        FormRow {
            visible: control.ipv6Manual

            FieldLabel { text: qsTr("IP Address") }

            AddressField {
                id: _ipv6Address
                placeholderText: qsTr("Required")
            }
        }

        FormRow {
            visible: control.ipv6Manual

            FieldLabel { text: qsTr("Prefix Length") }

            AddressField {
                id: _ipv6Prefix
                placeholderText: "64"
                inputMethodHints: Qt.ImhDigitsOnly
            }
        }

        FormRow {
            visible: control.ipv6Manual

            FieldLabel { text: qsTr("Router") }

            AddressField {
                id: _ipv6Gateway
                placeholderText: qsTr("Optional")
            }
        }
    }

    FormCard {
        FormRow {
            visible: control.automaticFamily

            FishUI.Label {
                Layout.fillWidth: true
                text: qsTr("Automatic DNS")
                textFormat: Text.PlainText
                wrapMode: Text.Wrap
            }

            FishUI.Switch {
                id: _automaticDns
                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                rightPadding: 0
                rightInset: 0
                onToggled: control.revision++
            }
        }

        FormRow {
            FieldLabel { text: qsTr("DNS Servers") }

            AddressField {
                id: _dns
                placeholderText: control.automaticFamily && _automaticDns.checked ? qsTr("Optional") : "1.1.1.1, 8.8.8.8"
            }
        }
    }

    FishUI.Label {
        id: _messageLabel
        Layout.fillWidth: true
        readonly property string problem: control.error.length > 0 ? control.error : (control.result.message ?? "")
        visible: text.length > 0
        text: problem.length > 0 ? problem
              : control.automaticFamily && _automaticDns.checked && _dns.text.trim().length > 0
                ? qsTr("These servers are asked before the network's own.")
              : ""
        textFormat: Text.PlainText
        color: problem.length > 0 ? FishUI.Theme.redColor : FishUI.Theme.disabledTextColor
        wrapMode: Text.Wrap
    }

    RowLayout {
        spacing: Metrics.dialogButtonSpacing
        Layout.alignment: Qt.AlignRight

        FishUI.ConfirmDialogButton {
            text: qsTr("Cancel")
            onClicked: control.finished()
        }

        FishUI.ConfirmDialogButton {
            text: control.saving ? qsTr("Saving…") : qsTr("Save")
            primary: true
            enabled: !control.saving && !!control.result.complete
            onClicked: control.save()
        }
    }
}
