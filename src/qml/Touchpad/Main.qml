// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FishUI as FishUI
import Cutefish.Settings
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
                    // Pads with physical buttons, or only one click method,
                    // have no choice to make.
                    visible: touchpad.secondaryClickConfigurable
                    interactive: false
                    minimumRowHeight: Metrics.controlRowHeight
                    text: qsTr("Secondary click")

                    RowComboBox {
                        model: [qsTr("Two-finger press"), qsTr("Bottom-right corner press")]
                        currentIndex: touchpad.twoFingerSecondaryClick ? 0 : 1
                        onActivated: touchpad.twoFingerSecondaryClick = currentIndex === 0
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

                    RowSlider {
                        // The label fills the rest of the row.
                        trackWidth: Math.round(parent.width / 2)

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

                ListRow {
                    interactive: false
                    minimumRowHeight: Metrics.controlRowHeight
                    text: qsTr("Scroll speed")

                    RowSlider {
                        // The label fills the rest of the row.
                        trackWidth: Math.round(parent.width / 2)

                        // Ten steps doubling every third, from a quarter to
                        // twice libinput's speed; the default half is step 4.
                        readonly property int defaultStep: 3
                        readonly property real defaultSpeed: 0.5

                        from: 0
                        to: 9
                        stepSize: 1
                        snapMode: Slider.SnapAlways

                        value: Math.round(defaultStep + 3 * Math.log(touchpad.scrollSpeed / defaultSpeed) / Math.LN2)
                        onMoved: touchpad.scrollSpeed = defaultSpeed * Math.pow(2, (Math.round(value) - defaultStep) / 3)
                    }
                }
            }
        }
    }
}
