import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FishUI as FishUI
import Cutefish.Settings
import Cutefish.Network as NM
import "../"

// One VPN: its switch connects and disconnects it; below its name, its type, or why it
// can't connect.
ListRow {
    id: control

    readonly property bool busy: model.connectionState === NM.Enums.Activating
                                 || model.connectionState === NM.Enums.Deactivating
    readonly property bool up: model.connectionState === NM.Enums.Activated
                               || model.connectionState === NM.Enums.Activating
    // Signing in takes a user name the imported file didn't have.
    readonly property bool needsUserName: model.vpnUsesUserName && model.vpnUserName.length === 0

    Accessible.name: model.name
    onClicked: openDetails()

    leading: FishUI.Icon {
        width: 18
        height: width
        name: "network-vpn"
    }

    ColumnLayout {
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignVCenter
        spacing: Metrics.textSpacing

        FishUI.Label {
            id: primaryLabel
            Layout.fillWidth: true
            text: model.name
            textFormat: Text.PlainText
            color: control.enabled ? FishUI.Theme.textColor : FishUI.Theme.disabledTextColor
            elide: Text.ElideRight
        }

        FishUI.Label {
            Layout.fillWidth: true
            text: model.statusMessage.length > 0 ? model.statusMessage
                  : model.connectionState === NM.Enums.Activating ? qsTr("Connecting…")
                  : control.needsUserName ? qsTr("Add your user name to connect.")
                  : model.vpnType
            textFormat: Text.PlainText
            color: model.statusMessage.length > 0 && model.canActivate ? FishUI.Theme.redColor
                                                                        : FishUI.Theme.disabledTextColor
            font.pixelSize: Math.max(1, primaryLabel.font.pixelSize - 1)
            wrapMode: Text.Wrap
        }
    }

    FishUI.BusyIndicator {
        width: 18
        height: width
        visible: control.busy
        running: visible
    }

    FishUI.Switch {
        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
        rightPadding: 0
        rightInset: 0
        enabled: model.canActivate || control.up
        checked: control.up
        onToggled: {
            if (checked && control.needsUserName)
                control.openDetails()
            else if (checked)
                handler.activateConnection(model.connectionPath, "", "")
            else
                handler.deactivateConnection(model.connectionPath, "")
            // The switch shows what NetworkManager reports, not what was asked for.
            checked = Qt.binding(function() { return control.up })
        }
    }

    IconButton {
        width: 20
        Layout.rightMargin: Metrics.trailingGlyphMargin(iconName, width)
        iconName: "info"
        iconColor: FishUI.Theme.highlightColor
        onClicked: control.openDetails()
    }

    function openDetails() {
        var component = Qt.createComponent("VpnDetailsDialog.qml")
        if (component.status === Component.Ready) {
            var dialog = component.createObject(rootWindow, {
                "parentWindow": rootWindow,
                "listModel": vpnModel,
                "connectionPath": model.connectionPath
            })
            dialog.show()
            dialog.raise()
            dialog.requestActivate()
        }
    }
}
