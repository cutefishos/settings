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

import Cutefish.Settings 1.0
import FishUI 1.0 as FishUI
import "../"
import "../Battery"

ItemPage {
    id: control
    headerTitle: qsTr("Power")

    PowerManager {
        id: power
    }

    Battery {
        id: _battery
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
            spacing: Metrics.sectionSpacing

            BatterySection {
                Layout.fillWidth: true
                battery: _battery
                visible: _battery.available
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: Metrics.sectionLabelSpacing

                SectionLabel {
                    text: qsTr("Mode")
                }

                RoundedItem {
                    RowLayout {
                        spacing: Metrics.tileSpacing

                        ImageSelectionItem {
                            source: "qrc:/images/powersave.svg"
                            text: qsTr("Power Save")
                            previewWidth: 80
                            previewHeight: 80
                            checked: power.mode === 0
                            onClicked: power.mode = 0
                        }

                        ImageSelectionItem {
                            source: "qrc:/images/performance.svg"
                            text: qsTr("Performance")
                            previewWidth: 80
                            previewHeight: 80
                            checked: power.mode === 1
                            onClicked: power.mode = 1
                        }
                    }
                }

                FishUI.Label {
                    color: FishUI.Theme.disabledTextColor
                    leftPadding: Metrics.cardHorizontalPadding
                    rightPadding: Metrics.cardHorizontalPadding
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    text: qsTr("Performance mode: CPU and GPU frequencies will be increased, while power consumption and heat generation will be increased.")
                }
            }

            ListCard {
                ListRow {
                    interactive: false
                    visible: _battery.available
                    minimumRowHeight: Metrics.controlRowHeight
                    text: qsTr("Turn off display when inactive on battery")

                    FishUI.ComboBox {
                        Layout.preferredWidth: 160
                        model: [qsTr("1 Minute"), qsTr("2 Minutes"), qsTr("5 Minutes"),
                                qsTr("10 Minutes"), qsTr("15 Minutes"), qsTr("20 Minutes"),
                                qsTr("30 Minutes"), qsTr("Never")]
                        currentIndex: timeoutToIndex(power.batteryScreenOff)
                        onActivated: power.batteryScreenOff = timeoutValues[currentIndex]
                    }
                }

                ListRow {
                    interactive: false
                    minimumRowHeight: Metrics.controlRowHeight
                    text: qsTr("Turn off display when inactive on power adapter")

                    FishUI.ComboBox {
                        Layout.preferredWidth: 160
                        model: [qsTr("1 Minute"), qsTr("2 Minutes"), qsTr("5 Minutes"),
                                qsTr("10 Minutes"), qsTr("15 Minutes"), qsTr("20 Minutes"),
                                qsTr("30 Minutes"), qsTr("Never")]
                        currentIndex: timeoutToIndex(power.acScreenOff)
                        onActivated: power.acScreenOff = timeoutValues[currentIndex]
                    }
                }
            }

        }
    }
}
