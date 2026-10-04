import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cutefish.Settings
import FishUI as FishUI
import "../"

// One Bluetooth device. Clicking pairs or connects; on a connected device, or
// one still pairing, it unfolds the disconnect or cancel control instead.
ListRow {
    id: control

    required property var device

    text: device.name
    checked: device.connected

    onClicked: {
        if (device.connected || (device.busy && !device.paired)) {
            expanded = !expanded
        } else if (device.busy) {
            return
        } else if (device.paired) {
            device.connectToDevice()
        } else {
            device.pair()
        }
    }

    Connections {
        target: control.device
        function onChanged() {
            if (!control.device.connected && !(control.device.busy && !control.device.paired))
                control.expanded = false
        }
    }

    leading: Image {
        width: 24
        height: width
        sourceSize: Qt.size(width, height)
        source: "image://icontheme/" + (control.device.icon || "bluetooth") + FishUI.Theme.iconThemeQuery
    }

    FishUI.BusyIndicator {
        width: 18
        height: width
        visible: control.device.busy
        running: visible
    }

    FishUI.Label {
        visible: text.length > 0
        color: FishUI.Theme.disabledTextColor
        text: {
            var status = control.device.blocked ? qsTr("Blocked")
                       : control.device.connected ? qsTr("Connected")
                       : control.device.paired ? qsTr("Not Connected")
                       : control.device.busy ? qsTr("Pairing…") : ""
            if (control.device.connected && control.device.batteryPercentage >= 0)
                status += " · " + qsTr("%1%").arg(control.device.batteryPercentage)
            return status
        }
    }

    IconButton {
        width: 20
        Layout.rightMargin: Metrics.trailingGlyphMargin(iconName, width)
        iconName: "info"
        iconColor: FishUI.Theme.highlightColor
        onClicked: {
            var component = Qt.createComponent("DeviceDetailsDialog.qml")
            if (component.status === Component.Ready) {
                var dialog = component.createObject(rootWindow, {
                    "parentWindow": rootWindow,
                    "device": control.device
                })
                dialog.show()
                dialog.raise()
                dialog.requestActivate()
            }
        }
    }

    details: [
        RowLayout {
            spacing: Metrics.controlSpacing

            FishUI.Button {
                visible: control.device.connected
                enabled: !control.device.busy
                text: qsTr("Disconnect")
                onClicked: control.device.disconnectFromDevice()
            }

            FishUI.Button {
                visible: control.device.busy && !control.device.paired
                text: qsTr("Cancel Pairing")
                onClicked: control.device.cancelPairing()
            }
        },

        Item {
            Layout.preferredHeight: Metrics.cardVerticalPadding
        }
    ]
}
