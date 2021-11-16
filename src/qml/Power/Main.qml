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
import QtGraphicalEffects 1.0

import Cutefish.Settings 1.0
import FishUI 1.0 as FishUI
import "../"

ItemPage {
    id: control
    headerTitle: qsTr("Power")

    PowerManager {
        id: power
    }

    Battery {
        id: battery
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent
            spacing: FishUI.Units.largeSpacing

            RoundedItem {
                Label {
                    text: qsTr("Mode")
                    color: FishUI.Theme.disabledTextColor
                }

                RowLayout {
                    spacing: FishUI.Units.largeSpacing * 2

                    IconCheckBox {
                        source: "qrc:/images/powersave.svg"
                        text: qsTr("Power Save")
                        checked: power.mode === 0
                        onClicked: power.mode = 0
                    }

//                    IconCheckBox {
//                        source: "qrc:/images/balance.svg"
//                        text: qsTr("Balance")
//                        checked: false
//                    }

                    IconCheckBox {
                        source: "qrc:/images/performance.svg"
                        text: qsTr("Performance")
                        checked: power.mode === 1
                        onClicked: power.mode = 1
                    }
                }
            }

            Label {
                color: FishUI.Theme.disabledTextColor
                leftPadding: FishUI.Units.largeSpacing * 2
                rightPadding: FishUI.Units.largeSpacing
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: qsTr("Performance mode: CPU and GPU frequencies will be increased, while power consumption and heat generation will be increased.")
            }

            RoundedItem {
                Layout.topMargin: FishUI.Units.largeSpacing

                Label {
                    text: qsTr("Plugged In")
                    color: FishUI.Theme.disabledTextColor
                }

                GridLayout {
                    columns: 2

                    Label {
                        text: qsTr("Turn off screen")
                        Layout.fillWidth: true
                    }

                    ComboBox {
                        model: ListModel {
                            ListElement { text: qsTr("2 Minutes") }
                            ListElement { text: qsTr("5 Minutes") }
                            ListElement { text: qsTr("10 Minutes") }
                            ListElement { text: qsTr("15 Minutes") }
                            ListElement { text: qsTr("30 Minutes") }
                            ListElement { text: qsTr("Never") }
                        }
                    }

                    Label {
                        text: qsTr("Hibernate")
                        Layout.fillWidth: true
                    }

                    ComboBox {
                        model: ListModel {
                            ListElement { text: qsTr("2 Minutes") }
                            ListElement { text: qsTr("5 Minutes") }
                            ListElement { text: qsTr("10 Minutes") }
                            ListElement { text: qsTr("15 Minutes") }
                            ListElement { text: qsTr("30 Minutes") }
                            ListElement { text: qsTr("Never") }
                        }
                    }
                }
            }

            RoundedItem {
                visible: battery.available
                Layout.topMargin: FishUI.Units.largeSpacing

                Label {
                    text: qsTr("On Battery")
                    color: FishUI.Theme.disabledTextColor
                }

                GridLayout {
                    columns: 2

                    Label {
                        text: qsTr("Turn off screen")
                        Layout.fillWidth: true
                    }

                    ComboBox {
                        model: ListModel {
                            ListElement { text: qsTr("2 Minutes") }
                            ListElement { text: qsTr("5 Minutes") }
                            ListElement { text: qsTr("10 Minutes") }
                            ListElement { text: qsTr("15 Minutes") }
                            ListElement { text: qsTr("30 Minutes") }
                            ListElement { text: qsTr("Never") }
                        }
                    }

                    Label {
                        text: qsTr("Hibernate")
                        Layout.fillWidth: true
                    }

                    ComboBox {
                        model: ListModel {
                            ListElement { text: qsTr("2 Minutes") }
                            ListElement { text: qsTr("5 Minutes") }
                            ListElement { text: qsTr("10 Minutes") }
                            ListElement { text: qsTr("15 Minutes") }
                            ListElement { text: qsTr("30 Minutes") }
                            ListElement { text: qsTr("Never") }
                        }
                    }
                }
            }

            Item {
                height: FishUI.Units.largeSpacing * 2
            }
        }
    }
}
