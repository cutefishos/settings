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
                    text: qsTr("Glass Effect")
                    leftText: qsTr("More Clear")
                    rightText: qsTr("More Tinted")
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

            Item {
                Layout.fillHeight: true
            }
        }
    }
}
