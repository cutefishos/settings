import QtQuick 2.4
import QtQuick.Window 2.12
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import Cutefish.Settings 1.0
import FishUI 1.0 as FishUI
import "../"

Window {
    id: control

    width: 200
    height: 100

    minimumWidth: 200
    minimumHeight: 100
    maximumWidth: 200
    maximumHeight: 100

    modality: Qt.WindowModal
    flags: Qt.WindowStaysOnTopHint
    visible: false
    title: " "

    Rectangle {
        anchors.fill: parent
        color: FishUI.Theme.secondBackgroundColor
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: FishUI.Units.largeSpacing

        Label {
            text: qsTr("Bluetooth Pairing Request")
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
