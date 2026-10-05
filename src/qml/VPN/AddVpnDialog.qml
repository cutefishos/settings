import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import Cutefish.Settings
import FishUI as FishUI
import Cutefish.Network as NM

// Sets up a VPN from what the provider or the network's administrator hands out: the
// server and how to sign in. A password left empty is asked for when connecting.
FishUI.Window {
    id: control

    required property NM.Handler handler

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

    property int contentWidth: 440
    property int contentHeight: _mainLayout.implicitHeight + 32

    title: qsTr("Add VPN")
    visible: false
    minimizeButtonVisible: false

    background.color: FishUI.Theme.secondBackgroundColor
    // The background item reaches a grown window a frame or two after its buffer does.
    color: background.color
    flags: Qt.Dialog | Qt.FramelessWindowHint

    // Read again each time the dialog opens, so a plugin installed meanwhile shows up.
    property var kinds: []
    readonly property var kindInfo: kinds[_kindBox.currentIndex] ?? ({})
    readonly property string kind: kindInfo.kind ?? ""
    readonly property bool installed: kindInfo.installed ?? false
    readonly property bool wireGuard: kind === "wireguard"
    readonly property bool certificates: kind === "openvpn" && _authenticationBox.currentValue === "certificates"
    readonly property bool usesUserName: installed && !wireGuard && !certificates

    property bool adding: false
    property string error

    readonly property bool canAdd: installed && !adding && filled(_serverTextField)
                                   && (wireGuard ? filled(_privateKeyTextField) && filled(_addressTextField)
                                                   && filled(_publicKeyTextField)
                                      : certificates ? filled(_certificateField.field) && filled(_keyField.field)
                                      : filled(_userNameTextField))

    function filled(field) {
        return field.text.trim().length > 0
    }

    function add() {
        if (!canAdd)
            return

        adding = true
        error = ""
        handler.addVpn({
            kind: kind,
            name: _nameTextField.text,
            server: _serverTextField.text,
            userName: _userNameTextField.text,
            password: _passwordTextField.text,
            authentication: _authenticationBox.currentValue,
            caFile: _caField.field.text,
            certFile: _certificateField.field.text,
            keyFile: _keyField.field.text,
            keyPassword: _keyPasswordTextField.text,
            presharedKey: _presharedKeyTextField.text,
            privateKey: _privateKeyTextField.text,
            publicKey: _publicKeyTextField.text,
            address: _addressTextField.text,
            allowedIps: _allowedIpsTextField.text,
            dns: _dnsTextField.text
        })
    }

    function chooseFile(field, title) {
        _fileDialog.field = field
        _fileDialog.title = title
        _fileDialog.open()
    }

    onVisibleChanged: {
        if (visible) {
            kinds = handler.vpnKinds().map(function(item) {
                item.text = item.installed ? item.label : qsTr("%1 (Not Installed)").arg(item.label)
                return item
            })
            _kindBox.currentIndex = 0
            _serverTextField.forceActiveFocus()
            return
        }

        for (const field of [_nameTextField, _serverTextField, _userNameTextField, _passwordTextField,
                             _presharedKeyTextField, _caField.field, _certificateField.field, _keyField.field,
                             _keyPasswordTextField, _privateKeyTextField, _publicKeyTextField, _addressTextField,
                             _allowedIpsTextField, _dnsTextField])
            field.clear()
        _kindBox.currentIndex = 0
        _authenticationBox.currentIndex = 0
        adding = false
        error = ""
    }

    Connections {
        target: control.handler

        function onVpnAdded() {
            if (!control.adding)
                return
            control.adding = false
            control.close()
        }

        function onVpnAddFailed(message) {
            if (!control.adding)
                return
            control.adding = false
            control.error = message
        }
    }

    Shortcut {
        sequence: "Escape"
        onActivated: control.close()
    }

    FileDialog {
        id: _fileDialog

        property Item field

        onAccepted: field.text = decodeURIComponent(selectedFile.toString().replace("file://", ""))
    }

    component FileField: RowLayout {
        property alias field: _fileTextField
        property alias placeholderText: _fileTextField.placeholderText

        signal choose()
        signal accepted()

        Layout.fillWidth: true
        spacing: Metrics.controlSpacing

        FishUI.TextField {
            id: _fileTextField
            Layout.fillWidth: true
            onAccepted: parent.accepted()
        }

        FishUI.Button {
            Layout.fillHeight: true
            text: qsTr("Choose…")
            onClicked: parent.choose()
        }
    }

    ColumnLayout {
        id: _mainLayout
        // Not filled to the window: the window height follows this layout's implicit
        // height a polish later, and a filled layout would be spread over the old height.
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
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
            id: _form
            Layout.fillWidth: true
            columns: 2
            columnSpacing: Metrics.dialogSpacing
            rowSpacing: Metrics.formRowSpacing

            // Hidden rows leave the grid, so the fields would move as they appear.
            readonly property real labelWidth: Math.max(_kindLabel.implicitWidth,
                                                        _nameLabel.implicitWidth,
                                                        _serverLabel.implicitWidth,
                                                        _authenticationLabel.implicitWidth,
                                                        _userNameLabel.implicitWidth,
                                                        _passwordLabel.implicitWidth,
                                                        _presharedKeyLabel.implicitWidth,
                                                        _certificateLabel.implicitWidth,
                                                        _keyLabel.implicitWidth,
                                                        _keyPasswordLabel.implicitWidth,
                                                        _caLabel.implicitWidth,
                                                        _publicKeyLabel.implicitWidth,
                                                        _privateKeyLabel.implicitWidth,
                                                        _addressLabel.implicitWidth,
                                                        _allowedIpsLabel.implicitWidth,
                                                        _dnsLabel.implicitWidth)

            FishUI.Label {
                id: _kindLabel
                Layout.preferredWidth: _form.labelWidth
                text: qsTr("Type")
            }

            FishUI.ComboBox {
                id: _kindBox
                Layout.fillWidth: true
                textRole: "text"
                valueRole: "kind"
                model: control.kinds
            }

            FishUI.Label {
                Layout.columnSpan: 2
                Layout.fillWidth: true
                visible: !control.installed && control.kind.length > 0
                text: qsTr("Install %1 to set up this kind of VPN.").arg(control.kindInfo.package ?? "")
                textFormat: Text.PlainText
                color: FishUI.Theme.disabledTextColor
                wrapMode: Text.Wrap
            }

            FishUI.Label {
                id: _nameLabel
                Layout.preferredWidth: _form.labelWidth
                visible: control.installed
                text: qsTr("Name")
            }

            FishUI.TextField {
                id: _nameTextField
                Layout.fillWidth: true
                visible: control.installed
                placeholderText: qsTr("Optional")
                onAccepted: control.add()
            }

            FishUI.Label {
                id: _serverLabel
                Layout.preferredWidth: _form.labelWidth
                visible: control.installed
                text: qsTr("Server")
            }

            FishUI.TextField {
                id: _serverTextField
                Layout.fillWidth: true
                visible: control.installed
                placeholderText: qsTr("Required")
                inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase | Qt.ImhUrlCharactersOnly
                onAccepted: control.add()
            }

            FishUI.Label {
                id: _authenticationLabel
                Layout.preferredWidth: _form.labelWidth
                visible: control.installed && control.kind === "openvpn"
                text: qsTr("Sign in with")
            }

            FishUI.ComboBox {
                id: _authenticationBox
                Layout.fillWidth: true
                visible: control.installed && control.kind === "openvpn"
                textRole: "text"
                valueRole: "value"
                model: [
                    { text: qsTr("User name and password"), value: "password" },
                    { text: qsTr("Certificate"), value: "certificates" }
                ]
            }

            FishUI.Label {
                id: _userNameLabel
                Layout.preferredWidth: _form.labelWidth
                visible: control.usesUserName
                text: qsTr("User name")
            }

            FishUI.TextField {
                id: _userNameTextField
                Layout.fillWidth: true
                visible: control.usesUserName
                placeholderText: qsTr("Required")
                inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase
                onAccepted: control.add()
            }

            FishUI.Label {
                id: _passwordLabel
                Layout.preferredWidth: _form.labelWidth
                visible: control.usesUserName
                text: qsTr("Password")
            }

            FishUI.TextField {
                id: _passwordTextField
                Layout.fillWidth: true
                visible: control.usesUserName
                placeholderText: qsTr("Ask when connecting")
                echoMode: TextInput.Password
                inputMethodHints: Qt.ImhSensitiveData | Qt.ImhNoPredictiveText
                onAccepted: control.add()
            }

            FishUI.Label {
                id: _presharedKeyLabel
                Layout.preferredWidth: _form.labelWidth
                visible: control.installed && control.kind === "l2tp"
                text: qsTr("Pre-shared key")
            }

            FishUI.TextField {
                id: _presharedKeyTextField
                Layout.fillWidth: true
                visible: control.installed && control.kind === "l2tp"
                placeholderText: qsTr("Optional")
                echoMode: TextInput.Password
                inputMethodHints: Qt.ImhSensitiveData | Qt.ImhNoPredictiveText
                onAccepted: control.add()
            }

            FishUI.Label {
                id: _certificateLabel
                Layout.preferredWidth: _form.labelWidth
                visible: control.certificates
                text: qsTr("Certificate")
            }

            FileField {
                id: _certificateField
                visible: control.certificates
                placeholderText: qsTr("Required")
                onChoose: control.chooseFile(field, qsTr("Choose Your Certificate"))
                onAccepted: control.add()
            }

            FishUI.Label {
                id: _keyLabel
                Layout.preferredWidth: _form.labelWidth
                visible: control.certificates
                text: qsTr("Private key")
            }

            FileField {
                id: _keyField
                visible: control.certificates
                placeholderText: qsTr("Required")
                onChoose: control.chooseFile(field, qsTr("Choose Your Private Key"))
                onAccepted: control.add()
            }

            FishUI.Label {
                id: _keyPasswordLabel
                Layout.preferredWidth: _form.labelWidth
                visible: control.certificates
                text: qsTr("Key password")
            }

            FishUI.TextField {
                id: _keyPasswordTextField
                Layout.fillWidth: true
                visible: control.certificates
                placeholderText: qsTr("Optional")
                echoMode: TextInput.Password
                inputMethodHints: Qt.ImhSensitiveData | Qt.ImhNoPredictiveText
                onAccepted: control.add()
            }

            FishUI.Label {
                id: _caLabel
                Layout.preferredWidth: _form.labelWidth
                visible: control.installed && control.kind === "openvpn"
                text: qsTr("CA certificate")
            }

            FileField {
                id: _caField
                visible: control.installed && control.kind === "openvpn"
                placeholderText: qsTr("Optional")
                onChoose: control.chooseFile(field, qsTr("Choose the CA Certificate"))
                onAccepted: control.add()
            }

            FishUI.Label {
                id: _publicKeyLabel
                Layout.preferredWidth: _form.labelWidth
                visible: control.wireGuard
                text: qsTr("Server public key")
            }

            FishUI.TextField {
                id: _publicKeyTextField
                Layout.fillWidth: true
                visible: control.wireGuard
                placeholderText: qsTr("Required")
                inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase
                onAccepted: control.add()
            }

            FishUI.Label {
                id: _privateKeyLabel
                Layout.preferredWidth: _form.labelWidth
                visible: control.wireGuard
                text: qsTr("Private key")
            }

            FishUI.TextField {
                id: _privateKeyTextField
                Layout.fillWidth: true
                visible: control.wireGuard
                placeholderText: qsTr("Required")
                echoMode: TextInput.Password
                inputMethodHints: Qt.ImhSensitiveData | Qt.ImhNoPredictiveText
                onAccepted: control.add()
            }

            FishUI.Label {
                id: _addressLabel
                Layout.preferredWidth: _form.labelWidth
                visible: control.wireGuard
                text: qsTr("Address")
            }

            FishUI.TextField {
                id: _addressTextField
                Layout.fillWidth: true
                visible: control.wireGuard
                placeholderText: "10.0.0.2/24"
                inputMethodHints: Qt.ImhNoPredictiveText
                onAccepted: control.add()
            }

            FishUI.Label {
                id: _allowedIpsLabel
                Layout.preferredWidth: _form.labelWidth
                visible: control.wireGuard
                text: qsTr("Allowed IPs")
            }

            FishUI.TextField {
                id: _allowedIpsTextField
                Layout.fillWidth: true
                visible: control.wireGuard
                placeholderText: qsTr("All traffic")
                inputMethodHints: Qt.ImhNoPredictiveText
                onAccepted: control.add()
            }

            FishUI.Label {
                id: _dnsLabel
                Layout.preferredWidth: _form.labelWidth
                visible: control.wireGuard
                text: qsTr("DNS")
            }

            FishUI.TextField {
                id: _dnsTextField
                Layout.fillWidth: true
                visible: control.wireGuard
                placeholderText: qsTr("Optional")
                inputMethodHints: Qt.ImhNoPredictiveText
                onAccepted: control.add()
            }
        }

        FishUI.Label {
            Layout.fillWidth: true
            visible: control.error.length > 0
            text: control.error
            textFormat: Text.PlainText
            color: FishUI.Theme.redColor
            wrapMode: Text.Wrap
        }

        RowLayout {
            spacing: Metrics.dialogButtonSpacing
            Layout.alignment: Qt.AlignRight

            FishUI.ConfirmDialogButton {
                text: qsTr("Cancel")
                onClicked: control.close()
            }

            FishUI.ConfirmDialogButton {
                enabled: control.canAdd
                primary: true
                text: qsTr("Add")
                onClicked: control.add()
            }
        }
    }
}
