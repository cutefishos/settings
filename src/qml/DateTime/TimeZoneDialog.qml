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

import QtQuick 2.0
import QtQuick.Controls 2.12
import QtQuick.Window 2.12
import QtQuick.Layouts 1.12
import FishUI 1.0 as FishUI

FishUI.Window {
    id: control
    title: "Hello World"
    width: 900
    height: 600
    visible: false

    background.opacity: 0.5
    contentTopMargin: 0

    onWidthChanged: control.reset()
    onHeightChanged: control.reset()

    function reset() {
        _popupItem.opacity = 0
        dot.visible = false
    }

    onVisibleChanged: {
        if (!visible)
            control.reset()
    }

    FishUI.WindowBlur {
        view: control
        geometry: Qt.rect(control.x, control.y, control.width, control.height)
        windowRadius: control.background.radius
        enabled: true
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onPressed: {
            if (_popupItem.opacity === 1) {
                control.reset()
                return
            }

            timeZoneMap.clicked(mouse.x, mouse.y, control.width, control.height)
            _popupItem.x = mouse.x
            _popupItem.y = mouse.y
            _popupItem.opacity = 1
            dot.show(mouse.x, mouse.y)
        }
    }

    Image {
        anchors.fill: parent
        source: FishUI.Theme.darkMode ? "qrc:/images/dark/world.svg" : "qrc:/images/light/world.svg"
        sourceSize: Qt.size(width, height)
        fillMode: Image.PreserveAspectFit
    }

    Rectangle {
        id: dot
        width: 20
        height: 20
        radius: height / 2
        color: FishUI.Theme.highlightColor
        z: 99
        visible: false
        border.width: 5
        border.color: Qt.rgba(FishUI.Theme.highlightColor.r,
                              FishUI.Theme.highlightColor.g,
                              FishUI.Theme.highlightColor.b, 0.5)

        function show(x, y) {
            dot.x = x - dot.width / 2
            dot.y = y - dot.height / 2
            dot.visible = true
        }
    }

    Item {
        id: _popupItem
        width: 200
        height: _popupLayout.implicitHeight + FishUI.Units.largeSpacing
        z: 100

        opacity: _view.count > 0 ? 1.0 : 0.0

        Behavior on opacity {
            NumberAnimation { duration: 200 }
        }

        Rectangle {
            anchors.fill: parent
            color: FishUI.Theme.secondBackgroundColor
            radius: FishUI.Theme.mediumRadius
            border.width: 1
            border.color: Qt.rgba(FishUI.Theme.textColor.r,
                                  FishUI.Theme.textColor.g,
                                  FishUI.Theme.textColor.b, 0.2)
        }

        ColumnLayout {
            id: _popupLayout
            anchors.fill: parent
            anchors.margins: FishUI.Units.smallSpacing

            ListView {
                id: _view
                clip: true
                Layout.fillWidth: true
                Layout.preferredHeight: itemSize * _view.count
                model: timeZoneMap.availableList

                property int itemSize: 30

                delegate: Item {
                    id: _item
                    width: ListView.view.width
                    height: _view.itemSize

                    Rectangle {
                        anchors.fill: parent
                        radius: FishUI.Theme.mediumRadius
                        color: FishUI.Theme.highlightColor
                        visible: index === _view.currentIndex
                        opacity: 0.8
                    }

                    MouseArea {
                        anchors.fill: parent
                        anchors.margins: 2
                        onClicked: _view.currentIndex = index
                    }

                    RowLayout {
                        anchors.fill: parent

                        Label {
                            Layout.alignment: Qt.AlignCenter
                            elide: Label.ElideRight
                            text: modelData
                            color: index === _view.currentIndex ? FishUI.Theme.highlightedTextColor : FishUI.Theme.textColor
                        }
                    }
                }
            }

            Button {
                Layout.preferredHeight: _view.itemSize
                Layout.fillWidth: true
                text: qsTr("Set")
                onClicked: {
                    _popupItem.opacity = 0
                    timeZoneMap.setTimeZone(timeZoneMap.availableList[_view.currentIndex])
                }
            }
        }
    }
}
