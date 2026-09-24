/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     Reion Wong <reion@cutefishos.com>
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
import "../"

ItemPage {
    headerTitle: qsTr("Touchpad")

    Touchpad {
        id: touchpad
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent
            spacing: Metrics.sectionSpacing

            ListCard {
                enabled: touchpad.available

                ListRow {
                    interactive: false
                    text: qsTr("Enable")

                    FishUI.Switch {
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                        checked: touchpad.enabled
                        rightPadding: 0
                        rightInset: 0
                        onToggled: touchpad.enabled = checked
                    }
                }

                ListRow {
                    enabled: touchpad.enabled
                    interactive: false
                    text: qsTr("Disable when a mouse is connected")

                    FishUI.Switch {
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                        checked: touchpad.disableWithMouse
                        rightPadding: 0
                        rightInset: 0
                        onToggled: touchpad.disableWithMouse = checked
                    }
                }

                ListRow {
                    enabled: touchpad.enabled
                    interactive: false
                    text: qsTr("Disable while typing")

                    FishUI.Switch {
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                        checked: touchpad.disableWhileTyping
                        rightPadding: 0
                        rightInset: 0
                        onToggled: touchpad.disableWhileTyping = checked
                    }
                }
            }

            ListCard {
                enabled: touchpad.available && touchpad.enabled

                ListRow {
                    interactive: false
                    text: qsTr("Tap to click")

                    FishUI.Switch {
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                        checked: touchpad.tapToClick
                        rightPadding: 0
                        rightInset: 0
                        onToggled: touchpad.tapToClick = checked
                    }
                }

                ListRow {
                    // Tap and drag starts with a tap, so it needs tapping on.
                    enabled: touchpad.tapToClick
                    interactive: false
                    text: qsTr("Tap and drag")

                    FishUI.Switch {
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                        checked: touchpad.tapDragging
                        rightPadding: 0
                        rightInset: 0
                        onToggled: touchpad.tapDragging = checked
                    }
                }

                ListRow {
                    interactive: false
                    text: qsTr("Natural scrolling")

                    FishUI.Switch {
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                        checked: touchpad.naturalScroll
                        rightPadding: 0
                        rightInset: 0
                        onToggled: touchpad.naturalScroll = checked
                    }
                }

                ListRow {
                    interactive: false
                    minimumRowHeight: Metrics.controlRowHeight
                    text: qsTr("Pointer speed")

                    FishUI.Slider {
                        // The label fills the rest of the row.
                        Layout.preferredWidth: Math.round(parent.width / 2)

                        // libinput's speed runs from -1 to 1; eleven steps
                        // put its default in the middle.
                        from: 0
                        to: 10
                        stepSize: 1
                        snapMode: Slider.SnapAlways

                        value: Math.round((touchpad.pointerAcceleration + 1) * to / 2)
                        onMoved: touchpad.pointerAcceleration = Math.round(value) * 2 / to - 1
                    }
                }
            }
        }
    }
}
