// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FishUI as FishUI
import Cutefish.Settings
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
