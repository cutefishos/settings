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
            spacing: FishUI.Units.largeSpacing * 2

            ColumnLayout {
                Layout.fillWidth: true
                spacing: FishUI.Units.smallSpacing

                SectionLabel {
                    text: qsTr("Position on screen")
                }

                RoundedItem {
                    RowLayout {
                        spacing: FishUI.Units.largeSpacing * 2

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

            // Dock Size
            ColumnLayout {
                Layout.fillWidth: true
                spacing: FishUI.Units.smallSpacing

                SectionLabel {
                    text: qsTr("Size")
                }

                FishUI.SegmentedControl {
                    id: dockSizeTabbar
                    Layout.fillWidth: true

                    FishUI.TabButton {
                        text: qsTr("Small")
                    }

                    FishUI.TabButton {
                        text: qsTr("Medium")
                    }

                    FishUI.TabButton {
                        text: qsTr("Large")
                    }

                    FishUI.TabButton {
                        text: qsTr("Huge")
                    }

                    currentIndex: {
                        var index = 0

                        if (dockSettings.dockIconSize <= 40)
                            index = 0
                        else if (dockSettings.dockIconSize <= 54)
                            index = 1
                        else if (dockSettings.dockIconSize <= 68)
                            index = 2
                        else
                            index = 3

                        return index
                    }

                    onCurrentIndexChanged: {
                        var iconSize = 0

                        switch (currentIndex) {
                        // The value is the icon size itself; the dock derives
                        // its paddings from it (54 is the design baseline).
                        case 0:
                            iconSize = 40
                            break;
                        case 1:
                            iconSize = 54
                            break;
                        case 2:
                            iconSize = 68
                            break;
                        case 3:
                            iconSize = 82
                            break;
                        }

                        dockSettings.setDockIconSize(iconSize)
                    }
                }
            }

            // Visibility
            ColumnLayout {
                Layout.fillWidth: true
                spacing: FishUI.Units.smallSpacing

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

            // Glass effect
            ColumnLayout {
                Layout.fillWidth: true
                spacing: FishUI.Units.smallSpacing

                SectionLabel {
                    text: qsTr("Glass Effect")
                }

                RoundedItem {
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: FishUI.Units.largeSpacing

                        FishUI.Label {
                            text: qsTr("More Clear")
                        }

                        FishUI.Slider {
                            Layout.fillWidth: true
                            from: 0
                            to: 1
                            value: dockSettings.dockGlassTint
                            onMoved: dockSettings.setDockGlassTint(value)
                        }

                        FishUI.Label {
                            text: qsTr("More Tinted")
                        }
                    }
                }
            }

            Item {
                Layout.fillHeight: true
            }
        }
    }
}
