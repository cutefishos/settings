/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     Reion Wong <reionwong@gmail.com>
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
import QtGraphicalEffects 1.0

import FishUI 1.0 as FishUI
import Cutefish.Settings 1.0
import Cutefish.Accounts 1.0
import "../"

ItemPage {
    headerTitle: qsTr("Mouse")

    CursorThemeModel {
        id: cursorModel
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent
            spacing: FishUI.Units.smallSpacing

            Label {
                text: qsTr("Theme")
                color: FishUI.Theme.disabledTextColor
                leftPadding: FishUI.Units.largeSpacing
                visible: _view.count > 0
            }

            GridView {
                id: _view
                Layout.fillWidth: true
                implicitHeight: Math.ceil(_view.count / rowCount) * cellHeight + FishUI.Units.largeSpacing
                model: cursorModel
                interactive: false
                visible: _view.count > 0

                cellHeight: itemHeight
                cellWidth: calcExtraSpacing(itemWidth, _view.width) + itemWidth

                currentIndex: cursorModel.themeIndex(cursorModel.currentTheme)

                property int rowCount: _view.width / itemWidth
                property int itemWidth: 250
                property int itemHeight: 170

                function calcExtraSpacing(cellSize, containerSize) {
                    var availableColumns = Math.floor(containerSize / cellSize)
                    var extraSpacing = 0
                    if (availableColumns > 0) {
                        var allColumnSize = availableColumns * cellSize
                        var extraSpace = Math.max(containerSize - allColumnSize, 0)
                        extraSpacing = extraSpace / availableColumns
                    }
                    return Math.floor(extraSpacing)
                }

                delegate: Item {
                    width: GridView.view.cellWidth
                    height: GridView.view.cellHeight
                    scale: _mouseArea.pressed ? 0.95 : 1.0

                    property bool isCurrent: _view.currentIndex === index

                    Behavior on scale {
                        NumberAnimation {
                            duration: 100
                        }
                    }

                    MouseArea {
                        id: _mouseArea
                        anchors.fill: parent
                        anchors.margins: FishUI.Units.largeSpacing
                        onClicked: {
                            _view.currentIndex = index
                            cursorModel.currentTheme = model.id
                            console.log(model.id)
                        }
                    }

                    Rectangle {
                        anchors.fill: parent
                        anchors.margins: FishUI.Units.largeSpacing
                        color: FishUI.Theme.secondBackgroundColor
                        radius: FishUI.Theme.mediumRadius
                        z: -1

                        border.width: isCurrent ? 3 : 0
                        border.color: FishUI.Theme.highlightColor
                    }

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: FishUI.Units.largeSpacing

                        FishUI.IconItem {
                            width: 24
                            height: 24
                            source: model.image
                            smooth: true
                            Layout.alignment: Qt.AlignHCenter
                        }

                        Label {
                            text: model.name
                            Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
                            bottomPadding: FishUI.Units.largeSpacing
                        }
                    }
                }
            }

            Item {
                height: FishUI.Units.smallSpacing
            }
        }
    }
}
