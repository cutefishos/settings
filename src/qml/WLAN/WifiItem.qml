import QtQuick 2.15
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import QtQuick.Window 2.3

import Cutefish.Settings 1.0
import FishUI 1.0 as FishUI
import Cutefish.Network 1.0 as NM
import "../"

// One wireless network. Clicking connects, or unfolds the connect/disconnect
// controls for the current connection. A password is asked for by the shell's
// secret agent once NetworkManager needs one.
ListRow {
    id: control

    text: model.itemUniqueName
    checked: model.connectionState === NM.Enums.Activated

    onClicked: {
        if (busyIndicator.visible)
            return

        if (connectionState === NM.Enums.Deactivated)
            control.connectNetwork()
        else
            expanded = !expanded
    }

    leading: Image {
        width: 18
        height: width
        sourceSize: Qt.size(width, height)
        source: "qrc:/images/" + (FishUI.Theme.darkMode ? "dark/" : "light/") + model.connectionIcon + ".svg"
        smooth: false
    }

    FishUI.BusyIndicator {
        id: busyIndicator
        width: 18
        height: width
        visible: connectionState === NM.Enums.Activating ||
                 connectionState === NM.Enums.Deactivating
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

    details: [
        RowLayout {
            spacing: Metrics.controlSpacing

            FishUI.Button {
                visible: connectionState === NM.Enums.Deactivated
                text: qsTr("Connect")
                flat: true

                onClicked: control.connectNetwork()
            }

            FishUI.Button {
                visible: connectionState === NM.Enums.Activated
                text: qsTr("Disconnect")

                onClicked: {
                    handler.deactivateConnection(model.connectionPath, model.devicePath)
                }
            }
        },

        Item {
            Layout.preferredHeight: Metrics.cardVerticalPadding
        }
    ]

    function connectNetwork() {
        if (model.uuid)
            handler.activateConnection(model.connectionPath, model.devicePath, model.specificPath)
        else
            handler.addAndActivateConnection(model.devicePath, model.specificPath)
    }
}
