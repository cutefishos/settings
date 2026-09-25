import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import FishUI 1.0 as FishUI
import Cutefish.Settings 1.0
import "../"

ItemPage {
    headerTitle: qsTr("Touchscreen")

    Touchscreen {
        id: touchscreen
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent
            spacing: Metrics.sectionSpacing

            ListCard {
                visible: touchscreen.available

                Repeater {
                    model: touchscreen.devices

                    ListRow {
                        required property string modelData

                        interactive: false
                        text: modelData

                        FishUI.Switch {
                            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                            checked: touchscreen.disabledDevices.indexOf(modelData) < 0
                            rightPadding: 0
                            rightInset: 0
                            onToggled: touchscreen.setDeviceEnabled(modelData, checked)
                        }
                    }
                }
            }
        }
    }
}
