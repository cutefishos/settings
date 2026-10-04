// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts
import Cutefish.Settings
import FishUI as FishUI

FishUI.RoundedRectangle {
    Layout.fillWidth: true

    default property alias content : _mainLayout.data
    property alias spacing: _mainLayout.spacing
    property alias layout: _mainLayout
    property real horizontalPadding: Metrics.cardHorizontalPadding
    property real verticalPadding: Metrics.cardVerticalPadding

    color: FishUI.Theme.secondBackgroundColor
    radius: FishUI.Theme.bigRadius

    implicitHeight: _mainLayout.implicitHeight +
                    _mainLayout.anchors.topMargin +
                    _mainLayout.anchors.bottomMargin

    ColumnLayout {
        id: _mainLayout
        anchors.fill: parent
        anchors.leftMargin: horizontalPadding
        anchors.rightMargin: horizontalPadding
        anchors.topMargin: verticalPadding
        anchors.bottomMargin: verticalPadding
        spacing: Metrics.cardSpacing
    }
}
