import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window

import Cutefish.Settings
import FishUI as FishUI
import Cutefish.Network as NM
import "../"

// One wireless network. Clicking connects; the current connection is managed
// from the info dialog. A password is asked for by the shell's secret agent
// once NetworkManager needs one.
ListRow {
    id: control

    // The model this row is a delegate of, for the details dialog.
    property var listModel

    // The state the row shows, which may lag behind NetworkManager's.
    property int displayedState: model.connectionState
    // Joined with a user name, which the join window asks for.
    readonly property bool enterprise: model.securityType === NM.Enums.WpaEap
                                       || model.securityType === NM.Enums.Wpa2Eap
                                       || model.securityType === NM.Enums.Wpa3SuiteB192

    text: model.itemUniqueName
    checked: model.connectionState === NM.Enums.Activated

    onClicked: {
        if (control.displayedState === NM.Enums.Deactivated)
            control.connectNetwork()
    }

    leading: FishUI.Icon {
        width: 18
        height: width
        name: model.connectionIcon
    }

    FishUI.BusyIndicator {
        id: busyIndicator
        width: 18
        height: width
        visible: control.displayedState === NM.Enums.Activating ||
                 control.displayedState === NM.Enums.Deactivating
        running: busyIndicator.visible
    }

    // Activated
    FishUI.Icon {
        name: "check"
        color: FishUI.Theme.highlightColor
        visible: control.checked
    }

    // Locked
    FishUI.Icon {
        width: 16
        height: 16
        name: "lock"
        visible: (model.securityType === -1 | model.securityType === 0) ? false : true
    }

    IconButton {
        width: 20
        Layout.rightMargin: Metrics.trailingGlyphMargin(iconName, width)
        iconName: "info"
        iconColor: FishUI.Theme.highlightColor
        onClicked: {
            var component = Qt.createComponent("WirelessDetailsDialog.qml")
            if (component.status === Component.Ready) {
                // A saved network by its connection, others by name.
                var saved = !!model.connectionPath
                var dialog = component.createObject(rootWindow, {
                    "parentWindow": rootWindow,
                    "listModel": control.listModel,
                    "trackedRole": saved ? "connectionPath" : "ssid",
                    "trackedValue": saved ? model.connectionPath : model.ssid
                })
                dialog.show()
                dialog.raise()
                dialog.requestActivate()
            }
        }
    }

    function connectNetwork() {
        if (model.uuid)
            handler.activateConnection(model.connectionPath, model.devicePath, model.specificPath)
        else if (enterprise)
            newNetworkDialog.openNetwork(model.specificPath, handler.wirelessNetwork(model.specificPath))
        else
            handler.addAndActivateConnection(model.devicePath, model.specificPath)
    }
}
