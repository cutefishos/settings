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
            spacing: FishUI.Units.largeSpacing * 2

            ColumnLayout {
                Layout.fillWidth: true
                spacing: FishUI.Units.smallSpacing

                SectionLabel {
                    text: qsTr("Screen")
                    visible: _screenView.count > 0
                }

                RoundedItem {
                    visible: _screenView.count > 0

                    ListView {
                        id: _screenView
                        Layout.fillWidth: true
                        model: screen.outputModel
                        orientation: ListView.Horizontal
                        interactive: false
                        currentIndex: 0
                        highlightMoveDuration: 0
                        clip: true

                        Layout.preferredHeight: currentItem ? currentItem.layout.implicitHeight + FishUI.Units.largeSpacing : 0

                        delegate: Item {
                            id: screenItem
                            height: ListView.view.height
                            width: ListView.view.width

                            property var element: model
                            property var layout: _mainLayout

                            ColumnLayout {
                                id: _mainLayout
                                anchors.fill: parent
                                enabled: screen.available && !screen.busy

                                GridLayout {
                                    Layout.fillWidth: true
                                    columns: 2
                                    columnSpacing: FishUI.Units.largeSpacing * 1.5
                                    rowSpacing: FishUI.Units.largeSpacing * 1.5

                                    Brightness {
                                        id: brightness
                                        display: element.drmConnector
                                    }

                                    Timer {
                                        id: brightnessTimer
                                        interval: 100
                                        onTriggered: brightness.setValue(brightnessSlider.value)
                                    }

                                    Label {
                                        text: qsTr("Brightness")
                                        visible: brightness.available
                                    }

                                    RowLayout {
                                        Layout.fillWidth: true
                                        spacing: FishUI.Units.largeSpacing
                                        visible: brightness.available

                                        Image {
                                            width: 16
                                            height: width
                                            sourceSize.width: width
                                            sourceSize.height: height
                                            Layout.alignment: Qt.AlignVCenter
                                            source: "qrc:/images/" + (FishUI.Theme.darkMode ? "dark" : "light") + "/display-brightness-low-symbolic.svg"
                                        }

                                        Slider {
                                            id: brightnessSlider
                                            Layout.fillWidth: true
                                            Layout.alignment: Qt.AlignVCenter
                                            value: brightness.value
                                            from: 1
                                            to: 100
                                            stepSize: 1
                                            onMoved: brightnessTimer.start()

                                            ToolTip {
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

                                    Label {
                                        text: qsTr("Screen Name")
                                        visible: _screenView.count > 1
                                    }

                                    Label {
                                        text: element.display
                                        color: FishUI.Theme.disabledTextColor
                                        visible: _screenView.count > 1
                                    }

                                    Label {
                                        text: qsTr("Resolution")
                                    }

                                    ComboBox {
                                        Layout.fillWidth: true
                                        model: element.resolutions
                                        leftPadding: FishUI.Units.largeSpacing
                                        rightPadding: FishUI.Units.largeSpacing
                                        topInset: 0
                                        bottomInset: 0
                                        currentIndex: element.resolutionIndex !== undefined ?
                                                          element.resolutionIndex : -1
                                        onActivated: {
                                            element.resolutionIndex = currentIndex
                                        }
                                    }

                                    Label {
                                        text: qsTr("Refresh rate")
                                    }

                                    ComboBox {
                                        id: refreshRate
                                        Layout.fillWidth: true
                                        model: element.refreshRates
                                        leftPadding: FishUI.Units.largeSpacing
                                        rightPadding: FishUI.Units.largeSpacing
                                        topInset: 0
                                        bottomInset: 0
                                        currentIndex: element.refreshRateIndex ?
                                                          element.refreshRateIndex : 0
                                        onActivated: {
                                            element.refreshRateIndex = currentIndex
                                        }
                                    }

                                    Label {
                                        text: qsTr("Scale")
                                    }

                                    FishUI.SegmentedControl {
                                        id: scaleTabbar
                                        Layout.fillWidth: true

                                        TabButton {
                                            text: "100%"
                                            onClicked: element.scale = 1.0
                                        }

                                        TabButton {
                                            text: "125%"
                                            onClicked: element.scale = 1.25
                                        }

                                        TabButton {
                                            text: "150%"
                                            onClicked: element.scale = 1.5
                                        }

                                        TabButton {
                                            text: "175%"
                                            onClicked: element.scale = 1.75
                                        }

                                        TabButton {
                                            text: "200%"
                                            onClicked: element.scale = 2.0
                                        }

                                        currentIndex: {
                                            var index = 0
                                            var scale = element.scale

                                            if (scale <= 1.0)
                                                index = 0
                                            else if (scale <= 1.25)
                                                index = 1
                                            else if (scale <= 1.50)
                                                index = 2
                                            else if (scale <= 1.75)
                                                index = 3
                                            else if (scale <= 2.0)
                                                index = 4

                                            return index
                                        }
                                    }

                                    Label {
                                        text: qsTr("Rotation")
                                    }

                                    Item {
                                        id: rotationItem
                                        Layout.fillWidth: true
                                        height: rotationLayout.implicitHeight

                                        RowLayout {
                                            id: rotationLayout
                                            anchors.fill: parent
                                            spacing: 0

                                            RotationButton {
                                                value: 0
                                            }

                                            Item {
                                                Layout.fillWidth: true
                                            }

                                            RotationButton {
                                                value: 90
                                            }

                                            Item {
                                                Layout.fillWidth: true
                                            }

                                            RotationButton {
                                                value: 180
                                            }

                                            Item {
                                                Layout.fillWidth: true
                                            }

                                            RotationButton {
                                                value: 270
                                            }
                                        }
                                    }

                                    Label {
                                        text: qsTr("Enabled")
                                        visible: enabledBox.visible
                                    }

                                    CheckBox {
                                        id: enabledBox
                                        checked: element.enabled
                                        visible: _screenView.count > 1
                                        onClicked: {
                                            element.enabled = checked
                                        }
                                    }

                                    Label { text: qsTr("Primary"); visible: _screenView.count > 1 }
                                    CheckBox {
                                        checked: element.primary
                                        visible: _screenView.count > 1
                                        enabled: element.enabled
                                        onClicked: if (checked) element.primary = true
                                    }

                                    Label { text: qsTr("Position"); visible: _screenView.count > 1 }
                                    RowLayout {
                                        visible: _screenView.count > 1
                                        SpinBox { from: -32768; to: 32768; value: element.position.x; onValueModified: element.position = Qt.point(value, element.position.y) }
                                        SpinBox { from: -32768; to: 32768; value: element.position.y; onValueModified: element.position = Qt.point(element.position.x, value) }
                                    }

                                    Label { text: qsTr("Mode"); visible: _screenView.count > 1 && (element.capabilities & 4) !== 0 }
                                    ComboBox {
                                        visible: _screenView.count > 1 && (element.capabilities & 4) !== 0
                                        Layout.fillWidth: true
                                        model: element.replicationSourceModel
                                        currentIndex: element.replicationSourceIndex
                                        onActivated: element.replicationSourceIndex = currentIndex
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
                }
            }

            Label {
                Layout.fillWidth: true
                visible: screen.loaded && (!screen.available || screen.error.length > 0)
                text: screen.available ? screen.error : qsTr("Display configuration is unavailable in this session.")
                color: "#d93025"
                wrapMode: Text.Wrap
            }

            RowLayout {
                Layout.fillWidth: true
                Button { text: qsTr("Identify"); enabled: screen.available; onClicked: screen.identify() }
                Item { Layout.fillWidth: true }
            }

            Item {
                Layout.fillHeight: true
            }
        }
    }
}
