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

import QtQuick 2.4
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3

import Cutefish.Settings 1.0
import Cutefish.Appearance 1.0 as FrameworkAppearance
import FishUI 1.0 as FishUI
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

                    HorizontalDivider {}

                    RowLayout {
                        spacing: Metrics.rowSpacing

                        FishUI.Label {
                            id: dimsTipsLabel
                            text: qsTr("Dim the wallpaper in dark theme")
                            Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                        }

                        Item {
                            Layout.fillWidth: true
                        }

                        FishUI.Switch {
                            checked: appearance.dimsWallpaper
                            height: dimsTipsLabel.height
                            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                            onCheckedChanged: appearance.setDimsWallpaper(checked)
                            rightPadding: 0
                            rightInset: 0
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
                                hoverEnabled: true
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
                                border.width: checked || _mouseArea.containsMouse ? 3 : 0

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
