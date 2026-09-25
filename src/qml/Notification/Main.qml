/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     Kate Leet <kateleet@cutefishos.com>
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
    headerTitle: qsTr("Notifications")

    Notifications {
        id: notifications
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent
            spacing: Metrics.sectionSpacing

            ListCard {
                ListRow {
                    interactive: false
                    text: qsTr("Do Not Disturb")

                    FishUI.Switch {
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                        rightPadding: 0
                        rightInset: 0
                        checked: notifications.doNotDisturb
                        onClicked: notifications.doNotDisturb = checked
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: Metrics.sectionLabelSpacing

                SectionLabel {
                    text: qsTr("Applications")
                }

                RoundedItem {
                    visible: notifications.applications.length === 0

                    FishUI.Label {
                        Layout.fillWidth: true
                        text: qsTr("Applications appear here after they send a notification.")
                        color: FishUI.Theme.disabledTextColor
                        textFormat: Text.PlainText
                        wrapMode: Text.WordWrap
                    }
                }

                ListCard {
                    visible: notifications.applications.length > 0

                    Repeater {
                        model: notifications.applications

                        delegate: ListRow {
                            interactive: false
                            text: modelData.name

                            leading: FishUI.IconItem {
                                width: 24
                                height: 24
                                source: modelData.icon ? modelData.icon : "application-x-executable"
                                smooth: true
                            }

                            // Indexes follow Notifications.ApplicationMode.
                            FishUI.ComboBox {
                                Layout.preferredWidth: 200
                                model: [qsTr("Banners"), qsTr("Notification Center only"), qsTr("Off")]
                                currentIndex: modelData.mode
                                Accessible.name: qsTr("Notifications from %1").arg(modelData.name)
                                onActivated: notifications.setApplicationMode(modelData.key, currentIndex)
                            }
                        }
                    }
                }

                FishUI.Label {
                    Layout.fillWidth: true
                    visible: notifications.applications.length > 0
                    text: qsTr("Notifications that skip the banner still go to the Notification Center. Turned off, they are dropped.")
                    color: FishUI.Theme.disabledTextColor
                    textFormat: Text.PlainText
                    wrapMode: Text.WordWrap
                    leftPadding: Metrics.cardHorizontalPadding
                    rightPadding: Metrics.cardHorizontalPadding
                }
            }
        }
    }
}
