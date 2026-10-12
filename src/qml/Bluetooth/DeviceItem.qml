import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cutefish.Settings
import FishUI as FishUI
import "../"

// One Bluetooth device. Its action shows on the right while the row is
// hovered, or after a tap where there is no hover; the row itself does nothing.
ListRow {
    id: control

    required property var device

    // BlueZ reports Connected as soon as the link for pairing is up, before
    // authentication finishes.
    readonly property bool pairing: device.busy && !device.paired
    readonly property bool linked: device.connected && !pairing
    property bool revealed: false

    Accessible.name: device.name
    checked: linked
    minimumRowHeight: statusLabel.visible ? Metrics.twoLineRowHeight : Metrics.rowHeight

    onClicked: revealed = true

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
        visible: !control.device.blocked && (control.pairing || !control.device.busy)
                 && (hoverHandler.hovered || control.revealed)
        focusPolicy: Qt.NoFocus
        text: control.pairing ? qsTr("Cancel pairing")
              : control.linked ? qsTr("Disconnect")
              : qsTr("Connect")
        onClicked: {
            if (control.pairing)
                control.device.cancelPairing()
            else if (control.linked)
                control.device.disconnectFromDevice()
            else if (control.device.paired)
                control.device.connectToDevice()
            else
                control.device.pair()
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
