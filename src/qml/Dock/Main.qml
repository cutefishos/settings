// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cutefish.Settings
import FishUI as FishUI
import "../"

ItemPage {
    headerTitle: qsTr("Dock")

    DockSettings {
        id: dockSettings
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

                SectionLabel {
                    text: qsTr("Position on screen")
                }

                RoundedItem {
                    RowLayout {
                        spacing: Metrics.tileSpacing

                        ImageSelectionItem {
                            source: "qrc:/images/dock_left.svg"
                            text: qsTr("Left")
                            previewWidth: 80
                            previewHeight: 80
                            checked: dockSettings.dockDirection === 0
                            onClicked: dockSettings.setDockDirection(0)
                        }

                        ImageSelectionItem {
                            source: "qrc:/images/dock_bottom.svg"
                            text: qsTr("Bottom")
                            previewWidth: 80
                            previewHeight: 80
                            checked: dockSettings.dockDirection === 1
                            onClicked: dockSettings.setDockDirection(1)
                        }

                        ImageSelectionItem {
                            source: "qrc:/images/dock_right.svg"
                            text: qsTr("Right")
                            previewWidth: 80
                            previewHeight: 80
                            checked: dockSettings.dockDirection === 2
                            onClicked: dockSettings.setDockDirection(2)
                        }
                    }
                }
            }

            ListCard {
                SliderRow {
                    text: qsTr("Size")
                    leftText: qsTr("Small")
                    rightText: qsTr("Huge")
                    // The value is the icon size itself; the dock derives
                    // its paddings from it (54 is the design baseline).
                    from: 32
                    to: 128
                    stepSize: 1
                    value: dockSettings.dockIconSize
                    onMoved: (newValue) => dockSettings.setDockIconSize(Math.round(newValue))
                }

                SliderRow {
                    text: qsTr("Glass effect")
                    leftText: qsTr("More clear")
                    rightText: qsTr("More tinted")
                    from: 0
                    to: 1
                    value: dockSettings.dockGlassTint
                    onMoved: (newValue) => dockSettings.setDockGlassTint(newValue)
                }
            }

            // Visibility
            ColumnLayout {
                Layout.fillWidth: true
                spacing: Metrics.sectionLabelSpacing

                SectionLabel {
                    text: qsTr("Display mode")
                }

                FishUI.SegmentedControl {
                    Layout.fillWidth: true
                    currentIndex: dockSettings.dockVisibility
                    onCurrentIndexChanged: dockSettings.setDockVisibility(currentIndex)

                    FishUI.TabButton {
                        text: qsTr("Always show")
                    }

                    FishUI.TabButton {
                        text: qsTr("Always hide")
                    }

                    FishUI.TabButton {
                        text: qsTr("Smart hide")
                    }
                }
            }

            ListCard {
                NavigationItem {
                    text: qsTr("Hot Corners")
                    onClicked: rootWindow.pushPage("qrc:/qml/HotCorners/Main.qml")
                }
            }

            ListCard {
                ListRow {
                    interactive: false
                    text: qsTr("Minimized windows area")

                    FishUI.Switch {
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                        checked: dockSettings.minimizedArea
                        rightPadding: 0
                        rightInset: 0
                        onToggled: dockSettings.setMinimizedArea(checked)
                    }
                }
            }

            Item {
                Layout.fillHeight: true
            }
        }
    }
}
