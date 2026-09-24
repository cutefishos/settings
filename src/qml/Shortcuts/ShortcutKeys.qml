import QtQuick 2.12
import QtQuick.Layouts 1.12
import Cutefish.Settings 1.0
import FishUI 1.0 as FishUI

// One shortcut drawn as key caps: [Ctrl] + [Shift] + [Q].
Row {
    id: control

    property var parts: []
    property bool dimmed: false
    property color capColor: FishUI.Theme.darkMode ? Qt.lighter(FishUI.Theme.secondBackgroundColor, 1.6)
                                                   : Qt.darker(FishUI.Theme.secondBackgroundColor, 1.08)
    spacing: Metrics.xs

    Repeater {
        model: control.parts

        Row {
            spacing: control.spacing

            Text {
                visible: index > 0
                anchors.verticalCenter: parent.verticalCenter
                text: "+"
                color: FishUI.Theme.disabledTextColor
                font.pointSize: 9
            }

            FishUI.RoundedRectangle {
                width: capLabel.implicitWidth + 14
                height: 24
                radius: 6
                color: control.capColor
                opacity: control.dimmed ? 0.5 : 1

                Text {
                    id: capLabel
                    anchors.centerIn: parent
                    text: modelData
                    color: FishUI.Theme.textColor
                    font.pointSize: 9
                }
            }
        }
    }
}
