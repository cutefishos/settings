import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import Cutefish.Settings 1.0
import FishUI 1.0 as FishUI
import "../"

FishUI.Window {
    id: control

    windowRadius: 16
    windowButtonsTopMargin: 8
    windowButtonsRightMargin: 8
    minimizeButtonVisible: false
    header.height: 44
    contentTopMargin: 0

    width: contentWidth
    height: contentHeight

    property int contentWidth: 360
    property int contentHeight: mainLayout.implicitHeight + 32

    minimumWidth: contentWidth
    minimumHeight: contentHeight
    maximumWidth: contentWidth
    maximumHeight: contentHeight

    modality: Qt.WindowModal
    flags: Qt.Dialog | Qt.FramelessWindowHint
    visible: false
    title: " "

    property var pin: ""

    background.color: FishUI.Theme.secondBackgroundColor

    DragHandler {
        target: null
        acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
        grabPermissions: PointerHandler.CanTakeOverFromItems | PointerHandler.CanTakeOverFromHandlersOfDifferentType | PointerHandler.ApprovesTakeOverByAnything
        onActiveChanged: if (active) { control.helper.startSystemMove(control) }
    }

    ColumnLayout {
        id: mainLayout
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16

        Label {
            Layout.fillWidth: true
            Layout.rightMargin: 32
            text: "Bluetooth Pairing Request"
            textFormat: Text.PlainText
            font.pixelSize: 14
            font.weight: Font.DemiBold
            color: FishUI.Theme.textColor
            wrapMode: Text.Wrap
        }

        Label {
            Layout.fillWidth: true
            text: control.title.trim()
            textFormat: Text.PlainText
            font.pixelSize: 14
            color: FishUI.Theme.disabledTextColor
            wrapMode: Text.Wrap
            visible: text.length > 0
        }

        Label {
            text: control.pin
            textFormat: Text.PlainText
            visible: control.pin !== ""
            font.pixelSize: 24
            font.weight: Font.DemiBold
            Layout.alignment: Qt.AlignHCenter
        }

        RowLayout {
            spacing: 8
            Layout.alignment: Qt.AlignRight

            FishUI.ConfirmDialogButton {
                text: qsTr("Cancel")
                onClicked: {
                    control.visible = false
                    bluetoothMgr.confirmMatchButton(false)
                }
            }

            FishUI.ConfirmDialogButton {
                text: qsTr("OK")
                primary: true
                onClicked: {
                    control.visible = false
                    bluetoothMgr.confirmMatchButton(true)
                }
            }
        }
    }
}
