import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import FishUI as FishUI
import Cutefish.Network as NM
import "../"

// One wired connection: click activates or deactivates it; the info button opens its details.
ListRow {
    id: control

    // The model this row is a delegate of, for the details dialog.
    property var listModel

    text: model.itemUniqueName
    checked: model.connectionState === NM.NetworkModel.Activated

    onClicked: {
        if (model.connectionState === NM.Enums.Deactivated) {
            handler.activateConnection(model.connectionPath, model.devicePath, model.specificPath)
        } else {
            handler.deactivateConnection(model.connectionPath, model.devicePath)
        }
    }

    leading: Image {
        width: 18
        height: width
        sourceSize: Qt.size(width, height)
        source: "qrc:/images/" + (FishUI.Theme.darkMode ? "dark/" : "light/") + "network-wired.svg"
    }

    FishUI.Icon {
        name: "check"
        color: FishUI.Theme.highlightColor
        visible: control.checked
    }

    IconButton {
        width: 20
        Layout.rightMargin: Metrics.trailingGlyphMargin(iconName, width)
        iconName: "info"
        iconColor: FishUI.Theme.highlightColor
        onClicked: {
            var component = Qt.createComponent("WiredDetailsDialog.qml")
            if (component.status !== Component.Ready)
                return
            var dialog = component.createObject(rootWindow, {
                "parentWindow": rootWindow,
                "listModel": control.listModel,
                "connectionPath": model.connectionPath
            })
            dialog.show()
            dialog.raise()
            dialog.requestActivate()
        }
    }
}
