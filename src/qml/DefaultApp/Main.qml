/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     Kate Leet <support@cutefishos.com>
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
