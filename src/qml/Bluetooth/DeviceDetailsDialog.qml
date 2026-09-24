import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cutefish.Settings 1.0
import FishUI 1.0 as FishUI

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
    background.color: FishUI.Theme.secondBackgroundColor

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
        anchors.fill: parent
        anchors.margins: Metrics.dialogPadding
        spacing: Metrics.dialogSpacing

        GridLayout {
            Layout.fillWidth: true
            columns: 2
            columnSpacing: Metrics.dialogSpacing
            rowSpacing: Metrics.s

            FishUI.Label {
                text: qsTr("Address")
                color: FishUI.Theme.disabledTextColor
            }

            FishUI.Label {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignRight
                text: control.device ? control.device.address : ""
                textFormat: Text.PlainText
            }

            FishUI.Label {
                text: qsTr("Adapter")
                color: FishUI.Theme.disabledTextColor
            }

            FishUI.Label {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignRight
                text: control.device ? control.device.adapterPath.split("/").pop() : ""
                textFormat: Text.PlainText
            }

            FishUI.Label {
                visible: batteryLabel.visible
                text: qsTr("Battery")
                color: FishUI.Theme.disabledTextColor
            }

            FishUI.Label {
                id: batteryLabel
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignRight
                visible: !!control.device && control.device.batteryPercentage >= 0
                text: control.device ? qsTr("%1%").arg(control.device.batteryPercentage) : ""
            }

            FishUI.Label {
                visible: trustedSwitch.visible
                text: qsTr("Trusted")
                color: FishUI.Theme.disabledTextColor
            }

            FishUI.Switch {
                id: trustedSwitch
                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                rightPadding: 0
                rightInset: 0
                visible: !!control.device && control.device.paired
                enabled: !!control.device && !control.device.busy
                checked: !!control.device && control.device.trusted
                onClicked: control.device.trusted = checked
            }

            FishUI.Label {
                text: qsTr("Blocked")
                color: FishUI.Theme.disabledTextColor
            }

            FishUI.Switch {
                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                rightPadding: 0
                rightInset: 0
                enabled: !!control.device && !control.device.busy
                checked: !!control.device && control.device.blocked
                onClicked: control.device.blocked = checked
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
