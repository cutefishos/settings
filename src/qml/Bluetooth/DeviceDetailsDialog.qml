import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cutefish.Settings
import FishUI as FishUI
import "../"

FishUI.Window {
    id: control

    property var device

    title: device ? device.name : ""

    x: parentWindow ? parentWindow.x + Math.round((parentWindow.width - width) / 2) : 0
    y: parentWindow ? parentWindow.y + Math.round((parentWindow.height - height) / 2) : 0
    width: 400
    property int contentHeight: detailsLayout.implicitHeight + header.height + 32
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
    // The page colour, so the cards stand out as they do on a page.
    background.color: FishUI.Theme.backgroundColor
    // The background item reaches a grown window a frame or two after its buffer does.
    color: background.color

    headerItem: FishUI.Label {
        anchors.left: parent.left
        anchors.leftMargin: Metrics.dialogPadding
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width - 64
        text: control.title
        textFormat: Text.PlainText
        font.pixelSize: 14
        font.weight: Font.DemiBold
        color: FishUI.Theme.textColor
        elide: Text.ElideRight
    }

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
                text: qsTr("Forget This Device")
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
