// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import FishUI as FishUI
import Cutefish.Settings
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
                    text: qsTr("Web Browser")
                    enabled: browserComboBox.count !== 0

                    RowComboBox {
                        id: browserComboBox
                        displayText: count !== 0 ? currentText : qsTr("None")
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
                    text: qsTr("Email Client")
                    enabled: emailComboBox.count !== 0

                    RowComboBox {
                        id: emailComboBox
                        displayText: count !== 0 ? currentText : qsTr("None")
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
                    text: qsTr("Terminal")
                    enabled: terminalComboBox.count !== 0

                    RowComboBox {
                        id: terminalComboBox
                        displayText: count !== 0 ? currentText : qsTr("None")
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
