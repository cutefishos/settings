import QtQuick 2.15
import QtQuick.Controls 2.4
import QtQuick.Window 2.3

import Cutefish.Settings 1.0
import FishUI 1.0 as FishUI
import Cutefish.Network 1.0 as NM
import "../"

// One wireless network. Clicking connects; the current connection is managed
// from the info dialog. A password is asked for by the shell's secret agent
// once NetworkManager needs one.
ListRow {
    id: control

    // The state the row shows, which may lag behind NetworkManager's.
    property int displayedState: model.connectionState

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
        iconName: "info"
        iconColor: FishUI.Theme.highlightColor
        onClicked: {
            var component = Qt.createComponent("WirelessDetailsDialog.qml")
            if (component.status === Component.Ready) {
                var dialog = component.createObject(rootWindow, { "parentWindow": rootWindow })
                dialog.show()
                dialog.raise()
                dialog.requestActivate()
            }
        }
    }

    function connectNetwork() {
        if (model.uuid)
            handler.activateConnection(model.connectionPath, model.devicePath, model.specificPath)
        else
            handler.addAndActivateConnection(model.devicePath, model.specificPath)
    }
}
