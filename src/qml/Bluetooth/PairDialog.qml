import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FishUI 1.0 as FishUI
import Cutefish.Bluetooth 1.0 as Bluetooth

FishUI.Window {
    id: dialog
    property var hostWindow
    parentWindow: hostWindow
    x: hostWindow ? hostWindow.x + Math.round((hostWindow.width - width) / 2) : 0
    y: hostWindow ? hostWindow.y + Math.round((hostWindow.height - height) / 2) : 0
    width: 420
    property int contentHeight: content.implicitHeight + header.height + 32
    height: contentHeight
    minimumWidth: width
    maximumWidth: width
    minimumHeight: contentHeight
    maximumHeight: contentHeight
    flags: Qt.Dialog | Qt.FramelessWindowHint
    modality: Qt.WindowModal
    visible: false
    minimizeButtonVisible: false
    header.height: 44
    contentTopMargin: header.height
    background.color: FishUI.Theme.secondBackgroundColor
    title: qsTr("Bluetooth pairing")

    headerItem: Label {
        anchors.left: parent.left
        anchors.leftMargin: 16
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width - 64
        text: dialog.title
        textFormat: Text.PlainText
        font.pixelSize: 14
        font.weight: Font.DemiBold
        color: FishUI.Theme.textColor
        elide: Text.ElideRight
    }

    readonly property string kind: Bluetooth.Manager.pairingKind
    readonly property bool inputRequired: kind === "RequestPinCode" || kind === "RequestPasskey"
    readonly property bool displayOnly: kind === "DisplayPinCode" || kind === "DisplayPasskey"

    Connections {
        target: Bluetooth.Manager
        function onPairingChanged() {
            if (Bluetooth.Manager.pairingKind.length) {
                if (!dialog.visible) {
                    codeInput.clear()
                    dialog.show()
                    dialog.raise()
                    dialog.requestActivate()
                    if (dialog.inputRequired)
                        codeInput.forceActiveFocus()
                }
            } else {
                dialog.close()
            }
        }
    }

    ColumnLayout {
        id: content
        anchors.fill: parent
        anchors.margins: 16
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
            spacing: 8

            FishUI.ConfirmDialogButton {
                text: qsTr("Cancel")
                onClicked: Bluetooth.Manager.cancelPairing()
            }
            FishUI.ConfirmDialogButton {
                visible: !dialog.displayOnly
                text: dialog.kind === "RequestConfirmation" ? qsTr("Confirm") : qsTr("Accept")
                primary: true
                enabled: !dialog.inputRequired || codeInput.text.length > 0
                onClicked: Bluetooth.Manager.respondPairing(true, codeInput.text)
            }
        }
    }
}
