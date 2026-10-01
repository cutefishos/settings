// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.4
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3

import Cutefish.Settings 1.0
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
            spacing: Metrics.sectionLabelSpacing

            SectionLabel {
                text: qsTr("Default application")
            }

            RoundedItem {
                GridLayout {
                    columns: 2
                    columnSpacing: Metrics.formColumnSpacing
                    rowSpacing: Metrics.formRowSpacing

                    FishUI.Label {
                        text: qsTr("Web browser")
                    }

                    FishUI.ComboBox {
                        Layout.fillWidth: true
                    }

                    FishUI.Label {
                        text: qsTr("File manager")
                    }

                    FishUI.ComboBox {
                        Layout.fillWidth: true
                    }

                    FishUI.Label {
                        text: qsTr("Email")
                    }

                    FishUI.ComboBox {
                        Layout.fillWidth: true
                    }

                    FishUI.Label {
                        text: qsTr("Terminal emulator")
                    }

                    FishUI.ComboBox {
                        Layout.fillWidth: true
                    }
                }
            }
        }
    }
}
