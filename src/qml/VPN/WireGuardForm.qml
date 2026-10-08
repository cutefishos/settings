// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cutefish.Settings
import FishUI as FishUI
import "../"

// A WireGuard tunnel as wg-quick lays it out: the interface, then a card per peer.
// collect() hands back what Handler.saveVpn() takes for the "wireguard" kind.
ColumnLayout {
    id: control

    property real labelWidth: 0
    property bool advancedShown: false
    // Whether a stored private key exists that an empty field keeps.
    property bool hasPrivateKey: false

    signal accepted()
    // A label's width, for the owner of the label column.
    signal measured(real width)

    readonly property bool complete: {
        _peers.revision
        if (_addressField.text.trim().length === 0)
            return false
        if (!hasPrivateKey && _privateKeyField.text.trim().length === 0)
            return false
        if (_peers.count === 0)
            return false
        for (var i = 0; i < _peers.count; ++i) {
            if (_peers.get(i).publicKey.trim().length === 0)
                return false
        }
        return true
    }

    spacing: Metrics.formRowSpacing

    function load(form) {
        _interfaceField.text = form.interfaceName ?? ""
        _privateKeyField.text = ""
        hasPrivateKey = false
        _addressField.text = form.addresses ?? ""
        _dnsField.text = form.dns ?? ""
        _listenPortField.text = form.listenPort > 0 ? String(form.listenPort) : ""
        _fwmarkField.text = form.fwmark > 0 ? String(form.fwmark) : ""
        _mtuField.text = form.mtu > 0 ? String(form.mtu) : ""
        _peerRoutesSwitch.checked = form.peerRoutes ?? true
        advancedShown = false
        _peers.clear()
        for (const peer of form.peers ?? [])
            _peers.append(peerRow(peer))
        if (_peers.count === 0)
            _peers.append(peerRow({}))
        _peers.revision++
    }

    function setSecrets(secrets) {
        if (secrets.privateKey) {
            _privateKeyField.text = secrets.privateKey
            hasPrivateKey = true
        }
        const presharedKeys = secrets.presharedKeys ?? {}
        for (var i = 0; i < _peers.count; ++i) {
            const key = presharedKeys[_peers.get(i).publicKey]
            if (key !== undefined)
                _peers.setProperty(i, "presharedKey", key)
        }
        _peers.revision++
    }

    function peerRow(peer) {
        return {
            publicKey: peer.publicKey ?? "",
            presharedKey: peer.presharedKey ?? "",
            hasPresharedKey: peer.hasPresharedKey ?? false,
            endpoint: peer.endpoint ?? "",
            allowedIps: peer.allowedIps ?? "",
            keepalive: peer.keepalive > 0 ? String(peer.keepalive) : ""
        }
    }

    function collect() {
        var peers = []
        for (var i = 0; i < _peers.count; ++i) {
            const peer = _peers.get(i)
            peers.push({
                publicKey: peer.publicKey,
                presharedKey: peer.presharedKey,
                hasPresharedKey: peer.hasPresharedKey,
                endpoint: peer.endpoint,
                allowedIps: peer.allowedIps,
                keepalive: peer.keepalive
            })
        }
        return {
            interfaceName: _interfaceField.text,
            privateKey: _privateKeyField.text,
            addresses: _addressField.text,
            dns: _dnsField.text,
            listenPort: _listenPortField.text,
            fwmark: _fwmarkField.text,
            mtu: _mtuField.text,
            peerRoutes: _peerRoutesSwitch.checked,
            peers: peers
        }
    }

    function measure(width) {
        measured(width)
    }

    ListModel {
        id: _peers
        // Bumped on edits, so `complete` reads the rows again.
        property int revision: 0
    }

    component FieldLabel: FishUI.Label {
        Layout.preferredWidth: control.labelWidth
        Layout.alignment: Qt.AlignVCenter
        textFormat: Text.PlainText
        wrapMode: Text.Wrap
        Component.onCompleted: control.measure(implicitWidth)
    }

    component Field: FishUI.TextField {
        Layout.fillWidth: true
        inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase
        onAccepted: control.accepted()
    }

    component NumberField: Field {
        property int maximum: 2147483647
        inputMethodHints: Qt.ImhDigitsOnly
        placeholderText: qsTr("Optional")
        validator: IntValidator { bottom: 0; top: maximum }
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: Metrics.dialogSpacing

        FieldLabel { text: qsTr("Private key") }

        Field {
            id: _privateKeyField
            placeholderText: qsTr("Required")
            echoMode: TextInput.Password
            inputMethodHints: Qt.ImhSensitiveData | Qt.ImhNoPredictiveText
        }
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: Metrics.dialogSpacing

        FieldLabel { text: qsTr("Address") }

        Field {
            id: _addressField
            placeholderText: "10.0.0.2/24"
            inputMethodHints: Qt.ImhNoPredictiveText
        }
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: Metrics.dialogSpacing

        FieldLabel { text: qsTr("DNS") }

        Field {
            id: _dnsField
            placeholderText: qsTr("Optional")
            inputMethodHints: Qt.ImhNoPredictiveText
        }
    }

    FishUI.Label {
        text: control.advancedShown ? qsTr("Hide advanced options") : qsTr("Show advanced options")
        textFormat: Text.PlainText
        color: FishUI.Theme.highlightColor

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: control.advancedShown = !control.advancedShown
        }
    }

    ColumnLayout {
        Layout.fillWidth: true
        visible: control.advancedShown
        spacing: Metrics.formRowSpacing

        RowLayout {
            Layout.fillWidth: true
            spacing: Metrics.dialogSpacing

            FieldLabel { text: qsTr("Interface name") }

            Field {
                id: _interfaceField
                placeholderText: qsTr("From the name")
                maximumLength: 15
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: Metrics.dialogSpacing

            FieldLabel { text: qsTr("Listen port") }

            NumberField {
                id: _listenPortField
                maximum: 65535
                placeholderText: qsTr("Automatic")
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: Metrics.dialogSpacing

            FieldLabel { text: qsTr("Firewall mark") }

            Field {
                id: _fwmarkField
                placeholderText: qsTr("Optional")
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: Metrics.dialogSpacing

            FieldLabel { text: qsTr("MTU") }

            NumberField {
                id: _mtuField
                placeholderText: qsTr("Automatic")
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: Metrics.dialogSpacing

            FishUI.Label {
                Layout.fillWidth: true
                text: qsTr("Add routes for the peers' allowed IPs")
                textFormat: Text.PlainText
                wrapMode: Text.Wrap
            }

            FishUI.Switch {
                id: _peerRoutesSwitch
                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                rightPadding: 0
                rightInset: 0
                checked: true
            }
        }
    }

    Repeater {
        model: _peers

        delegate: RoundedItem {
            id: _peerCard
            required property int index
            required property var model

            Layout.topMargin: Metrics.s
            verticalPadding: Metrics.cardHorizontalPadding

            ColumnLayout {
                Layout.fillWidth: true
                spacing: Metrics.formRowSpacing

                RowLayout {
                    Layout.fillWidth: true
                    spacing: Metrics.dialogSpacing

                    FishUI.Label {
                        Layout.fillWidth: true
                        text: _peers.count > 1 ? qsTr("Peer %1").arg(_peerCard.index + 1) : qsTr("Peer")
                        textFormat: Text.PlainText
                        font.weight: Font.DemiBold
                        color: FishUI.Theme.textColor
                    }

                    FishUI.Button {
                        visible: _peers.count > 1
                        text: qsTr("Remove")
                        onClicked: {
                            _peers.remove(_peerCard.index)
                            _peers.revision++
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: Metrics.dialogSpacing

                    FieldLabel { text: qsTr("Public key") }

                    Field {
                        text: _peerCard.model.publicKey
                        placeholderText: qsTr("Required")
                        onTextEdited: {
                            _peerCard.model.publicKey = text
                            _peers.revision++
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: Metrics.dialogSpacing

                    FieldLabel { text: qsTr("Preshared key") }

                    Field {
                        text: _peerCard.model.presharedKey
                        placeholderText: _peerCard.model.hasPresharedKey ? qsTr("Kept") : qsTr("Optional")
                        echoMode: TextInput.Password
                        inputMethodHints: Qt.ImhSensitiveData | Qt.ImhNoPredictiveText
                        onTextEdited: {
                            _peerCard.model.presharedKey = text
                            // Clearing the field drops the stored key.
                            _peerCard.model.hasPresharedKey = text.length > 0
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: Metrics.dialogSpacing

                    FieldLabel { text: qsTr("Endpoint") }

                    Field {
                        text: _peerCard.model.endpoint
                        placeholderText: qsTr("vpn.example.com:51820")
                        inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase | Qt.ImhUrlCharactersOnly
                        onTextEdited: _peerCard.model.endpoint = text
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: Metrics.dialogSpacing

                    FieldLabel { text: qsTr("Allowed IPs") }

                    Field {
                        text: _peerCard.model.allowedIps
                        placeholderText: qsTr("All traffic")
                        inputMethodHints: Qt.ImhNoPredictiveText
                        onTextEdited: _peerCard.model.allowedIps = text
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: Metrics.dialogSpacing

                    FieldLabel { text: qsTr("Keepalive") }

                    NumberField {
                        text: _peerCard.model.keepalive
                        maximum: 65535
                        placeholderText: qsTr("Off")
                        onTextEdited: _peerCard.model.keepalive = text
                    }
                }
            }
        }
    }

    FishUI.Label {
        text: qsTr("Add peer")
        textFormat: Text.PlainText
        color: FishUI.Theme.highlightColor

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                _peers.append(control.peerRow({}))
                _peers.revision++
            }
        }
    }
}
