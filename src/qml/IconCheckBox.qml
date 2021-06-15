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
import QtGraphicalEffects 1.12
import FishUI 1.0 as FishUI

Item {
    id: control

    property var iconSpacing: FishUI.Units.smallSpacing * 0.8
    property alias source: icon.source
    property alias text: label.text
    property bool checked: false

    property var iconSize: 104

    signal clicked

    implicitHeight: mainLayout.implicitHeight
    implicitWidth: mainLayout.implicitWidth

    scale: 1.0

    ColumnLayout {
        id: mainLayout
        anchors.fill: parent
        spacing: FishUI.Units.smallSpacing

        Image {
            id: icon
            width: control.iconSize
            height: width
            sourceSize: Qt.size(icon.width, icon.height)
            opacity: 1

            layer.enabled: true
            layer.effect: OpacityMask {
                maskSource: Item {
                    width: icon.width
                    height: icon.height

                    Rectangle {
                        anchors.fill: parent
                        radius: FishUI.Theme.bigRadius
                    }
                }
            }

            Behavior on opacity {
                NumberAnimation {
                    duration: 100
                    easing.type: Easing.InOutCubic
                }
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true

                onEntered: function() {
                    icon.opacity = 0.8
                }
                onExited: function() {
                    icon.opacity = 1.0
                }
            }
        }

        Label {
            id: label
            visible: label.text
            Layout.alignment: Qt.AlignHCenter
        }

        RadioButton {
            checkable: false
            checked: control.checked
            Layout.alignment: Qt.AlignHCenter
        }
    }

    Behavior on scale {
        NumberAnimation {
            duration: 100
        }
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton
        onClicked: control.clicked()

        onPressedChanged: {
            control.scale = pressed ? 0.95 : 1.0
        }
    }
}
