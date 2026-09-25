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
import FishUI 1.0 as FishUI
import Cutefish.Settings 1.0
import "../"

ItemPage {
    id: batteryPage
    headerTitle: qsTr("Battery")

    property int historyType: BatteryHistoryModel.ChargeType
    readonly property var timespanComboChoices: [qsTr("Last hour"),qsTr("Last 2 hours"),qsTr("Last 12 hours"),qsTr("Last 24 hours"),qsTr("Last 48 hours"), qsTr("Last 7 days")]
    readonly property var timespanComboDurations: [3600, 7200, 43200, 86400, 172800, 604800]

    readonly property int chargePercent: battery.chargePercent

    onChargePercentChanged: {
        valueAnimation.restart()
    }

    NumberAnimation {
        id: valueAnimation
        target: percentLabel
        property: "value"
        from: percentLabel.value
        to: batteryPage.chargePercent
        duration: 500
    }

    Battery {
        id: battery

        Component.onCompleted: {
            // battery.refresh()
            batteryBackground.value = battery.chargePercent
        }
    }

    BatteryHistoryModel {
        id: history
        duration: timespanComboDurations[3]
        device: battery.udi
        type: BatteryHistoryModel.ChargeType
    }

    // Picks up new samples and moves the plot's right edge along with the clock,
    // only while the page is in front.
    Timer {
        interval: 60 * 1000
        running: battery.available && batteryPage.visible
                 && Qt.application.state === Qt.ApplicationActive
        repeat: true
        onTriggered: history.refresh()
        // The model loads by itself at first, so only a return needs a refresh.
        onRunningChanged: {
            if (running && history.count > 0)
                history.refresh()
        }
    }

    Connections {
        target: battery

        function onChargePercentChanged(value) {
            batteryBackground.value = battery.chargePercent
        }
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight
        visible: battery.available

        ColumnLayout {
            id: layout
            anchors.fill: parent
            spacing: Metrics.sectionSpacing

            // Battery Info
            BatteryItem {
                id: batteryBackground
                Layout.fillWidth: true
                // chargeState follows the service enum: NoCharge, Charging, Discharging, FullyCharged
                enableAnimation: battery.chargeState === 1 && battery.chargePercent < 100
                height: 150

                ColumnLayout {
                    anchors.fill: parent
                    anchors.leftMargin: Metrics.cardHorizontalPadding

                    Item {
                        Layout.fillHeight: true
                    }

                    RowLayout {
                        FishUI.Label {
                            property int value: 0

                            id: percentLabel
                            text: value
                            color: "white"
                            font.pointSize: 40
                            font.weight: Font.DemiBold
                        }

                        FishUI.Label {
                            text: "%"
                            color: "white"
                            font.pointSize: 12
                            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        }

                        Image {
                            id: sensorsVoltage
                            width: 30
                            height: width
                            sourceSize: Qt.size(width, height)
                            source: "qrc:/images/sensors-voltage-symbolic.svg"
                            visible: !battery.onBattery
                        }
                    }

                    FishUI.Label {
                        text: battery.statusString
                        color: "white"
                    }

                    Item {
                        Layout.fillHeight: true
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: Metrics.sectionLabelSpacing
                visible: history.count > 2

                SectionLabel {
                    text: qsTr("History")
                }

                RoundedItem {
                    spacing: 0

                    HistoryGraph {
                        Layout.fillWidth: true
                        height: 240

                        points: history.points
                        end: history.lastDataPointTime
                        duration: history.duration
                        yUnits: batteryPage.historyType === BatteryHistoryModel.RateType ? qsTr("W") : "%"
                        yMax: {
                            if (batteryPage.historyType === BatteryHistoryModel.RateType) {
                                // ceil to nearest 10
                                var max = Math.floor(history.largestValue)
                                max = max - max % 10 + 10

                                return max;
                            } else {
                                return 100;
                            }
                        }
                        yStep: batteryPage.historyType === BatteryHistoryModel.RateType ? 10 : 20
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: Metrics.sectionLabelSpacing
                visible: battery.capacity

                SectionLabel {
                    text: qsTr("Health")
                }

                ListCard {
                    ListRow {
                        interactive: false
                        text: qsTr("Condition")

                        // Only the current condition, tinted in its own colour.
                        Item {
                            readonly property color tint: battery.capacity >= 90 ? "#00CD23"
                                                          : battery.capacity >= 80 ? "#3385FF"
                                                          : "#FF8738"

                            implicitWidth: _conditionLabel.implicitWidth + Metrics.l
                            implicitHeight: _conditionLabel.implicitHeight + Metrics.s

                            FishUI.RoundedRectangle {
                                anchors.fill: parent
                                color: parent.tint
                                radius: FishUI.Theme.mediumRadius
                                opacity: 0.1
                            }

                            FishUI.Label {
                                id: _conditionLabel
                                anchors.centerIn: parent
                                text: battery.capacity >= 90 ? qsTr("Excellent")
                                      : battery.capacity >= 80 ? qsTr("Normal")
                                      : qsTr("Poor")
                                color: parent.tint
                            }
                        }
                    }

                    ListRow {
                        interactive: false
                        text: qsTr("Maximum Capacity")

                        FishUI.Label {
                            text: battery.capacity + "%"
                            color: FishUI.Theme.disabledTextColor
                        }
                    }

                    ListRow {
                        interactive: false
                        visible: battery.lastChargedPercent !== 0
                        text: qsTr("Last Charged to") + " " + battery.lastChargedPercent + "%"

                        FishUI.Label {
                            text: battery.lastChargedTime
                            color: FishUI.Theme.disabledTextColor
                        }
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: Metrics.sectionLabelSpacing
                visible: battery.chargeLimitSupported

                SectionLabel {
                    text: qsTr("Charging")
                }

                ListCard {
                    ListRow {
                        interactive: false
                        text: qsTr("Limit charging to %1%").arg(battery.chargeLimitEnd)

                        FishUI.Switch {
                            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                            rightPadding: 0
                            rightInset: 0
                            checked: battery.chargeLimitEnabled
                            onToggled: battery.setChargeLimitEnabled(checked)
                        }
                    }
                }

                FishUI.Label {
                    Layout.fillWidth: true
                    leftPadding: Metrics.cardHorizontalPadding
                    rightPadding: Metrics.cardHorizontalPadding
                    text: battery.chargeLimitHeld
                          ? qsTr("While plugged in, charging stops at %1% and resumes below %2%, so the battery wears more slowly.")
                            .arg(battery.chargeLimitEnd).arg(battery.chargeLimitStart)
                          : qsTr("This computer's battery controller did not keep the limit, so the battery still charges fully.")
                    color: battery.chargeLimitHeld ? FishUI.Theme.disabledTextColor : FishUI.Theme.orangeColor
                    wrapMode: Text.WordWrap
                }
            }

            ListCard {
                ListRow {
                    interactive: false
                    text: qsTr("Show percentage in status bar")

                    FishUI.Switch {
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                        rightPadding: 0
                        rightInset: 0
                        checked: battery.showPercent
                        onCheckedChanged: battery.setPercentEnabled(checked)
                    }
                }
            }

        }
    }

    FishUI.Label {
        anchors.centerIn: parent
        text: qsTr("No battery found")
        visible: !battery.available
    }
}
