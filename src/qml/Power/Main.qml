/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     revenmartin <revenmartin@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.4
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import Qt5Compat.GraphicalEffects

import Cutefish.Settings 1.0
import FishUI 1.0 as FishUI
import "../"

ItemPage {
    id: control
    headerTitle: qsTr("Power")

    PowerManager {
        id: power
    }

    Battery {
        id: battery
    }

    property var timeoutValues: [60, 120, 300, 600, 900, 1200, 1800, -1]

    function timeoutToIndex(timeout) {
        var index = timeoutValues.indexOf(timeout)
        return index >= 0 ? index : timeoutValues.indexOf(300)
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent
            spacing: FishUI.Units.largeSpacing

            RoundedItem {
                Label {
                    text: qsTr("Mode")
                    color: FishUI.Theme.disabledTextColor
                }

                RowLayout {
                    spacing: FishUI.Units.largeSpacing * 2

                    IconCheckBox {
                        source: "qrc:/images/powersave.svg"
                        text: qsTr("Power Save")
                        checked: power.mode === 0
                        onClicked: power.mode = 0
                    }

                    IconCheckBox {
                        source: "qrc:/images/performance.svg"
                        text: qsTr("Performance")
                        checked: power.mode === 1
                        onClicked: power.mode = 1
                    }
                }
            }

            Label {
                color: FishUI.Theme.disabledTextColor
                leftPadding: FishUI.Units.largeSpacing * 2
                rightPadding: FishUI.Units.largeSpacing
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: qsTr("Performance mode: CPU and GPU frequencies will be increased, while power consumption and heat generation will be increased.")
            }

            RoundedItem {
                Layout.topMargin: FishUI.Units.largeSpacing

                GridLayout {
                    columns: 2
                    rowSpacing: FishUI.Units.largeSpacing * 2
                    Layout.bottomMargin: FishUI.Units.largeSpacing

                    Label {
                        visible: battery.available
                        text: qsTr("Turn off display when inactive on battery")
                        Layout.fillWidth: true
                    }

                    ComboBox {
                        visible: battery.available
                        Layout.preferredWidth: 160

                        model: [qsTr("1 Minute"), qsTr("2 Minutes"), qsTr("5 Minutes"),
                                qsTr("10 Minutes"), qsTr("15 Minutes"), qsTr("20 Minutes"),
                                qsTr("30 Minutes"), qsTr("Never")]
                        currentIndex: timeoutToIndex(power.batteryScreenOff)
                        onActivated: power.batteryScreenOff = timeoutValues[currentIndex]
                    }

                    Label {
                        text: qsTr("Turn off display when inactive on power adapter")
                        Layout.fillWidth: true
                    }

                    ComboBox {
                        Layout.preferredWidth: 160
                        model: [qsTr("1 Minute"), qsTr("2 Minutes"), qsTr("5 Minutes"),
                                qsTr("10 Minutes"), qsTr("15 Minutes"), qsTr("20 Minutes"),
                                qsTr("30 Minutes"), qsTr("Never")]
                        currentIndex: timeoutToIndex(power.acScreenOff)
                        onActivated: power.acScreenOff = timeoutValues[currentIndex]
                    }

                }
            }

            Item {
                height: FishUI.Units.largeSpacing * 2
            }
        }
    }
}
