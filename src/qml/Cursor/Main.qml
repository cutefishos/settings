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
            spacing: Metrics.sectionSpacing

            ListCard {
                enabled: mouse.available

                ListRow {
                    interactive: false
                    text: qsTr("Left hand")

                    FishUI.Switch {
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                        checked: mouse.leftHanded
                        rightPadding: 0
                        rightInset: 0
                        onToggled: mouse.leftHanded = checked
                    }
                }

                ListRow {
                    interactive: false
                    text: qsTr("Natural scrolling")

                    FishUI.Switch {
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                        checked: mouse.naturalScroll
                        rightPadding: 0
                        rightInset: 0
                        onToggled: mouse.naturalScroll = checked
                    }
                }

                ListRow {
                    interactive: false
                    text: qsTr("Mouse acceleration")

                    FishUI.Switch {
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                        checked: mouse.acceleration
                        rightPadding: 0
                        rightInset: 0
                        onToggled: mouse.acceleration = checked
                    }
                }

                ListRow {
                    interactive: false
                    minimumRowHeight: Metrics.controlRowHeight
                    text: qsTr("Pointer speed")

                    RowSlider {
                        id: accelerationSlider
                        // The label fills the rest of the row.
                        trackWidth: Math.round(parent.width / 2)

                        // Ten steps, one per acceleration curve the compositor
                        // has. Speed 0 is the fourth and default; below it a
                        // step is a third of the range, above it a sixth.
                        readonly property int defaultStep: 3

                        from: 0
                        to: 9
                        stepSize: 1
                        snapMode: Slider.SnapAlways

                        value: {
                            const speed = mouse.pointerAcceleration
                            return Math.round(speed < 0 ? defaultStep * (1 + speed)
                                                        : defaultStep + speed * (to - defaultStep))
                        }

                        onMoved: {
                            const step = Math.round(value)
                            mouse.pointerAcceleration = step < defaultStep ? step / defaultStep - 1
                                                                           : (step - defaultStep) / (to - defaultStep)
                        }
                    }
                }
            }
        }
    }
}
