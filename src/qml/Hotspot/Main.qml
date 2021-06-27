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

import QtQuick 2.4
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0

import FishUI 1.0 as FishUI
import Cutefish.NetworkManagement 1.0 as NM

import "../"

ItemPage {
    id: control
    headerTitle: qsTr("Hotspot")

    property var itemHeight: 45
    property var settingsMap: ({})

    NM.Handler {
        id: handler
    }

    NM.Configuration {
        id: configuration
    }

    Label {
        anchors.centerIn: parent
        text: qsTr("Not supported")
        visible: !handler.hotspotSupported
        font.pointSize: 15
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: mainLayout.implicitHeight

        ColumnLayout {
            id: mainLayout
            anchors.fill: parent
            anchors.bottomMargin: FishUI.Units.largeSpacing
            spacing: FishUI.Units.largeSpacing * 2

            // Hotspot
            RoundedItem {
                id: hotspotItem
                visible: handler.hotspotSupported

                RowLayout {
                    Label {
                        text: qsTr("Hotspot")
                        color: FishUI.Theme.disabledTextColor
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Switch {
                        id: _switch
                        Layout.fillHeight: true
                        rightPadding: 0

                        onToggled: {
                            if (checked) {
                                handler.createHotspot()
                            } else {
                                handler.stopHotspot()
                            }
                        }
                    }
                }

                Item {
                    height: FishUI.Units.largeSpacing
                }

                GridLayout {
                    columns: 2
                    columnSpacing: FishUI.Units.largeSpacing * 2
                    rowSpacing: FishUI.Units.largeSpacing

                    Label {
                        text: qsTr("SSID")
                    }

                    TextField {
                        id: ssidName
                        text: configuration.hotspotName
                        // placeholderText: qsTr("SSID")
                        Layout.fillWidth: true
                    }

                    Label {
                        text: qsTr("Password")
                    }

                    TextField {
                        id: hotspotPassword
                        // placeholderText: qsTr("Password")
                        text: configuration.hotspotPassword
                        Layout.fillWidth: true
                    }
                }

                Item {
                    height: FishUI.Units.largeSpacing
                }

                Button {
                    text: qsTr("Save")
                    Layout.alignment: Qt.AlignRight
                    onClicked: {
                        configuration.hotspotName = ssidName.text
                        configuration.hotspotPassword = hotspotPassword.text

                        if (_switch.checked) {
                            handler.stopHotspot()
                            handler.createHotspot()
                        }
                    }
                }
            }

            Item {
                height: FishUI.Units.largeSpacing
            }
        }
    }
}
