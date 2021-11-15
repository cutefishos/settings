import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import Cutefish.Settings 1.0
import FishUI 1.0 as FishUI
import "../"

FishUI.Window {
    id: control

    width: contentWidth
    height: contentHeight

    property int contentWidth: mainLayout.implicitWidth + FishUI.Units.largeSpacing * 2 + control.header.height
    property int contentHeight: mainLayout.implicitHeight + FishUI.Units.largeSpacing * 2 + control.header.height

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
    headerItem: Item {
        Label {
            anchors.fill: parent
            anchors.leftMargin: FishUI.Units.largeSpacing
            text: control.title
        }
    }

    DragHandler {
        target: null
        acceptedDevices: PointerDevice.GenericPointer
        grabPermissions: PointerHandler.CanTakeOverFromItems | PointerHandler.CanTakeOverFromHandlersOfDifferentType | PointerHandler.ApprovesTakeOverByAnything
        onActiveChanged: if (active) { control.helper.startSystemMove(control) }
    }

    ColumnLayout {
        id: mainLayout
        anchors.fill: parent
        anchors.margins: FishUI.Units.largeSpacing

        Label {
            text: qsTr("Bluetooth Pairing Request")
            Layout.alignment: Qt.AlignHCenter
        }

        Label {
            text: "<b>%1</b>".arg(control.pin)
            visible: control.pin !== ""
            font.pointSize: 16

            Layout.alignment: Qt.AlignHCenter
            Layout.bottomMargin: FishUI.Units.largeSpacing
        }

        RowLayout {
            spacing: FishUI.Units.largeSpacing

            Button {
                text: qsTr("Cancel")
                Layout.fillWidth: true
                onClicked: {
                    control.visible = false
                    bluetoothMgr.confirmMatchButton(false)
                }
            }

            Button {
                text: qsTr("OK")
                Layout.fillWidth: true
                flat: true
                onClicked: {
                    control.visible = false
                    bluetoothMgr.confirmMatchButton(true)
                }
            }
        }
    }
}
