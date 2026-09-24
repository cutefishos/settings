import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cutefish.Settings 1.0
import FishUI 1.0 as FishUI

FishUI.Window {
    id: control

    property var adapter

    title: adapter ? adapter.name : ""

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

    function rename() {
        if (nameField.text.length > 0 && nameField.text !== adapter.name)
            adapter.name = nameField.text
    }

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
                text: qsTr("Name")
                color: FishUI.Theme.disabledTextColor
            }

            FishUI.TextField {
                id: nameField
                Layout.fillWidth: true
                text: control.adapter ? control.adapter.name : ""
                maximumLength: 248
                selectByMouse: true
                onAccepted: control.rename()
            }

            FishUI.Label {
                text: qsTr("Address")
                color: FishUI.Theme.disabledTextColor
            }

            FishUI.Label {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignRight
                text: control.adapter ? control.adapter.address : ""
                textFormat: Text.PlainText
            }

            FishUI.Label {
                text: qsTr("Powered")
                color: FishUI.Theme.disabledTextColor
            }

            FishUI.Switch {
                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                rightPadding: 0
                rightInset: 0
                checked: !!control.adapter && control.adapter.powered
                onClicked: control.adapter.powered = checked
            }

            FishUI.Label {
                text: qsTr("Visible to Other Devices")
                color: FishUI.Theme.disabledTextColor
            }

            FishUI.Switch {
                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                rightPadding: 0
                rightInset: 0
                enabled: !!control.adapter && control.adapter.powered
                checked: !!control.adapter && control.adapter.discoverable
                onClicked: control.adapter.discoverable = checked
            }

            FishUI.Label {
                text: qsTr("Allow Incoming Pairing")
                color: FishUI.Theme.disabledTextColor
            }

            FishUI.Switch {
                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                rightPadding: 0
                rightInset: 0
                enabled: !!control.adapter && control.adapter.powered
                checked: !!control.adapter && control.adapter.pairable
                onClicked: control.adapter.pairable = checked
            }
        }

        RowLayout {
            spacing: Metrics.dialogButtonSpacing
            Layout.alignment: Qt.AlignRight

            FishUI.ConfirmDialogButton {
                text: qsTr("Close")
                onClicked: {
                    control.rename()
                    control.close()
                }
            }
        }
    }
}
