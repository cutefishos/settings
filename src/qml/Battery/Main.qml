import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Cutefish.Settings
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
