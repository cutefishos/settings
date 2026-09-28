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

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import FishUI 1.0 as FishUI
import Cutefish.Settings 1.0
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
