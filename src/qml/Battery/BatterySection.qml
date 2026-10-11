// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FishUI as FishUI
import Cutefish.Settings
import "../"

ColumnLayout {
    id: section
    spacing: Metrics.sectionSpacing

    required property Battery battery

    property int historyType: BatteryHistoryModel.ChargeType
    readonly property var historyDurations: [86400, 7 * 86400]
    // The range the plotted samples cover. It follows the picker only once
    // they arrive, so the old samples are never drawn across the new range.
    property real plottedDuration: historyDurations[0]

    BatteryHistoryModel {
        id: history
        duration: section.historyDurations[rangePicker.currentIndex]
        device: battery.udi
        type: BatteryHistoryModel.ChargeType
        onDataChanged: section.plottedDuration = duration
    }

    // Picks up new samples and moves the plot's right edge along with the clock,
    // only while the page is in front.
    Timer {
        interval: 60 * 1000
        running: battery.available && section.visible
                 && Qt.application.state === Qt.ApplicationActive
        repeat: true
        onTriggered: history.refresh()
        // The model loads by itself at first, so only a return needs a refresh.
        onRunningChanged: {
            if (running && history.count > 0)
                history.refresh()
        }
    }

    ColumnLayout {
        Layout.fillWidth: true
        spacing: Metrics.sectionLabelSpacing
        visible: history.count > 2

        RowLayout {
            Layout.fillWidth: true
            spacing: Metrics.m

            SectionLabel {
                text: qsTr("History")
            }

            FishUI.SegmentedControl {
                id: rangePicker
                // Two equal segments, each wide enough for the longer label. A whole
                // width, as the layout rounds the position and the right edge must meet the card's.
                Layout.preferredWidth: Math.ceil(Math.max(dayButton.implicitWidth, weekButton.implicitWidth) * 2
                                                 + Metrics.l * 4 + leftPadding + rightPadding)
                // Compact, to sit in the title row.
                contentHeight: 28

                FishUI.TabButton {
                    id: dayButton
                    text: qsTr("24 hours")
                }

                FishUI.TabButton {
                    id: weekButton
                    text: qsTr("7 days")
                }
            }
        }

        RoundedItem {
            spacing: 0

            HistoryGraph {
                Layout.fillWidth: true
                height: 240

                points: history.points
                pointStates: history.states
                end: history.lastDataPointTime
                duration: section.plottedDuration
                yUnits: section.historyType === BatteryHistoryModel.RateType ? qsTr("W") : "%"
                yMax: {
                    if (section.historyType === BatteryHistoryModel.RateType) {
                        // ceil to nearest 10
                        var max = Math.floor(history.largestValue)
                        max = max - max % 10 + 10

                        return max;
                    } else {
                        return 100;
                    }
                }
                yStep: section.historyType === BatteryHistoryModel.RateType ? 10 : 25
                decimals: section.historyType === BatteryHistoryModel.RateType ? 1 : 0
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

                FishUI.Label {
                    text: battery.capacity >= 90 ? qsTr("Excellent")
                          : battery.capacity >= 80 ? qsTr("Normal")
                          : qsTr("Poor")
                    color: FishUI.Theme.disabledTextColor
                }
            }

            ListRow {
                interactive: false
                text: qsTr("Maximum capacity")

                FishUI.Label {
                    text: battery.capacity + "%"
                    color: FishUI.Theme.disabledTextColor
                }
            }

            ListRow {
                interactive: false
                visible: battery.lastChargedPercent !== 0
                text: qsTr("Last charged to") + " " + battery.lastChargedPercent + "%"

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
