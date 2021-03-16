import QtQuick 2.4
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0

import MeuiKit 1.0 as Meui
import Cutefish.NetworkManagement 1.0 as NM

Item {
    id: control

    RowLayout {
        anchors.fill: parent
        spacing: Meui.Units.largeSpacing

        Image {
            width: 22
            height: width
            sourceSize: Qt.size(width, height)
            source: "qrc:/images/" + (Meui.Theme.darkMode ? "dark/" : "light/") + "network-wired.svg"
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
                color: Meui.Theme.highlightColor
                opacity: 1
                visible: true
            }
        }
    }
}
