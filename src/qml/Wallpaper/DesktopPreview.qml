// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import FishUI 1.0 as FishUI

Item {
    id: control

    Component {
        id: wallpaperItem

        Image {
            id: image
            source: "file://" + background.path
        }
    }

    Component {
        id: colorItem

        Rectangle {
            color: background.color
        }
    }

    Loader {
        id: bgLoader
        anchors.fill: parent

        sourceComponent: {
            if (background.type === 0)
                return wallpaperItem

            return colorItem
        }
    }

    FishUI.RoundedRectangle {
        id: desktopItem

        anchors.left: parent.left
        anchors.top: parent.top

        anchors.leftMargin: 10
        anchors.topMargin: 10

        width: 30
        height: width

        radius: height * 0.2
        opacity: 0.5
        color: FishUI.Theme.backgroundColor
    }

    FishUI.RoundedRectangle {
        id: desktopItem2

        anchors.left: parent.left
        anchors.top: desktopItem.bottom

        anchors.leftMargin: 10
        anchors.topMargin: 10

        width: 30
        height: width

        radius: height * 0.2
        opacity: 0.5
        color: FishUI.Theme.backgroundColor
    }

    FishUI.RoundedRectangle {
        id: dockArea
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        anchors.leftMargin: 8
        anchors.rightMargin: 8
        anchors.bottomMargin: 8

        height: 38
        radius: height * 0.3
        color: FishUI.Theme.backgroundColor
        opacity: 0.7
    }

    layer.enabled: true
    layer.effect: FishUI.OpacityMask {
        maskSource: Item {
            width: control.width
            height: control.height

            FishUI.RoundedRectangle {
                anchors.fill: parent
                radius: FishUI.Theme.bigRadius
            }
        }
    }
}
