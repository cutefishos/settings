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

            // Always there: with no screens to list, the error below is all
            // it shows.
            ColumnLayout {
                Layout.fillWidth: true
                spacing: Metrics.sectionLabelSpacing

                // One page per screen, flipped with the indicator below.
                ListView {
                    id: _screenView
                    Layout.fillWidth: true
                    visible: count > 0
                    Layout.preferredHeight: currentItem ? currentItem.layout.implicitHeight : 0
                    model: screen.outputModel
                    orientation: ListView.Horizontal
                    interactive: false
                    currentIndex: 0
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
                            spacing: Metrics.sectionSpacing

                            Brightness {
                                id: brightness
                                display: element.drmConnector
                            }

                            Timer {
                                id: brightnessTimer
                                interval: 100
                                onTriggered: brightness.setValue(brightnessSlider.value)
                            }

                            // The name heads both cards, so it keeps the same distance
                            // from them as they keep from each other.
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: Metrics.cardSpacing

                                SectionLabel {
                                    text: element.display ? element.display : qsTr("Screen")
                                }

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: Metrics.cardSpacing

                                    ListCard {
                                        visible: brightness.available

                                        ListRow {
                                            interactive: false
                                            minimumRowHeight: Metrics.controlRowHeight
                                            text: qsTr("Brightness")

                                            RowLayout {
                                                // The label fills the rest of the row.
                                                Layout.preferredWidth: Math.round(parent.width / 2)
                                                spacing: Metrics.controlSpacing

                                                Image {
                                                    width: 16
                                                    height: width
                                                    sourceSize.width: width
                                                    sourceSize.height: height
                                                    Layout.alignment: Qt.AlignVCenter
                                                    source: "qrc:/images/" + (FishUI.Theme.darkMode ? "dark" : "light") + "/display-brightness-low-symbolic.svg"
                                                }

                                                FishUI.Slider {
                                                    id: brightnessSlider
                                                    Layout.fillWidth: true
                                                    Layout.alignment: Qt.AlignVCenter
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

                                                Image {
                                                    width: 16
                                                    height: width
                                                    sourceSize.width: width
                                                    sourceSize.height: height
                                                    Layout.alignment: Qt.AlignVCenter
                                                    source: "qrc:/images/" + (FishUI.Theme.darkMode ? "dark" : "light") + "/display-brightness-symbolic.svg"
                                                }
                                            }
                                        }
                                    }

                                    ListCard {
                                        separatorsVisible: false

                                        ListRow {
                                            interactive: false
                                            minimumRowHeight: Metrics.controlRowHeight
                                            text: qsTr("Resolution")

                                            FishUI.ComboBox {
                                                Layout.preferredWidth: Math.round(parent.width / 2)
                                                model: element.resolutions
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

                                        ListRow {
                                            interactive: false
                                            visible: _screenView.count > 1
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
                                            visible: _screenView.count > 1
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
                                            visible: _screenView.count > 1
                                            text: qsTr("Position")

                                            RowLayout {
                                                spacing: Metrics.controlSpacing

                                                SpinBox {
                                                    from: -32768
                                                    to: 32768
                                                    value: element.position.x
                                                    onValueModified: element.position = Qt.point(value, element.position.y)
                                                }

                                                SpinBox {
                                                    from: -32768
                                                    to: 32768
                                                    value: element.position.y
                                                    onValueModified: element.position = Qt.point(element.position.x, value)
                                                }
                                            }
                                        }

                                        ListRow {
                                            interactive: false
                                            minimumRowHeight: Metrics.controlRowHeight
                                            visible: _screenView.count > 1 && (element.capabilities & 4) !== 0
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
                                            text: qsTr("Scale")

                                            // As tall as the combo boxes above it, not the bar's
                                            // default, which stands out in a row.
                                            FishUI.SegmentedControl {
                                                id: scaleBar

                                                readonly property int buttonHeight: 31

                                                Layout.preferredWidth: Math.round(parent.width / 2)

                                                FishUI.TabButton {
                                                    implicitHeight: scaleBar.buttonHeight
                                                    text: "100%"
                                                    onClicked: element.scale = 1.0
                                                }

                                                FishUI.TabButton {
                                                    implicitHeight: scaleBar.buttonHeight
                                                    text: "125%"
                                                    onClicked: element.scale = 1.25
                                                }

                                                FishUI.TabButton {
                                                    implicitHeight: scaleBar.buttonHeight
                                                    text: "150%"
                                                    onClicked: element.scale = 1.5
                                                }

                                                FishUI.TabButton {
                                                    implicitHeight: scaleBar.buttonHeight
                                                    text: "175%"
                                                    onClicked: element.scale = 1.75
                                                }

                                                FishUI.TabButton {
                                                    implicitHeight: scaleBar.buttonHeight
                                                    text: "200%"
                                                    onClicked: element.scale = 2.0
                                                }

                                                currentIndex: {
                                                    var scale = element.scale

                                                    if (scale <= 1.0)
                                                        return 0
                                                    else if (scale <= 1.25)
                                                        return 1
                                                    else if (scale <= 1.50)
                                                        return 2
                                                    else if (scale <= 1.75)
                                                        return 3
                                                    return 4
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                PageIndicator {
                    id: screenPageIndicator
                    Layout.alignment: Qt.AlignHCenter
                    count: _screenView.count
                    currentIndex: _screenView.currentIndex
                    onCurrentIndexChanged: _screenView.currentIndex = currentIndex
                    interactive: true
                    visible: count > 1
                }

                FishUI.Label {
                    Layout.fillWidth: true
                    leftPadding: Metrics.cardHorizontalPadding
                    rightPadding: Metrics.cardHorizontalPadding
                    visible: screen.loaded && (!screen.available || screen.error.length > 0)
                    text: screen.available ? screen.error : qsTr("Display configuration is unavailable in this session.")
                    color: "#d93025"
                    wrapMode: Text.Wrap
                }
            }

            StandardButton {
                Layout.fillWidth: true
                text: qsTr("Identify Displays")
                enabled: screen.available
                onClicked: screen.identify()
            }
        }
    }
}
