// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

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
                        // Greyed out, with the note below, when no NTP service is installed.
                        enabled: time.canNTP
                        interactive: false
                        text: qsTr("Auto Sync")

                        FishUI.Switch {
                            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                            rightPadding: 0
                            rightInset: 0
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
