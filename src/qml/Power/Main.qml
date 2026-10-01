// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.4
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3

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

            ColumnLayout {
                Layout.fillWidth: true
                spacing: Metrics.sectionLabelSpacing
                visible: power.profiles.length > 0

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
                            visible: power.profiles.indexOf("power-saver") >= 0
                            checked: power.profile === "power-saver"
                            onClicked: power.profile = "power-saver"
                        }

                        ImageSelectionItem {
                            source: "qrc:/images/balanced.svg"
                            text: qsTr("Balanced")
                            previewWidth: 80
                            previewHeight: 80
                            visible: power.profiles.indexOf("balanced") >= 0
                            checked: power.profile === "balanced"
                            onClicked: power.profile = "balanced"
                        }

                        ImageSelectionItem {
                            source: "qrc:/images/performance.svg"
                            text: qsTr("Performance")
                            previewWidth: 80
                            previewHeight: 80
                            visible: power.profiles.indexOf("performance") >= 0
                            checked: power.profile === "performance"
                            onClicked: power.profile = "performance"
                        }
                    }
                }

                FishUI.Label {
                    color: FishUI.Theme.disabledTextColor
                    leftPadding: Metrics.cardHorizontalPadding
                    rightPadding: Metrics.cardHorizontalPadding
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    visible: text !== ""
                    text: {
                        if (power.profile === "power-saver")
                            return qsTr("Reduces performance and power consumption.")
                        if (power.profile === "balanced")
                            return qsTr("Standard performance and power consumption.")
                        if (power.profile === "performance")
                            return qsTr("Higher performance, with more power consumption and heat.")
                        return ""
                    }
                }

                FishUI.Label {
                    color: FishUI.Theme.disabledTextColor
                    leftPadding: Metrics.cardHorizontalPadding
                    rightPadding: Metrics.cardHorizontalPadding
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    visible: power.profile === "performance" && power.performanceDegraded !== ""
                    text: {
                        if (power.performanceDegraded.indexOf("lap-detected") >= 0)
                            return qsTr("Performance is limited because the computer is on your lap.")
                        if (power.performanceDegraded.indexOf("high-operating-temperature") >= 0)
                            return qsTr("Performance is limited because the computer is too hot.")
                        return qsTr("Performance is currently limited.")
                    }
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
                    text: _battery.available ? qsTr("Turn off display when inactive on power adapter")
                                             : qsTr("Turn off display when inactive")

                    FishUI.ComboBox {
                        Layout.preferredWidth: 160
                        model: [qsTr("1 Minute"), qsTr("2 Minutes"), qsTr("5 Minutes"),
                                qsTr("10 Minutes"), qsTr("15 Minutes"), qsTr("20 Minutes"),
                                qsTr("30 Minutes"), qsTr("Never")]
                        currentIndex: timeoutToIndex(power.acScreenOff)
                        onActivated: power.acScreenOff = timeoutValues[currentIndex]
                    }
                }

                ListRow {
                    interactive: false
                    text: _battery.available ? qsTr("Prevent automatic sleeping on power adapter when the display is off")
                                             : qsTr("Prevent automatic sleeping when the display is off")

                    FishUI.Switch {
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                        rightPadding: 0
                        rightInset: 0
                        checked: power.acPreventSleep
                        onToggled: power.acPreventSleep = checked
                    }
                }
            }

        }
    }
}
