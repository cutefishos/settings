// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FishUI as FishUI
import Cutefish.Settings
import "../"

ItemPage {
    headerTitle: qsTr("About")

    About {
        id: about
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent
            spacing: Metrics.sectionSpacing

            // A nested layout never grows past its children's width, so it is centred
            // itself rather than filled.
            ColumnLayout {
                Layout.alignment: Qt.AlignHCenter
                Layout.topMargin: Metrics.l
                spacing: Metrics.s

                Image {
                    Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
                    width: 140
                    height: 72
                    sourceSize: Qt.size(width, height)
                    source: "qrc:/images/logo.svg"
                }

                FishUI.Label {
                    Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
                    text: "<b>Cutefish</b>"
                    font.pointSize: 22
                    color: "#3385FF"
                }

                FishUI.Label {
                    text: qsTr("Running on %1").arg(about.prettyProductName)
                    Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
                    color: FishUI.Theme.disabledTextColor
                }
            }

            ListCard {
                InfoRow {
                    text: qsTr("System Version")
                    value: about.version
                }

                InfoRow {
                    text: qsTr("System Type")
                    value: about.architecture
                }

                InfoRow {
                    text: qsTr("Kernel Version")
                    value: about.kernelVersion
                }

                InfoRow {
                    text: qsTr("Processor")
                    value: about.cpuInfo
                }

                InfoRow {
                    text: qsTr("RAM")
                    value: about.memorySize
                }

                InfoRow {
                    text: qsTr("Internal Storage")
                    value: about.internalStorage
                }
            }
        }
    }
}
