// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.4
import QtQuick.Controls 2.4
import Cutefish.Settings 1.0
import FishUI 1.0 as FishUI

Flickable {
    id: root
    flickableDirection: Flickable.VerticalFlick
    clip: true

    // The window header floats over the top of the page; content scrolls beneath it.
    readonly property real topInset: rootWindow.headerInset

    topMargin: topInset + Metrics.pageTopMargin
    bottomMargin: Metrics.pageMargin
    leftMargin: Metrics.pageMargin
    rightMargin: Metrics.pageMargin

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
