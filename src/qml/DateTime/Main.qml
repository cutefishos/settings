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
    headerTitle: qsTr("Date & Time")

    TimeZoneDialog {
        id: timeZoneDialog
    }

    TimeZoneMap {
        id: timeZoneMap
    }

    Time {
        id: time
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent
            spacing: Metrics.sectionSpacing

            ColumnLayout {
                Layout.fillWidth: true
                spacing: Metrics.sectionLabelSpacing

                ListCard {
                    ListRow {
                        interactive: false
                        text: qsTr("Auto Sync")

                        FishUI.Switch {
                            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                            rightPadding: 0
                            rightInset: 0
                            checkable: time.canNTP
                            checked: time.useNtp
                            onCheckedChanged: time.useNtp = checked
                        }
                    }

                    ListRow {
                        interactive: false
                        text: qsTr("24-Hour Time")

                        FishUI.Switch {
                            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                            rightPadding: 0
                            rightInset: 0
                            checked: time.twentyFour
                            onCheckedChanged: time.twentyFour = checked
                        }
                    }

                    NavigationItem {
                        text: qsTr("Time Zone")
                        value: timeZoneMap.currentTimeZone
                        onClicked: timeZoneDialog.show()
                    }
                }

                FishUI.Label {
                    Layout.fillWidth: true
                    leftPadding: Metrics.cardHorizontalPadding
                    rightPadding: Metrics.cardHorizontalPadding
                    text: qsTr("Unable to use Auto Sync. Please ensure your NTP service is installed.")
                    color: FishUI.Theme.disabledTextColor
                    wrapMode: Text.WordWrap
                    visible: !time.canNTP
                }
            }
        }
    }
}
