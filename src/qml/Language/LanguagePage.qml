// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

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
            spacing: Metrics.sectionSpacing

            ColumnLayout {
                Layout.fillWidth: true
                spacing: Metrics.sectionLabelSpacing

                SectionLabel {
                    text: qsTr("Preferred languages")
                }

                RoundedItem {
                    verticalPadding: Metrics.xs

                    PreferredLanguageList {
                        Layout.fillWidth: true
                        language: languageSettings
                    }
                }

                FishUI.Label {
                    Layout.fillWidth: true
                    leftPadding: Metrics.cardHorizontalPadding
                    rightPadding: Metrics.cardHorizontalPadding
                    text: qsTr("Drag a language to the top to make it the system language. Right-click a language to remove it.")
                    color: FishUI.Theme.disabledTextColor
                    wrapMode: Text.WordWrap
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
