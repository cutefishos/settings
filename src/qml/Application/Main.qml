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

import FishUI 1.0 as FishUI
import "../"

ItemPage {
    id: control
    headerTitle: qsTr("Application")

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent
            spacing: FishUI.Units.smallSpacing

            Label {
                text: qsTr("Default application")
                leftPadding: FishUI.Units.largeSpacing
            }

            RoundedItem {
                GridLayout {
                    columns: 2
                    columnSpacing: FishUI.Units.largeSpacing * 2

                    Label {
                        text: qsTr("Web browser")
                    }

                    ComboBox {
                        Layout.fillWidth: true
                    }

                    Label {
                        text: qsTr("File manager")
                    }

                    ComboBox {
                        Layout.fillWidth: true
                    }

                    Label {
                        text: qsTr("Email")
                    }

                    ComboBox {
                        Layout.fillWidth: true
                    }

                    Label {
                        text: qsTr("Terminal emulator")
                    }

                    ComboBox {
                        Layout.fillWidth: true
                    }
                }
            }
        }
    }
}
