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
import Cutefish.Screen 1.0 as CS
import FishUI 1.0 as FishUI
import "../"

ItemPage {
    headerTitle: qsTr("Display")

    CS.Screen {
        id: screen
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

                                    FishUI.Slider {
                                        id: brightnessSlider
                                        // The label fills the rest of the row.
                                        Layout.preferredWidth: Math.round(parent.width / 2)
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
                                    minimumRowHeight: Metrics.controlRowHeight
                                    text: qsTr("Resolution")

                                    FishUI.ComboBox {
                                        Layout.preferredWidth: Math.round(parent.width / 2)
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
                                    minimumRowHeight: Metrics.controlRowHeight
                                    text: qsTr("Refresh rate")

                                    FishUI.ComboBox {
                                        Layout.preferredWidth: Math.round(parent.width / 2)
                                        model: element.refreshRates
                                        currentIndex: element.refreshRateIndex ?
                                                          element.refreshRateIndex : 0
                                        onActivated: element.refreshRateIndex = currentIndex
                                    }
                                }

                                ListRow {
                                    interactive: false
                                    minimumRowHeight: Metrics.controlRowHeight
                                    text: qsTr("Scale")

                                    FishUI.ComboBox {
                                        readonly property var scales: element.supportedScales

                                        Layout.preferredWidth: Math.round(parent.width / 2)
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
                                    minimumRowHeight: Metrics.controlRowHeight
                                    text: qsTr("Rotation")

                                    FishUI.ComboBox {
                                        // Paired with the labels as the old rotation pictures were.
                                        readonly property var rotations: [CS.Output.None, CS.Output.Left,
                                                                          CS.Output.Inverted, CS.Output.Right]

                                        Layout.preferredWidth: Math.round(parent.width / 2)
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
                                    minimumRowHeight: Metrics.controlRowHeight
                                    visible: (element.capabilities & 4) !== 0
                                    text: qsTr("Mode")

                                    FishUI.ComboBox {
                                        Layout.preferredWidth: Math.round(parent.width / 2)
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

            StandardButton {
                Layout.fillWidth: true
                visible: _screenView.count > 1
                text: qsTr("Identify Displays")
                enabled: screen.available
                onClicked: screen.identify()
            }
        }
    }
}
