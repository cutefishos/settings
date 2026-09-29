import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import FishUI 1.0 as FishUI
import Cutefish.Network 1.0 as NM
import "../"

// One wired connection: click activates or deactivates it.
ListRow {
    id: control

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
        Layout.rightMargin: Metrics.trailingGlyphMargin(name, width)
        name: "check"
        color: FishUI.Theme.highlightColor
        visible: control.checked
    }
}
