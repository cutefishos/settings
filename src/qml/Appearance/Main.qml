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
import QtGraphicalEffects 1.0

import Cutefish.Settings 1.0
import FishUI 1.0 as FishUI
import "../"

ItemPage {
    headerTitle: qsTr("Appearance")

    FontsModel {
        id: fontsModel
    }

    Appearance {
        id: appearance
    }

    Connections {
        target: fontsModel

        function onLoadFinished() {
            for (var i in fontsModel.generalFonts) {
                if (fontsModel.systemGeneralFont === fontsModel.generalFonts[i]) {
                    generalFontComboBox.currentIndex = i
                    break;
                }
            }

            for (i in fontsModel.fixedFonts) {
                if (fontsModel.systemFixedFont === fontsModel.fixedFonts[i]) {
                    fixedFontComboBox.currentIndex = i
                    break;
                }
            }

            console.log("fonts load finished")
        }
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent
            // anchors.bottomMargin: FishUI.Units.largeSpacing
            spacing: FishUI.Units.largeSpacing * 2

            RoundedItem {
                Label {
                    text: qsTr("Theme")
                    color: FishUI.Theme.disabledTextColor
                    bottomPadding: FishUI.Units.smallSpacing
                }

                // Light Mode and Dark Mode
                RowLayout {
                    spacing: FishUI.Units.largeSpacing * 2

                    IconCheckBox {
                        source: "qrc:/images/light_mode.svg"
                        text: qsTr("Light")
                        checked: !FishUI.Theme.darkMode
                        onClicked: appearance.switchDarkMode(false)
                    }

                    IconCheckBox {
                        source: "qrc:/images/dark_mode.svg"
                        text: qsTr("Dark")
                        checked: FishUI.Theme.darkMode
                        onClicked: appearance.switchDarkMode(true)
                    }
                }

                Item {
                    height: FishUI.Units.largeSpacing
                }

                RowLayout {
                    spacing: FishUI.Units.largeSpacing

                    Label {
                        id: dimsTipsLabel
                        text: qsTr("Dim the wallpaper in dark theme")
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Switch {
                        checked: appearance.dimsWallpaper
                        height: dimsTipsLabel.height
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                        onCheckedChanged: appearance.setDimsWallpaper(checked)
                        rightPadding: 0
                    }
                }
            }

            RoundedItem {
                Label {
                    text: qsTr("Accent color")
                    color: FishUI.Theme.disabledTextColor
                    bottomPadding: FishUI.Units.smallSpacing
                }

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

                        Rectangle {
                            anchors.fill: parent
                            anchors.margins: FishUI.Units.smallSpacing
                            color: "transparent"
                            radius: width / 2

                            border.color: _mouseArea.pressed ? Qt.rgba(currentColor.r,
                                                                       currentColor.g,
                                                                       currentColor.b, 0.6)
                                                             : Qt.rgba(currentColor.r,
                                                                       currentColor.g,
                                                                       currentColor.b, 0.4)
                            border.width: checked || _mouseArea.containsMouse ? 3 : 0

                            Rectangle {
                                anchors.fill: parent
                                anchors.margins: FishUI.Units.smallSpacing
                                color: currentColor
                                radius: width / 2

                                Image {
                                    anchors.centerIn: parent
                                    width: parent.height * 0.6
                                    height: width
                                    sourceSize: Qt.size(width, height)
                                    source: "qrc:/images/dark/checked.svg"
                                    visible: checked
                                }
                            }
                        }
                    }
                }
            }

            RoundedItem {
                // Font
                Label {
                    text: qsTr("Font")
                    color: FishUI.Theme.disabledTextColor
                    bottomPadding: FishUI.Units.smallSpacing
                }

                GridLayout {
                    rows: 3
                    columns: 2

                    columnSpacing: FishUI.Units.largeSpacing * 1.5
                    rowSpacing: FishUI.Units.largeSpacing

                    Label {
                        text: qsTr("General Font")
                        bottomPadding: FishUI.Units.smallSpacing
                    }

                    ComboBox {
                        id: generalFontComboBox
                        model: fontsModel.generalFonts
                        enabled: true
                        Layout.fillWidth: true
                        topInset: 0
                        bottomInset: 0
                        leftPadding: FishUI.Units.largeSpacing
                        rightPadding: FishUI.Units.largeSpacing
                        onActivated: appearance.setGenericFontFamily(currentText)
                    }

                    Label {
                        text: qsTr("Fixed Font")
                        bottomPadding: FishUI.Units.smallSpacing
                    }

                    ComboBox {
                        id: fixedFontComboBox
                        model: fontsModel.fixedFonts
                        enabled: true
                        Layout.fillWidth: true
                        topInset: 0
                        bottomInset: 0
                        leftPadding: FishUI.Units.largeSpacing
                        rightPadding: FishUI.Units.largeSpacing
                        onActivated: appearance.setFixedFontFamily(currentText)
                    }

                    Label {
                        text: qsTr("Font Size")
                        bottomPadding: FishUI.Units.smallSpacing
                    }

                    TabBar {
                        Layout.fillWidth: true

                        TabButton {
                            text: qsTr("Small")
                        }

                        TabButton {
                            text: qsTr("Medium")
                        }

                        TabButton {
                            text: qsTr("Large")
                        }

                        TabButton {
                            text: qsTr("Huge")
                        }

                        currentIndex: {
                            var index = 0

                            if (appearance.fontPointSize <= 10)
                                index = 0
                            else if (appearance.fontPointSize <= 11)
                                index = 1
                            else if (appearance.fontPointSize <= 13)
                                index = 2
                            else if (appearance.fontPointSize <= 16)
                                index = 3

                            return index
                        }

                        onCurrentIndexChanged: {
                            var fontSize = 0

                            switch (currentIndex) {
                            case 0:
                                fontSize = 10
                                break;
                            case 1:
                                fontSize = 11
                                break;
                            case 2:
                                fontSize = 13
                                break;
                            case 3:
                                fontSize = 16
                                break;
                            }

                            appearance.setFontPointSize(fontSize)
                        }
                    }
                }
            }

            Item {
                Layout.fillHeight: true
            }
        }
    }
}
