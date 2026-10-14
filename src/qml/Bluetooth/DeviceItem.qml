import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cutefish.Settings
import FishUI as FishUI
import "../"

// One Bluetooth device. Clicking pairs or connects; on a connected device, or
// one still pairing, the disconnect or cancel action shows on the right while
// the row is hovered, or after a tap where there is no hover.
ListRow {
    id: control

    required property var device

    // BlueZ reports Connected as soon as the link for pairing is up, before
    // authentication finishes.
    readonly property bool pairing: device.busy && !device.paired
    readonly property bool linked: device.connected && !pairing
    readonly property bool connectable: !linked && !pairing && !device.busy && !device.blocked
    property bool revealed: false

    Accessible.name: device.name
    checked: linked
    highlightOnPress: connectable
    minimumRowHeight: statusLabel.visible ? Metrics.twoLineRowHeight : Metrics.rowHeight

    onClicked: {
        if (connectable) {
            if (device.paired)
                device.connectToDevice()
            else
                device.pair()
        } else {
            revealed = true
        }
    }

    HoverHandler {
        id: hoverHandler
        onHoveredChanged: {
            if (!hovered)
                control.revealed = false
        }
    }

    leading: Image {
        width: 24
        height: width
        sourceSize: Qt.size(width, height)
        source: "image://icontheme/" + (control.device.icon || "bluetooth") + FishUI.Theme.iconThemeQuery
    }

    ColumnLayout {
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignVCenter
        spacing: Metrics.textSpacing

        FishUI.Label {
            id: primaryLabel
            Layout.fillWidth: true
            text: control.device.name
            textFormat: Text.PlainText
            color: control.enabled ? FishUI.Theme.textColor : FishUI.Theme.disabledTextColor
            elide: Text.ElideRight
        }

        FishUI.Label {
            id: statusLabel
            Layout.fillWidth: true
            visible: text.length > 0
            text: {
                var status = control.device.blocked ? qsTr("Blocked")
                           : control.linked ? qsTr("Connected")
                           : control.device.paired ? qsTr("Not connected") : ""
                if (control.linked && control.device.batteryPercentage >= 0)
                    status += " · " + qsTr("%1%").arg(control.device.batteryPercentage)
                return status
            }
            textFormat: Text.PlainText
            color: FishUI.Theme.disabledTextColor
            font.pixelSize: Math.max(1, primaryLabel.font.pixelSize - 1)
            elide: Text.ElideRight
        }
    }

    FishUI.BusyIndicator {
        width: 18
        height: width
        visible: control.device.busy
        running: visible
    }

    // The busy indicator stands in for the action, except that pairing can be cancelled.
    FishUI.Button {
        visible: !control.device.blocked && (control.pairing || (control.linked && !control.device.busy))
                 && (hoverHandler.hovered || control.revealed)
        focusPolicy: Qt.NoFocus
        text: control.pairing ? qsTr("Cancel pairing") : qsTr("Disconnect")
        onClicked: {
            if (control.pairing)
                control.device.cancelPairing()
            else
                control.device.disconnectFromDevice()
        }
    }

    IconButton {
        visible: control.device.paired
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
}
