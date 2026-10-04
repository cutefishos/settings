import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FishUI as FishUI
import Cutefish.Settings
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
