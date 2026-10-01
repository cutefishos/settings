// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import FishUI 1.0 as FishUI
import Cutefish.Settings 1.0
import "../"

ItemPage {
    headerTitle: qsTr("Default Applications")

    DefaultApplications {
        id: defaultApps
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
                    minimumRowHeight: Metrics.controlRowHeight
                    text: qsTr("Web Browser")
                    enabled: browserComboBox.count !== 0

                    FishUI.ComboBox {
                        id: browserComboBox
                        // The label fills the rest of the row.
                        Layout.preferredWidth: Math.round(parent.width / 2)
                        textRole: "name"
                        iconRole: "icon"
                        model: defaultApps.browserList
                        currentIndex: defaultApps.browserIndex >= 0
                                      ? defaultApps.browserIndex
                                      : (defaultApps.browserList.length > 0 ? 0 : -1)
                        onActivated: defaultApps.setDefaultBrowser(browserComboBox.currentIndex)
                    }
                }

                ListRow {
                    interactive: false
                    minimumRowHeight: Metrics.controlRowHeight
                    text: qsTr("Email Client")
                    enabled: emailComboBox.count !== 0

                    FishUI.ComboBox {
                        id: emailComboBox
                        // The label fills the rest of the row.
                        Layout.preferredWidth: Math.round(parent.width / 2)
                        textRole: "name"
                        iconRole: "icon"
                        model: defaultApps.emailList
                        currentIndex: defaultApps.emailIndex >= 0
                                      ? defaultApps.emailIndex
                                      : (defaultApps.emailList.length > 0 ? 0 : -1)
                        onActivated: defaultApps.setDefaultEMail(emailComboBox.currentIndex)
                    }
                }

                ListRow {
                    interactive: false
                    minimumRowHeight: Metrics.controlRowHeight
                    text: qsTr("Terminal")
                    enabled: terminalComboBox.count !== 0

                    FishUI.ComboBox {
                        id: terminalComboBox
                        // The label fills the rest of the row.
                        Layout.preferredWidth: Math.round(parent.width / 2)
                        textRole: "name"
                        iconRole: "icon"
                        model: defaultApps.terminalList
                        currentIndex: defaultApps.terminalIndex >= 0
                                      ? defaultApps.terminalIndex
                                      : (defaultApps.terminalList.length > 0 ? 0 : -1)
                        onActivated: defaultApps.setDefaultTerminal(terminalComboBox.currentIndex)
                    }
                }
            }
        }
    }
}
