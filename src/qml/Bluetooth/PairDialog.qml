import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cutefish.Bluetooth 1.0 as Bluetooth

Dialog {
    id: dialog
    anchors.centerIn: parent
    width: Math.min(parent.width - 32, 420)
    modal: true
    closePolicy: Popup.NoAutoClose
    title: qsTr("Bluetooth pairing")

    readonly property string kind: Bluetooth.Manager.pairingKind
    readonly property bool inputRequired: kind === "RequestPinCode" || kind === "RequestPasskey"
    readonly property bool displayOnly: kind === "DisplayPinCode" || kind === "DisplayPasskey"

    Connections {
        target: Bluetooth.Manager
        function onPairingChanged() {
            if (Bluetooth.Manager.pairingKind.length) {
                if (!dialog.opened) {
                    codeInput.clear()
                    dialog.open()
                    if (dialog.inputRequired)
                        codeInput.forceActiveFocus()
                }
            } else {
                dialog.close()
            }
        }
    }

    contentItem: ColumnLayout {
        spacing: 16
        Label {
            Layout.fillWidth: true
            text: Bluetooth.Manager.pairingDevice
            textFormat: Text.PlainText
            wrapMode: Text.Wrap
        }
        Label {
            Layout.fillWidth: true
            text: dialog.kind === "RequestConfirmation" ? qsTr("Does this code match the code on your device?")
                  : dialog.kind === "RequestPinCode" ? qsTr("Enter the PIN for your device.")
                  : dialog.kind === "RequestPasskey" ? qsTr("Enter the passkey shown on your device.")
                  : dialog.displayOnly ? qsTr("Enter this code on your device, then press Enter.")
                  : dialog.kind === "AuthorizeService" ? qsTr("Allow this device to use the following Bluetooth service?")
                  : qsTr("Allow this device to pair?")
            wrapMode: Text.Wrap
        }
        Label {
            Layout.fillWidth: true
            visible: Bluetooth.Manager.pairingCode.length > 0
            text: Bluetooth.Manager.pairingCode
            textFormat: Text.PlainText
            wrapMode: Text.WrapAnywhere
            font.pixelSize: dialog.kind === "AuthorizeService" ? 14 : 28
            horizontalAlignment: Text.AlignHCenter
        }
        Label {
            visible: dialog.kind === "DisplayPasskey"
            text: qsTr("%1 digits entered").arg(Bluetooth.Manager.pairingEntered)
        }
        TextField {
            id: codeInput
            Layout.fillWidth: true
            visible: dialog.inputRequired
            maximumLength: dialog.kind === "RequestPasskey" ? 6 : 16
            inputMethodHints: dialog.kind === "RequestPasskey" ? Qt.ImhDigitsOnly : Qt.ImhNone
            onAccepted: Bluetooth.Manager.respondPairing(true, text)
        }
        Label {
            Layout.fillWidth: true
            visible: Bluetooth.Manager.errorText.length > 0
            text: Bluetooth.Manager.errorText
            textFormat: Text.PlainText
            wrapMode: Text.Wrap
        }
        RowLayout {
            Layout.alignment: Qt.AlignRight
            Button {
                text: qsTr("Cancel")
                onClicked: Bluetooth.Manager.cancelPairing()
            }
            Button {
                visible: !dialog.displayOnly
                text: dialog.kind === "RequestConfirmation" ? qsTr("Confirm") : qsTr("Accept")
                enabled: !dialog.inputRequired || codeInput.text.length > 0
                onClicked: Bluetooth.Manager.respondPairing(true, codeInput.text)
            }
        }
    }
}
