import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cutefish.Settings
import FishUI as FishUI
import "../"

FishUI.DialogWindow {
    id: control

    property var device
    // The Device Information service shows up once the device's services resolve.
    readonly property bool servicesResolved: !!device && device.servicesResolved

    title: device ? device.name : ""

    width: 400
    contentHeight: detailsLayout.implicitHeight + 32
    // The page colour, so the cards stand out as they do on a page.
    background.color: FishUI.Theme.backgroundColor
    // The background item reaches a grown window a frame or two after its buffer does.
    color: background.color

    // Created per click, so each one goes away with its window.
    onClosing: destroy()

    onServicesResolvedChanged: {
        if (servicesResolved)
            device.readDeviceInformation()
    }
    Component.onCompleted: {
        if (device)
            device.readDeviceInformation()
    }

    // The kind of device behind each icon name BlueZ reports.
    function deviceType(icon) {
        var types = {
            "audio-card": qsTr("Audio device"),
            "audio-headphones": qsTr("Headphones"),
            "audio-headset": qsTr("Headset"),
            "camera-photo": qsTr("Camera"),
            "camera-video": qsTr("Video camera"),
            "computer": qsTr("Computer"),
            "input-gaming": qsTr("Game controller"),
            "input-keyboard": qsTr("Keyboard"),
            "input-mouse": qsTr("Mouse"),
            "input-tablet": qsTr("Drawing tablet"),
            "modem": qsTr("Modem"),
            "multimedia-player": qsTr("Media player"),
            "network-wireless": qsTr("Network device"),
            "phone": qsTr("Phone"),
            "printer": qsTr("Printer"),
            "scanner": qsTr("Scanner"),
            "video-display": qsTr("Display")
        }
        return types[icon] || ""
    }

    ColumnLayout {
        id: detailsLayout
        // Not filled to the window: the window height follows this layout's implicit
        // height a polish later, and a filled layout would be spread over the old height.
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: Metrics.dialogPadding
        spacing: Metrics.dialogSpacing

        ListCard {
            InfoRow {
                text: qsTr("Device type")
                value: control.device ? control.deviceType(control.device.icon) : ""
            }

            InfoRow {
                text: qsTr("Model number")
                value: control.device ? control.device.modelNumber : ""
            }

            InfoRow {
                text: qsTr("Version")
                value: control.device ? control.device.firmwareRevision : ""
            }

            InfoRow {
                text: qsTr("Address")
                value: control.device ? control.device.address : ""
            }
        }

        ListCard {
            ListRow {
                interactive: false
                visible: !!control.device && control.device.paired
                text: qsTr("Trusted")

                FishUI.Switch {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                    rightPadding: 0
                    rightInset: 0
                    enabled: !!control.device && !control.device.busy
                    checked: !!control.device && control.device.trusted
                    onClicked: control.device.trusted = checked
                }
            }

            ListRow {
                interactive: false
                text: qsTr("Blocked")

                FishUI.Switch {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                    rightPadding: 0
                    rightInset: 0
                    enabled: !!control.device && !control.device.busy
                    checked: !!control.device && control.device.blocked
                    onClicked: control.device.blocked = checked
                }
            }
        }

        RowLayout {
            spacing: Metrics.dialogButtonSpacing
            Layout.alignment: Qt.AlignRight

            FishUI.ConfirmDialogButton {
                text: qsTr("Close")
                onClicked: control.close()
            }

            FishUI.ConfirmDialogButton {
                visible: !!control.device && control.device.paired
                enabled: !!control.device && !control.device.busy
                text: qsTr("Forget this device")
                primary: true
                destructive: true
                onClicked: {
                    control.device.forget()
                    control.close()
                }
            }
        }
    }
}
