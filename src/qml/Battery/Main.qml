import QtQuick 2.4
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3

import Cutefish.Settings 1.0
import "../"

ItemPage {
    id: control
    headerTitle: qsTr("Battery")

    Battery {
        id: _battery
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent
            spacing: Metrics.sectionSpacing

            BatterySection {
                Layout.fillWidth: true
                battery: _battery
            }
        }
    }
}
