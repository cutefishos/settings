import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cutefish.Settings
import FishUI as FishUI
import "../"

FishUI.DialogWindow {
    id: control

    property var device

    title: device ? device.name : ""

    width: 400
    contentHeight: detailsLayout.implicitHeight + 32
    // The page colour, so the cards stand out as they do on a page.
    background.color: FishUI.Theme.backgroundColor
    // The background item reaches a grown window a frame or two after its buffer does.
    color: background.color

    // Created per click, so each one goes away with its window.
    onClosing: destroy()

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
                text: qsTr("Address")
                value: control.device ? control.device.address : ""
            }

            InfoRow {
                text: qsTr("Adapter")
                value: control.device ? control.device.adapterPath.split("/").pop() : ""
            }

            InfoRow {
                text: qsTr("Battery")
                value: !!control.device && control.device.batteryPercentage >= 0
                       ? qsTr("%1%").arg(control.device.batteryPercentage) : ""
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
