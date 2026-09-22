/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     Reion Wong <reionwong@gmail.com>
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

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import FishUI 1.0 as FishUI
import Cutefish.Settings 1.0
import Cutefish.Accounts 1.0
import "../"

ItemPage {
    headerTitle: qsTr("Mouse")

    Mouse {
        id: mouse
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent
            spacing: FishUI.Units.largeSpacing * 2

            RoundedItem {
                enabled: mouse.available
                GridLayout {
                    columns: 2
                    columnSpacing: FishUI.Units.largeSpacing * 1.5
                    rowSpacing: FishUI.Units.largeSpacing * 2

                    FishUI.Label {
                        text: qsTr("Left hand")
                        Layout.fillWidth: true
                    }

                    FishUI.Switch {
                        Layout.fillHeight: true
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                        checked: mouse.leftHanded
                        rightPadding: 0
                        rightInset: 0
                        onToggled: mouse.leftHanded = checked
                    }

                    FishUI.Label {
                        text: qsTr("Natural scrolling")
                    }

                    FishUI.Switch {
                        Layout.fillHeight: true
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                        checked: mouse.naturalScroll
                        onToggled: mouse.naturalScroll = checked
                        rightPadding: 0
                        rightInset: 0
                    }

                    FishUI.Label {
                        text: qsTr("Mouse acceleration")
                    }

                    FishUI.Switch {
                        Layout.fillHeight: true
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                        checked: mouse.acceleration
                        onToggled: mouse.acceleration = checked
                        rightPadding: 0
                        rightInset: 0
                    }

//                    Label {
//                        text: qsTr("Disable touchpad when mouse is connected")
//                    }

//                    Switch {
//                        Layout.fillHeight: true
//                        Layout.alignment: Qt.AlignRight
//                        rightPadding: 0
//                    }
                }

                HorizontalDivider {}

                RowLayout {
                    spacing: FishUI.Units.largeSpacing * 2

                    FishUI.Label {
                        text: qsTr("Pointer speed")
                    }

                    FishUI.Slider {
                        id: accelerationSlider
                        Layout.fillWidth: true
                        rightPadding: FishUI.Units.largeSpacing
                        from: 1
                        to: 11
                        stepSize: 1

                        value: 6 + mouse.pointerAcceleration / 0.2

                        onMoved: {
                            mouse.pointerAcceleration = Math.round(((value - 6) * 0.2) * 10) / 10
                        }
                    }
                }
            }

        }
    }
}
