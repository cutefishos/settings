import QtQuick 2.4
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0

import FishUI 1.0 as FishUI
import Cutefish.NetworkManagement 1.0 as NM

Item {
    id: control

    MouseArea {
        anchors.fill: parent
        onClicked: {
            if (model.connectionState === NM.Enums.Deactivated) {
                handler.activateConnection(model.connectionPath, model.devicePath, model.specificPath)
            } else {
                handler.deactivateConnection(model.connectionPath, model.devicePath)
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: FishUI.Units.largeSpacing

        Image {
            width: 22
            height: width
            sourceSize: Qt.size(width, height)
            source: "qrc:/images/" + (FishUI.Theme.darkMode ? "dark/" : "light/") + "network-wired.svg"
        }

        Label {
            text: model.itemUniqueName
            Layout.fillWidth: true
        }

        // Activated
        Image {
            width: 16
            height: width
            sourceSize: Qt.size(width, height)
            source: "qrc:/images/checked.svg"
            visible: model.connectionState === NM.NetworkModel.Activated

            ColorOverlay {
                anchors.fill: parent
                source: parent
                color: FishUI.Theme.highlightColor
                opacity: 1
                visible: true
            }
        }
    }
}
