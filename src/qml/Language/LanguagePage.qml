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

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import FishUI 1.0 as FishUI
import Cutefish.Settings 1.0 as Settings
import "../"

ItemPage {
    id: page
    headerTitle: qsTr("Language")

    Settings.Language {
        id: languageSettings
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent
            spacing: FishUI.Units.largeSpacing * 2

            ColumnLayout {
                Layout.fillWidth: true
                spacing: FishUI.Units.smallSpacing

                SectionLabel {
                    text: qsTr("Preferred languages")
                }

                RoundedItem {
                    layout.anchors.topMargin: 4
                    layout.anchors.bottomMargin: 4

                    PreferredLanguageList {
                        Layout.fillWidth: true
                        language: languageSettings
                    }
                }
            }

            StandardButton {
                text: qsTr("Add Language…")
                Layout.fillWidth: true
                enabled: !languageSettings.saving
                onClicked: {
                    if (dialogLoader.active)
                        dialogLoader.item.show()
                    else
                        dialogLoader.active = true
                }
            }

            FishUI.Label {
                Layout.fillWidth: true
                visible: languageSettings.errorMessage.length > 0
                text: languageSettings.errorMessage
                wrapMode: Text.WordWrap
            }

        }
    }

    Loader {
        id: dialogLoader
        active: false
        sourceComponent: AddLanguageDialog {
            language: languageSettings
        }
        onLoaded: item.show()
    }

}
