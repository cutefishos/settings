// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Cutefish.Settings
import Cutefish.Appearance as FrameworkAppearance
import FishUI as FishUI
import "../"

ItemPage {
    headerTitle: qsTr("Appearance")

    FrameworkAppearance.Appearance {
        id: appearance
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
                    text: qsTr("Appearance")
                }

                RoundedItem {
                    // Light Mode and Dark Mode
                    RowLayout {
                        spacing: Metrics.tileSpacing

                        ImageSelectionItem {
                            source: "qrc:/images/light_mode.svg"
                            text: qsTr("Light")
                            // The 100x68 artwork at its own size inside the 4px inset: the
                            // SVG loader letterboxes any other aspect with transparent bars.
                            previewWidth: 108
                            previewHeight: 76
                            checked: !FishUI.Theme.darkMode
                            onClicked: appearance.switchDarkMode(false)
                        }

                        ImageSelectionItem {
                            source: "qrc:/images/dark_mode.svg"
                            text: qsTr("Dark")
                            previewWidth: 108
                            previewHeight: 76
                            checked: FishUI.Theme.darkMode
                            onClicked: appearance.switchDarkMode(true)
                        }
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: Metrics.sectionLabelSpacing

                SectionLabel {
                    text: qsTr("Accent color")
                }

                RoundedItem {
                    GridView {
                        id: accentColorView
                        height: itemSize
                        Layout.fillWidth: true
                        // The ring sits smallSpacing inside its cell; it, like a preview's
                        // selection ring, starts on the text column.
                        Layout.leftMargin: -FishUI.Units.smallSpacing
                        cellWidth: height
                        cellHeight: height
                        interactive: false
                        model: ListModel {}

                        property var itemSize: 30 + FishUI.Units.largeSpacing * 2

                        Component.onCompleted: {
                            model.append({"accentColor": String(FishUI.Theme.blueColor)})
                            model.append({"accentColor": String(FishUI.Theme.redColor)})
                            model.append({"accentColor": String(FishUI.Theme.greenColor)})
                            model.append({"accentColor": String(FishUI.Theme.purpleColor)})
                            model.append({"accentColor": String(FishUI.Theme.pinkColor)})
                            model.append({"accentColor": String(FishUI.Theme.orangeColor)})
                            model.append({"accentColor": String(FishUI.Theme.greyColor)})
                        }

                        delegate: Item {
                            id: _accentColorItem

                            property bool checked: Qt.colorEqual(FishUI.Theme.highlightColor, accentColor)
                            property color currentColor: accentColor

                            width: GridView.view.itemSize
                            height: width

                            MouseArea {
                                id: _mouseArea
                                anchors.fill: parent
                                onClicked: appearance.setAccentColor(index)
                            }

                            // The dot is a parallel curve of the ring, so both snap to
                            // the same circle and the gap is even all round.
                            FishUI.RoundedRectangle {
                                id: _accentRing
                                anchors.fill: parent
                                anchors.margins: FishUI.Units.smallSpacing
                                color: "transparent"
                                radius: width / 2
                                continuousCorners: false

                                border.color: _mouseArea.pressed ? Qt.rgba(currentColor.r,
                                                                           currentColor.g,
                                                                           currentColor.b, 0.6)
                                                                 : Qt.rgba(currentColor.r,
                                                                           currentColor.g,
                                                                           currentColor.b, 0.4)
                                border.width: checked || _mouseArea.pressed ? 3 : 0

                                FishUI.RoundedRectangle {
                                    anchors.fill: parent
                                    anchors.margins: FishUI.Units.smallSpacing
                                    color: currentColor
                                    radius: _accentRing.radius
                                    inset: FishUI.Units.smallSpacing
                                    continuousCorners: false

                                    FishUI.Icon {
                                        name: "check"
                                        anchors.centerIn: parent
                                        width: parent.height * 0.6
                                        height: width
                                        visible: checked
                                        color: FishUI.Theme.highlightedTextColor
                                    }
                                }
                            }
                        }
                    }
                }
            }

            ListCard {
                NavigationItem {
                    text: qsTr("Fonts")
                    leading: RowIcon {
                        source: "fonts.svg"
                        tint: "#FE8433"
                    }
                    onClicked: rootWindow.pushPage("qrc:/qml/Appearance/Fonts.qml")
                }

                NavigationItem {
                    text: qsTr("Icon Theme")
                    leading: RowIcon {
                        source: "icontheme.svg"
                        tint: "#2EC347"
                    }
                    onClicked: rootWindow.pushPage("qrc:/qml/Appearance/IconTheme.qml")
                }

                NavigationItem {
                    text: qsTr("Cursor Style")
                    leading: RowIcon {
                        source: "cursor.svg"
                        tint: "#3385FF"
                    }
                    onClicked: rootWindow.pushPage("qrc:/qml/Appearance/CursorTheme.qml")
                }
            }
        }
    }
}
