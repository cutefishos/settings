// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cutefish.Settings
import Cutefish.Screen as CS
import FishUI as FishUI
import "../"

ItemPage {
    headerTitle: qsTr("Display")

    // A time of day, in half-hour steps; a time set some other way shows as the nearest.
    component TimeComboBox: RowComboBox {
        // Minutes after midnight.
        property int minutes
        signal picked(int minutes)
        readonly property var steps: Array.from({length: 48}, (_, index) => index * 30)

        model: steps.map(step => new Date(2000, 0, 1, Math.floor(step / 60), step % 60)
                                 .toLocaleTimeString(Qt.locale(), Locale.ShortFormat))
        currentIndex: Math.round(minutes / 30) % steps.length
        onActivated: picked(steps[currentIndex])
    }

    CS.Screen {
        id: screen
    }

    CS.NightLight {
        id: nightLight
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
                visible: _screenView.count > 0
                spacing: Metrics.sectionLabelSpacing

                // A single screen is named; several are picked between here.
                SectionLabel {
                    visible: _screenView.count === 1
                    text: _screenView.currentItem && _screenView.currentItem.element.display
                          ? _screenView.currentItem.element.display : qsTr("Screen")
                }

                FishUI.SegmentedControl {
                    id: _screenTabs
                    Layout.fillWidth: true
                    visible: _screenView.count > 1

                    Repeater {
                        model: screen.outputModel

                        FishUI.TabButton {
                            text: model.display ? model.display : qsTr("Screen")
                        }
                    }
                }

                // One page per screen, flipped with the tabs above.
                ListView {
                    id: _screenView
                    Layout.fillWidth: true
                    Layout.preferredHeight: currentItem ? currentItem.layout.implicitHeight : 0
                    model: screen.outputModel
                    orientation: ListView.Horizontal
                    interactive: false
                    currentIndex: Math.max(0, _screenTabs.currentIndex)
                    highlightMoveDuration: 0
                    clip: true

                    delegate: Item {
                        id: screenItem
                        height: ListView.view.height
                        width: ListView.view.width

                        property var element: model
                        property var layout: _mainLayout

                        ColumnLayout {
                            id: _mainLayout
                            anchors.left: parent.left
                            anchors.right: parent.right
                            enabled: screen.available && !screen.busy
                            spacing: Metrics.cardSpacing

                            Brightness {
                                id: brightness
                                display: element.drmConnector
                            }

                            Timer {
                                id: brightnessTimer
                                interval: 100
                                onTriggered: brightness.setValue(brightnessSlider.value)
                            }

                            ListCard {
                                ListRow {
                                    interactive: false
                                    visible: brightness.available
                                    minimumRowHeight: Metrics.controlRowHeight
                                    text: qsTr("Brightness")

                                    RowSlider {
                                        id: brightnessSlider
                                        // The label fills the rest of the row.
                                        trackWidth: Math.round(parent.width / 2)
                                        value: brightness.value
                                        from: 1
                                        to: 100
                                        stepSize: 1
                                        onMoved: brightnessTimer.start()

                                        FishUI.ToolTip {
                                            parent: brightnessSlider.handle
                                            visible: brightnessSlider.pressed
                                            text: brightnessSlider.value.toFixed(0)
                                        }
                                    }
                                }

                                ListRow {
                                    interactive: false
                                    text: qsTr("Resolution")

                                    RowComboBox {
                                        // "Recommended" is only a hint while choosing.
                                        model: element.resolutions.map(function (text, index) {
                                            return index === element.recommendedResolutionIndex
                                                    ? qsTr("%1 (Recommended)").arg(text) : text
                                        })
                                        displayText: currentIndex >= 0 ?
                                                         element.resolutions[currentIndex] : ""
                                        currentIndex: element.resolutionIndex !== undefined ?
                                                          element.resolutionIndex : -1
                                        onActivated: element.resolutionIndex = currentIndex
                                    }
                                }

                                ListRow {
                                    interactive: false
                                    text: qsTr("Refresh rate")

                                    RowComboBox {
                                        model: element.refreshRates
                                        currentIndex: element.refreshRateIndex ?
                                                          element.refreshRateIndex : 0
                                        onActivated: element.refreshRateIndex = currentIndex
                                    }
                                }

                                ListRow {
                                    interactive: false
                                    text: qsTr("Scale")

                                    RowComboBox {
                                        readonly property var scales: element.supportedScales

                                        model: scales.map(function (scale) {
                                            var text = Math.round(scale * 100) + "%"
                                            return Math.abs(scale - element.recommendedScale) < 0.001
                                                    ? qsTr("%1 (Recommended)").arg(text) : text
                                        })
                                        displayText: currentIndex < scales.length
                                                     ? Math.round(scales[currentIndex] * 100) + "%" : ""
                                        // The nearest step, for a scale set some other way.
                                        currentIndex: {
                                            var best = 0
                                            for (var i = 1; i < scales.length; ++i) {
                                                if (Math.abs(scales[i] - element.scale)
                                                        < Math.abs(scales[best] - element.scale))
                                                    best = i
                                            }
                                            return best
                                        }
                                        onActivated: element.scale = scales[currentIndex]
                                    }
                                }

                                ListRow {
                                    interactive: false
                                    text: qsTr("Rotation")

                                    RowComboBox {
                                        // Paired with the labels as the old rotation pictures were.
                                        readonly property var rotations: [CS.Output.None, CS.Output.Left,
                                                                          CS.Output.Inverted, CS.Output.Right]

                                        model: [qsTr("Normal"), "90°", "180°", "270°"]
                                        currentIndex: Math.max(0, rotations.indexOf(element.rotation))
                                        onActivated: element.rotation = rotations[currentIndex]
                                    }
                                }
                            }

                            // How this screen sits among the others.
                            ListCard {
                                visible: _screenView.count > 1

                                ListRow {
                                    interactive: false
                                    text: qsTr("Enabled")

                                    FishUI.Switch {
                                        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                                        rightPadding: 0
                                        rightInset: 0
                                        checked: element.enabled
                                        onToggled: element.enabled = checked
                                    }
                                }

                                ListRow {
                                    interactive: false
                                    text: qsTr("Primary")

                                    // A screen stops being primary only when another one
                                    // takes over, so the switch turns on and never off.
                                    FishUI.Switch {
                                        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                                        rightPadding: 0
                                        rightInset: 0
                                        enabled: element.enabled && !element.primary
                                        checked: element.primary
                                        onToggled: element.primary = true
                                    }
                                }

                                ListRow {
                                    interactive: false
                                    visible: (element.capabilities & 4) !== 0
                                    text: qsTr("Mode")

                                    RowComboBox {
                                        model: element.replicationSourceModel
                                        currentIndex: element.replicationSourceIndex
                                        onActivated: element.replicationSourceIndex = currentIndex
                                    }
                                }

                                ListRow {
                                    interactive: false
                                    minimumRowHeight: Metrics.controlRowHeight
                                    text: qsTr("Position")

                                    RowLayout {
                                        Layout.preferredWidth: Math.round(parent.width / 2)
                                        spacing: Metrics.controlSpacing

                                        // Typing leaves the text binding in place, so a position
                                        // changed elsewhere still shows up here.
                                        FishUI.TextField {
                                            Layout.fillWidth: true
                                            Layout.preferredWidth: 1
                                            text: element.position.x
                                            validator: IntValidator { bottom: -32768; top: 32768 }
                                            Accessible.name: qsTr("Horizontal position")
                                            onEditingFinished: element.position = Qt.point(parseInt(text), element.position.y)
                                        }

                                        FishUI.TextField {
                                            Layout.fillWidth: true
                                            Layout.preferredWidth: 1
                                            text: element.position.y
                                            validator: IntValidator { bottom: -32768; top: 32768 }
                                            Accessible.name: qsTr("Vertical position")
                                            onEditingFinished: element.position = Qt.point(element.position.x, parseInt(text))
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            RoundedItem {
                visible: screen.loaded && (!screen.available || screen.error.length > 0)

                FishUI.Label {
                    Layout.fillWidth: true
                    text: screen.available ? screen.error : qsTr("Display configuration is unavailable in this session.")
                    textFormat: Text.PlainText
                    wrapMode: Text.Wrap
                }
            }

            ListCard {
                visible: nightLight.available
                enabled: nightLight.loaded

                ListRow {
                    interactive: false
                    text: qsTr("Night Light")

                    FishUI.Switch {
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                        rightPadding: 0
                        rightInset: 0
                        checked: nightLight.enabled
                        onToggled: nightLight.enabled = checked
                    }
                }

                Timer {
                    id: temperatureTimer
                    property real pending
                    interval: 100
                    onTriggered: nightLight.temperature = pending
                }

                SliderRow {
                    text: qsTr("Color temperature")
                    leftText: qsTr("Less warm")
                    rightText: qsTr("More warm")
                    from: 0
                    to: 1
                    value: nightLight.temperature
                    onMoved: (newValue) => {
                        temperatureTimer.pending = newValue
                        temperatureTimer.start()
                    }
                    onPressedChanged: {
                        // The last move goes out before the preview ends, or it would only be saved.
                        if (!pressed && temperatureTimer.running) {
                            temperatureTimer.stop()
                            nightLight.temperature = temperatureTimer.pending
                        }
                        nightLight.preview = pressed
                    }
                }

                ListRow {
                    interactive: false
                    text: qsTr("Schedule")

                    RowComboBox {
                        model: [qsTr("Never"), qsTr("Sunset to sunrise"), qsTr("Custom")]
                        currentIndex: nightLight.schedule
                        onActivated: nightLight.schedule = currentIndex
                    }
                }

                ListRow {
                    interactive: false
                    visible: nightLight.schedule === CS.NightLight.Custom
                    text: qsTr("From")

                    TimeComboBox {
                        minutes: nightLight.customStart
                        onPicked: (minutes) => nightLight.customStart = minutes
                    }
                }

                ListRow {
                    interactive: false
                    visible: nightLight.schedule === CS.NightLight.Custom
                    text: qsTr("To")

                    TimeComboBox {
                        minutes: nightLight.customEnd
                        onPicked: (minutes) => nightLight.customEnd = minutes
                    }
                }
            }
        }
    }
}
