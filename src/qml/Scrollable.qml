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
import FishUI 1.0 as FishUI

Flickable {
    id: root
    flickableDirection: Flickable.VerticalFlick
    clip: true

    // The window header floats over the top of the page; content scrolls beneath it.
    readonly property real topInset: rootWindow.header.height

    topMargin: topInset + FishUI.Units.largeSpacing + FishUI.Units.smallSpacing
    bottomMargin: FishUI.Units.largeSpacing * 2
    leftMargin: FishUI.Units.largeSpacing * 2
    rightMargin: FishUI.Units.largeSpacing * 2

    contentWidth: width - (leftMargin + rightMargin)

    // Flickable does not include topMargin in its initial content position.
    Component.onCompleted: contentY = -topMargin

    FishUI.WheelHandler {
        id: wheelHandler
        target: root
    }

    FishUI.ScrollBar.vertical: FishUI.ScrollBar {
        topPadding: root.topInset
        bottomPadding: FishUI.Theme.smallRadius
    }

    FishUI.HeaderBackdrop {
        // Declared children of a Flickable would otherwise scroll with its content.
        parent: root
        z: 1
        width: root.width
        height: root.topInset
        view: root
        baseColor: rootWindow.background.color
    }
}
