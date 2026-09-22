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
import QtQuick.Window 2.12
import QtQuick.Layouts 1.12
import FishUI 1.0 as FishUI

FishUI.Window {
    id: control

    windowButtonsTopMargin: 8
    windowButtonsRightMargin: 8
    minimizeButtonVisible: false
    header.height: 44
    title: "Time Zone"
    width: 900
    height: 600
    minimumWidth: 900
    minimumHeight: 600
    maximumWidth: 900
    maximumHeight: 600
    visible: false
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint

    background.color: FishUI.Theme.secondBackgroundColor
    background.opacity: blurAvailable ? 0.5 : 1
    contentTopMargin: 0

    // The zone the dot and the Set button refer to.
    property string selectedZone: ""


    onVisibleChanged: {
        if (visible) {
            searchField.clear()
            selectZone(timeZoneMap.currentTimeZoneId)
            searchField.forceActiveFocus()
        } else {
            resultsPopup.close()
        }
    }

    function selectZone(zone) {
        if (!zone)
            return
        timeZoneMap.select(zone)
    }

    function pickResult(index) {
        if (index < 0 || index >= resultsView.count)
            return
        var zone = resultsView.model[index].id
        resultsPopup.close()
        searchField.text = ""
        selectZone(zone)
    }

    // Marks the selected zone where it sits on the map, with its name beside.
    function place() {
        if (!selectedZone) {
            dot.visible = false
            popupItem.visible = false
            return
        }
        // The projection covers the painted map, which the aspect-fit image
        // centres inside the item.
        var point = timeZoneMap.position(selectedZone, _worldMap.paintedWidth, _worldMap.paintedHeight)
        if (point.x < 0) {
            dot.visible = false
            popupItem.visible = false
            return
        }
        point.x += (_worldMap.width - _worldMap.paintedWidth) / 2
        point.y += (_worldMap.height - _worldMap.paintedHeight) / 2
        dot.show(point.x, point.y)
        popupText.text = timeZoneMap.localeTimeZoneName(selectedZone)
        popupItem.x = point.x + FishUI.Units.smallSpacing * 1.5
        popupItem.y = point.y + FishUI.Units.smallSpacing * 1.5
        if (popupItem.x + popupItem.width >= _worldMap.width)
            popupItem.x = _worldMap.width - popupItem.width - 2
        if (popupItem.y + popupItem.height >= _worldMap.height)
            popupItem.y = _worldMap.height - popupItem.height - 2
        popupItem.visible = true
    }

    Connections {
        target: timeZoneMap

        function onAvailableListChanged() {
            control.selectedZone = timeZoneMap.availableList.length > 0
                    ? timeZoneMap.availableList[0] : ""
            control.place()
        }
    }

    Item {
        z: -1
        anchors.fill: parent

        DragHandler {
            acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
            grabPermissions: PointerHandler.CanTakeOverFromItems | PointerHandler.CanTakeOverFromHandlersOfDifferentType | PointerHandler.ApprovesTakeOverByAnything
            onActiveChanged: if (active) control.helper.startSystemMove(control)
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16

        // The title sits at the left and the search box in the middle of the
        // window, not in the middle of what the title leaves free.
        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: searchField.implicitHeight

            FishUI.Label {
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                text: control.title
                textFormat: Text.PlainText
                font.pixelSize: 14
                font.weight: Font.DemiBold
                color: FishUI.Theme.textColor
            }

            // Typing lists matching zones; Up/Down move through them, Enter picks.
            FishUI.TextField {
                id: searchField
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                width: 320
                placeholderText: qsTr("Search for a city or time zone")
                selectByMouse: true

                onTextChanged: {
                    resultsView.model = timeZoneMap.search(text)
                    resultsView.currentIndex = resultsView.count > 0 ? 0 : -1
                    if (resultsView.count > 0)
                        resultsPopup.open()
                    else
                        resultsPopup.close()
                }
                onAccepted: control.pickResult(resultsView.currentIndex)
                Keys.onDownPressed: {
                    if (resultsView.count > 0)
                        resultsView.currentIndex = (resultsView.currentIndex + 1) % resultsView.count
                }
                Keys.onUpPressed: {
                    if (resultsView.count > 0)
                        resultsView.currentIndex = (resultsView.currentIndex - 1 + resultsView.count) % resultsView.count
                }
                Keys.onEscapePressed: function(event) {
                    if (resultsPopup.opened) {
                        resultsPopup.close()
                        event.accepted = true
                    }
                }

                Popup {
                    id: resultsPopup
                    y: searchField.height + 4
                    width: searchField.width
                    height: Math.min(resultsView.contentHeight + padding * 2, 320)
                    padding: 4
                    focus: false
                    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

                    background: FishUI.RoundedRectangle {
                        radius: FishUI.Theme.mediumRadius
                        color: FishUI.Theme.secondBackgroundColor
                        border.width: 1
                        border.color: Qt.rgba(FishUI.Theme.textColor.r, FishUI.Theme.textColor.g,
                                              FishUI.Theme.textColor.b, 0.15)
                    }

                    ListView {
                        id: resultsView
                        anchors.fill: parent
                        clip: true
                        boundsBehavior: Flickable.StopAtBounds
                        highlightMoveDuration: 0
                        onCurrentIndexChanged: positionViewAtIndex(currentIndex, ListView.Contain)
                        FishUI.ScrollBar.vertical: FishUI.ScrollBar {}
                        FishUI.WheelHandler { target: resultsView }

                        delegate: Item {
                            width: ListView.view.width
                            height: 36
                            readonly property bool current: ListView.isCurrentItem

                            FishUI.RoundedRectangle {
                                anchors.fill: parent
                                radius: FishUI.Theme.smallRadius
                                color: current ? FishUI.Theme.highlightColor
                                     : resultArea.containsMouse
                                       ? Qt.rgba(FishUI.Theme.textColor.r, FishUI.Theme.textColor.g,
                                                 FishUI.Theme.textColor.b, 0.08)
                                       : "transparent"
                            }

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: FishUI.Units.largeSpacing
                                anchors.rightMargin: FishUI.Units.largeSpacing
                                spacing: FishUI.Units.largeSpacing

                                FishUI.Label {
                                    Layout.fillWidth: true
                                    text: modelData.name
                                    color: current ? FishUI.Theme.highlightedTextColor : FishUI.Theme.textColor
                                    elide: Text.ElideRight
                                }

                                FishUI.Label {
                                    text: modelData.offset
                                    color: current ? FishUI.Theme.highlightedTextColor : FishUI.Theme.disabledTextColor
                                    font.pointSize: 9
                                }
                            }

                            MouseArea {
                                id: resultArea
                                anchors.fill: parent
                                hoverEnabled: true
                                onClicked: control.pickResult(index)
                            }
                        }
                    }
                }
            }
        }

        Image {
            id: _worldMap
            Layout.fillWidth: true
            Layout.fillHeight: true
            source: FishUI.Theme.darkMode ? "qrc:/images/dark/world.svg" : "qrc:/images/light/world.svg"
            sourceSize: Qt.size(width, height)
            fillMode: Image.PreserveAspectFit
            onPaintedWidthChanged: control.place()
            onPaintedHeightChanged: control.place()

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
                id: popupItem
                visible: popupText.text !== ""
                width: popupText.implicitWidth + FishUI.Units.largeSpacing
                height: popupText.implicitHeight + FishUI.Units.largeSpacing

                FishUI.RoundedRectangle {
                    anchors.fill: parent
                    radius: FishUI.Theme.smallRadius
                    color: FishUI.Theme.highlightColor
                }

                FishUI.Label {
                    id: popupText
                    anchors.centerIn: parent
                    color: FishUI.Theme.highlightedTextColor
                }
            }

            MouseArea {
                anchors.fill: parent

                onClicked: {
                    resultsPopup.close()
                    timeZoneMap.clicked(mouse.x - (_worldMap.width - _worldMap.paintedWidth) / 2,
                                        mouse.y - (_worldMap.height - _worldMap.paintedHeight) / 2,
                                        _worldMap.paintedWidth, _worldMap.paintedHeight)
                }
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignRight
            spacing: 8

            FishUI.ConfirmDialogButton {
                text: qsTr("Cancel")
                onClicked: control.close()
            }

            FishUI.ConfirmDialogButton {
                text: qsTr("Set")
                primary: true
                // Only a change is worth applying.
                enabled: control.selectedZone !== "" && control.selectedZone !== timeZoneMap.currentTimeZoneId
                onClicked: {
                    timeZoneMap.setTimeZone(control.selectedZone)
                    control.close()
                }
            }

        }
    }
}
