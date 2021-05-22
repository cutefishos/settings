import QtQuick 2.4
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0

import FishUI 1.0 as FishUI
import "../"

ItemPage {
    id: control
    headerTitle: qsTr("Power")

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent
            spacing: FishUI.Units.largeSpacing * 2

            RoundedItem {
                Label {
                    text: qsTr("Mode")
                    color: FishUI.Theme.disabledTextColor
                    bottomPadding: FishUI.Units.smallSpacing
                }

                RowLayout {
                    spacing: FishUI.Units.largeSpacing * 2

                    IconCheckBox {
                        source: "qrc:/images/powersave.svg"
                        text: qsTr("Power Save")
                        checked: true
                    }

                    IconCheckBox {
                        source: "qrc:/images/performance.svg"
                        text: qsTr("Performance")
                        checked: false
                    }
                }
            }
        }
    }
}
