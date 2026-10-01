// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.4
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3

import Cutefish.Settings 1.0
import FishUI 1.0 as FishUI
import Cutefish.Network 1.0 as NM

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

    FishUI.Label {
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
            spacing: Metrics.sectionSpacing

            // Hotspot
            RoundedItem {
                id: hotspotItem
                visible: handler.hotspotSupported

                RowLayout {
                    spacing: Metrics.rowSpacing

                    FishUI.Label {
                        text: qsTr("Hotspot")
                        color: FishUI.Theme.disabledTextColor
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    FishUI.Switch {
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

                GridLayout {
                    Layout.topMargin: Metrics.cardSpacing
                    columns: 2
                    columnSpacing: Metrics.formColumnSpacing
                    rowSpacing: Metrics.formRowSpacing

                    FishUI.Label {
                        text: qsTr("SSID")
                    }

                    FishUI.TextField {
                        id: ssidName
                        text: configuration.hotspotName
                        // placeholderText: qsTr("SSID")
                        Layout.fillWidth: true
                    }

                    FishUI.Label {
                        text: qsTr("Password")
                    }

                    FishUI.TextField {
                        id: hotspotPassword
                        // placeholderText: qsTr("Password")
                        text: configuration.hotspotPassword
                        Layout.fillWidth: true
                    }
                }

                FishUI.Button {
                    text: qsTr("Save")
                    Layout.topMargin: Metrics.cardSpacing
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

        }
    }
}
