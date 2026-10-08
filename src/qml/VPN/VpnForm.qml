// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import Cutefish.Settings
import FishUI as FishUI
import "../"

// The fields of a plugin VPN, drawn from Handler.vpnForm().fields: the basic ones, then
// the advanced ones by section behind a toggle. values(), storage() and secrets() hand
// back what Handler.saveVpn() takes.
ColumnLayout {
    id: control

    property var fields: []
    // Whether the profile belongs to everyone, which makes a saved password the
    // system's rather than this user's.
    property bool shared: false
    property real labelWidth: 0
    property bool advancedShown: false

    // Edits go to these; bumping revision re-reads what depends on them.
    property var _values: ({})
    property var _storage: ({})
    property var _secrets: ({})
    property int revision: 0

    signal accepted()
    // A label's width, for the owner of the label column.
    signal measured(real width)
    // Fields read their values again.
    signal reloaded()

    readonly property bool hasAdvanced: fields.some(function(field) { return field.advanced })
    readonly property var sections: {
        var list = []
        for (const field of fields) {
            if (!field.advanced)
                continue
            if (list.length === 0 || list[list.length - 1].title !== field.section)
                list.push({ title: field.section, fields: [] })
            list[list.length - 1].fields.push(field)
        }
        return list
    }
    readonly property bool complete: {
        revision
        for (const field of fields) {
            if (field.required && shown(field) && String(_values[field.key] ?? "").trim().length === 0)
                return false
        }
        return true
    }

    spacing: Metrics.formRowSpacing

    // Type values of VpnSchema::Type.
    readonly property int textType: 0
    readonly property int passwordType: 1
    readonly property int fileType: 2
    readonly property int choiceType: 3
    readonly property int boolType: 4
    readonly property int numberType: 5

    function load(fields) {
        var values = {}
        var storage = {}
        for (const field of fields) {
            values[field.key] = field.value
            if (field.type === passwordType)
                storage[field.key] = field.storage
        }
        control.fields = fields
        _values = values
        _storage = storage
        _secrets = {}
        advancedShown = false
        revision++
        reloaded()
    }

    function setSecrets(secrets) {
        var copy = {}
        for (const field of fields) {
            if (field.type === passwordType && secrets[field.key] !== undefined)
                copy[field.key] = secrets[field.key]
        }
        _secrets = copy
        revision++
        reloaded()
    }

    function values() { return _values }
    function storage() { return _storage }
    function secrets() { return _secrets }

    function setValue(key, value) {
        _values[key] = value
        revision++
    }

    function shown(field) {
        if (!field.showWhen)
            return true
        const equals = field.showWhen.indexOf("=")
        const key = field.showWhen.substring(0, equals)
        const accepted = field.showWhen.substring(equals + 1).split("|")
        var value = _values[key]
        for (const other of fields) {
            if (other.key === key && other.type === boolType)
                value = value ? other.trueValue : ""
        }
        return accepted.indexOf(String(value ?? "")) >= 0
    }

    function measure(width) {
        measured(width)
    }

    FileDialog {
        id: _fileDialog
        property var field

        onAccepted: {
            control.setValue(field.key, decodeURIComponent(selectedFile.toString().replace("file://", "")))
            control.reloaded()
        }
    }

    component FieldLabel: FishUI.Label {
        required property var field
        Layout.preferredWidth: control.labelWidth
        Layout.alignment: Qt.AlignVCenter
        text: field.label
        textFormat: Text.PlainText
        wrapMode: Text.Wrap
        Component.onCompleted: control.measure(implicitWidth)
    }

    component TextRow: RowLayout {
        id: _textRow
        required property var field
        readonly property bool number: field.type === control.numberType
        Layout.fillWidth: true
        spacing: Metrics.dialogSpacing

        FieldLabel { field: _textRow.field }

        FishUI.TextField {
            id: _textField
            Layout.fillWidth: true
            placeholderText: _textRow.field.placeholder || (_textRow.field.required ? qsTr("Required") : qsTr("Optional"))
            inputMethodHints: _textRow.number ? Qt.ImhDigitsOnly
                                              : Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase
            validator: _textRow.number ? _intValidator : null
            onTextEdited: control.setValue(_textRow.field.key, text)
            onAccepted: control.accepted()

            IntValidator {
                id: _intValidator
                bottom: 0
                top: _textRow.field.maximum > 0 ? _textRow.field.maximum : 2147483647
            }

            function load() {
                const value = control._values[_textRow.field.key]
                text = value === undefined || value === null ? "" : String(value)
            }
            Component.onCompleted: load()
            Connections {
                target: control
                function onReloaded() { _textField.load() }
            }
        }
    }

    component PasswordRow: RowLayout {
        id: _passwordRow
        required property var field
        readonly property int saveFlags: control.shared ? 0 : 1
        readonly property int storage: control.revision, (control._storage[field.key] ?? 1)
        readonly property bool saving: storage !== 2 && storage !== 4
        Layout.fillWidth: true
        spacing: Metrics.dialogSpacing

        FieldLabel { field: _passwordRow.field }

        FishUI.TextField {
            id: _passwordField
            Layout.fillWidth: true
            visible: _passwordRow.saving
            placeholderText: qsTr("Asked on connect")
            echoMode: TextInput.Password
            inputMethodHints: Qt.ImhSensitiveData | Qt.ImhNoPredictiveText
            onTextEdited: {
                control._secrets[_passwordRow.field.key] = text
                control.revision++
            }
            onAccepted: control.accepted()

            function load() { text = control._secrets[_passwordRow.field.key] ?? "" }
            Component.onCompleted: load()
            Connections {
                target: control
                function onReloaded() { _passwordField.load() }
            }
        }

        FishUI.ComboBox {
            id: _storageBox
            Layout.fillWidth: !_passwordRow.saving
            Layout.preferredWidth: 120
            textRole: "text"
            valueRole: "value"
            model: {
                var choices = [{ text: control.shared ? qsTr("Saved for all") : qsTr("Saved"), value: _passwordRow.saveFlags },
                               { text: qsTr("Ask each time"), value: 2 }]
                if (_passwordRow.field.allowNotRequired)
                    choices.push({ text: qsTr("Not needed"), value: 4 })
                return choices
            }
            onActivated: {
                control._storage[_passwordRow.field.key] = currentValue
                control.revision++
            }

            function load() {
                const stored = control._storage[_passwordRow.field.key] ?? 1
                currentIndex = Math.max(0, indexOfValue(stored === 0 || stored === 1 ? _passwordRow.saveFlags : stored))
            }
            Component.onCompleted: load()
            onModelChanged: load()
            Connections {
                target: control
                function onReloaded() { _storageBox.load() }
            }
        }
    }

    component FileRow: RowLayout {
        id: _fileRow
        required property var field
        Layout.fillWidth: true
        spacing: Metrics.dialogSpacing

        FieldLabel { field: _fileRow.field }

        FishUI.TextField {
            id: _fileField
            Layout.fillWidth: true
            placeholderText: _fileRow.field.required ? qsTr("Required") : qsTr("Optional")
            inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase
            onTextEdited: control.setValue(_fileRow.field.key, text)
            onAccepted: control.accepted()

            function load() { text = control._values[_fileRow.field.key] ?? "" }
            Component.onCompleted: load()
            Connections {
                target: control
                function onReloaded() { _fileField.load() }
            }
        }

        FishUI.Button {
            Layout.fillHeight: true
            text: qsTr("Choose…")
            onClicked: {
                _fileDialog.field = _fileRow.field
                _fileDialog.title = _fileRow.field.label
                _fileDialog.open()
            }
        }
    }

    component ChoiceRow: RowLayout {
        id: _choiceRow
        required property var field
        Layout.fillWidth: true
        spacing: Metrics.dialogSpacing

        FieldLabel { field: _choiceRow.field }

        FishUI.ComboBox {
            id: _choiceBox
            Layout.fillWidth: true
            textRole: "label"
            valueRole: "value"
            model: _choiceRow.field.choices
            onActivated: control.setValue(_choiceRow.field.key, currentValue)

            function load() {
                currentIndex = Math.max(0, indexOfValue(String(control._values[_choiceRow.field.key] ?? "")))
            }
            Component.onCompleted: load()
            Connections {
                target: control
                function onReloaded() { _choiceBox.load() }
            }
        }
    }

    component BoolRow: RowLayout {
        id: _boolRow
        required property var field
        Layout.fillWidth: true
        spacing: Metrics.dialogSpacing

        FieldLabel {
            field: _boolRow.field
            Layout.fillWidth: true
            Layout.preferredWidth: -1
        }

        FishUI.Switch {
            id: _boolSwitch
            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
            rightPadding: 0
            rightInset: 0
            onToggled: control.setValue(_boolRow.field.key, checked)

            function load() { checked = !!control._values[_boolRow.field.key] }
            Component.onCompleted: load()
            Connections {
                target: control
                function onReloaded() { _boolSwitch.load() }
            }
        }
    }

    component FieldRow: Item {
        id: _fieldRow
        required property var modelData
        readonly property var field: modelData
        readonly property bool shown: control.revision, control.shown(field)
        Layout.fillWidth: true
        visible: shown
        implicitHeight: _loader.implicitHeight

        Loader {
            id: _loader
            anchors.left: parent.left
            anchors.right: parent.right
            property var field: _fieldRow.field
            sourceComponent: field.type === control.passwordType ? _passwordComponent
                           : field.type === control.fileType ? _fileComponent
                           : field.type === control.choiceType ? _choiceComponent
                           : field.type === control.boolType ? _boolComponent
                           : _textComponent
        }
    }

    Component { id: _textComponent; TextRow { field: parent.field } }
    Component { id: _passwordComponent; PasswordRow { field: parent.field } }
    Component { id: _fileComponent; FileRow { field: parent.field } }
    Component { id: _choiceComponent; ChoiceRow { field: parent.field } }
    Component { id: _boolComponent; BoolRow { field: parent.field } }

    Repeater {
        model: control.fields.filter(function(field) { return !field.advanced })
        delegate: FieldRow {}
    }

    FishUI.Label {
        visible: control.hasAdvanced
        text: control.advancedShown ? qsTr("Hide advanced options") : qsTr("Show advanced options")
        textFormat: Text.PlainText
        color: FishUI.Theme.highlightColor

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: control.advancedShown = !control.advancedShown
        }
    }

    Repeater {
        model: control.advancedShown ? control.sections : []
        delegate: ColumnLayout {
            id: _section
            required property var modelData
            Layout.fillWidth: true
            spacing: Metrics.formRowSpacing

            FishUI.Label {
                Layout.fillWidth: true
                Layout.topMargin: Metrics.s
                text: _section.modelData.title
                textFormat: Text.PlainText
                font.weight: Font.DemiBold
                color: FishUI.Theme.textColor
            }

            Repeater {
                model: _section.modelData.fields
                delegate: FieldRow {}
            }
        }
    }
}
