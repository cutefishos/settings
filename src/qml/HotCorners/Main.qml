// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FishUI as FishUI
import Cutefish.Settings
import "../"

ItemPage {
    id: page
    headerTitle: qsTr("Hot Corners")

    HotCorners {
        id: hotCorners
    }

    // The compositor's action names, "" for none, and what the menu calls them.
    readonly property var actions: ["", "toggle-overview", "toggle-show-desktop", "toggle-app-list", "lock-screen"]
    readonly property var actionNames: [qsTr("None"), qsTr("Show All Windows"), qsTr("Show Desktop"),
                                        qsTr("Launcher"), qsTr("Lock Screen")]

    component CornerRow: ListRow {
        id: row

        property string corner
        property string action
        property int modifiers

        interactive: false

        RowComboBox {
            model: page.actionNames
            currentIndex: Math.max(0, page.actions.indexOf(row.action))
            displayText: row.modifiers !== 0 && currentIndex > 0
                         ? hotCorners.modifierText(row.modifiers) + " – " + currentText
                         : currentText
            // Keys held while choosing are the ones the corner will ask for.
            onActivated: (index) => hotCorners.setCorner(row.corner, page.actions[index],
                                                         index > 0 ? hotCorners.heldModifiers() : 0)
        }
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent
            spacing: Metrics.sectionSpacing

            RoundedItem {
                visible: !hotCorners.available

                FishUI.Label {
                    Layout.fillWidth: true
                    text: qsTr("Hot corners are managed by the Cutefish compositor, which is not running.")
                    color: FishUI.Theme.disabledTextColor
                    wrapMode: Text.WordWrap
                }
            }

            ListCard {
                visible: hotCorners.available

                CornerRow {
                    text: qsTr("Top left")
                    corner: "top-left"
                    action: hotCorners.topLeft
                    modifiers: hotCorners.topLeftModifiers
                }

                CornerRow {
                    text: qsTr("Top right")
                    corner: "top-right"
                    action: hotCorners.topRight
                    modifiers: hotCorners.topRightModifiers
                }

                CornerRow {
                    text: qsTr("Bottom left")
                    corner: "bottom-left"
                    action: hotCorners.bottomLeft
                    modifiers: hotCorners.bottomLeftModifiers
                }

                CornerRow {
                    text: qsTr("Bottom right")
                    corner: "bottom-right"
                    action: hotCorners.bottomRight
                    modifiers: hotCorners.bottomRightModifiers
                }
            }

            FishUI.Label {
                visible: hotCorners.available
                Layout.fillWidth: true
                Layout.leftMargin: Metrics.cardHorizontalPadding
                Layout.rightMargin: Metrics.cardHorizontalPadding
                text: qsTr("Move the pointer into a corner of the screen to run its action. Hold modifier keys while choosing an action to make the corner work only while they are held.")
                color: FishUI.Theme.disabledTextColor
                wrapMode: Text.WordWrap
            }

            Item {
                Layout.fillHeight: true
            }
        }
    }
}
