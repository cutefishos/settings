// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cutefish.Settings
import FishUI as FishUI
import "../"

// What a VPN profile is made of, for the Add VPN and details dialogs: its name, the
// fields of its kind, and who may use it. load() takes Handler.vpnForm(), collect()
// gives Handler.saveVpn() its form.
ColumnLayout {
    id: control

    property var form: ({})
    readonly property string kind: form.kind ?? ""
    readonly property bool installed: form.installed ?? false
    readonly property bool wireGuard: kind === "wireguard"
    readonly property bool complete: installed && (_formLoader.item ? _formLoader.item.complete : false)
    property alias name: _nameField.text
    property alias shared: _sharedSwitch.checked
    property alias ownNetworkOnly: _ownNetworkSwitch.checked
    // One label column for the name and the form's rows, measured as they appear; a
    // longer label wraps rather than squeezing the fields.
    property real labelWidth: 0
    readonly property real maxLabelWidth: 140

    signal accepted()
    spacing: Metrics.formRowSpacing

    function measure(width) {
        labelWidth = Math.max(labelWidth, Math.min(width, maxLabelWidth))
    }

    function load(form) {
        control.form = form
        labelWidth = 0
        measure(_nameLabel.implicitWidth)
        _nameField.text = form.name ?? ""
        _sharedSwitch.checked = form.shared ?? false
        _ownNetworkSwitch.checked = form.ownNetworkOnly ?? false
        _formLoader.sourceComponent = undefined
        if (!form.installed)
            return
        _formLoader.sourceComponent = form.kind === "wireguard" ? _wireGuardComponent : _fieldsComponent
        _formLoader.item.load(form.kind === "wireguard" ? form.wireguard : form.fields)
    }

    function setSecrets(secrets) {
        if (_formLoader.item)
            _formLoader.item.setSecrets(secrets)
    }

    function collect() {
        var result = control.wireGuard ? _formLoader.item.collect()
                   : { values: _formLoader.item.values(), storage: _formLoader.item.storage(), secrets: _formLoader.item.secrets() }
        result.kind = kind
        result.name = _nameField.text
        result.shared = _sharedSwitch.checked
        result.ownNetworkOnly = _ownNetworkSwitch.checked
        return result
    }

    function focusFirst() {
        _nameField.forceActiveFocus()
    }

    FishUI.Label {
        Layout.fillWidth: true
        visible: !control.installed && control.kind.length > 0
        text: qsTr("Install %1 to set up this kind of VPN.").arg(control.form.package ?? "")
        textFormat: Text.PlainText
        color: FishUI.Theme.disabledTextColor
        wrapMode: Text.Wrap
    }

    RowLayout {
        Layout.fillWidth: true
        visible: control.installed
        spacing: Metrics.dialogSpacing

        FishUI.Label {
            id: _nameLabel
            Layout.preferredWidth: control.labelWidth
            Layout.alignment: Qt.AlignVCenter
            text: qsTr("Name")
            textFormat: Text.PlainText
            Component.onCompleted: control.measure(implicitWidth)
        }

        FishUI.TextField {
            id: _nameField
            Layout.fillWidth: true
            placeholderText: qsTr("Optional")
            onAccepted: control.accepted()
        }
    }

    Loader {
        id: _formLoader
        Layout.fillWidth: true
        visible: control.installed
        onLoaded: item.accepted.connect(control.accepted)
    }

    Component {
        id: _fieldsComponent
        VpnForm {
            shared: control.shared
            labelWidth: control.labelWidth
            onMeasured: (width) => control.measure(width)
        }
    }

    Component {
        id: _wireGuardComponent
        WireGuardForm {
            labelWidth: control.labelWidth
            onMeasured: (width) => control.measure(width)
        }
    }

    ColumnLayout {
        Layout.fillWidth: true
        Layout.topMargin: Metrics.s
        visible: control.installed
        spacing: Metrics.formRowSpacing

        RowLayout {
            Layout.fillWidth: true
            spacing: Metrics.dialogSpacing

            FishUI.Label {
                Layout.fillWidth: true
                text: qsTr("Available to all users")
                textFormat: Text.PlainText
                wrapMode: Text.Wrap
            }

            FishUI.Switch {
                id: _sharedSwitch
                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                rightPadding: 0
                rightInset: 0
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: Metrics.dialogSpacing

            FishUI.Label {
                Layout.fillWidth: true
                text: qsTr("Use only for resources on its network")
                textFormat: Text.PlainText
                wrapMode: Text.Wrap
            }

            FishUI.Switch {
                id: _ownNetworkSwitch
                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                rightPadding: 0
                rightInset: 0
            }
        }
    }
}
