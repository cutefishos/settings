// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import Cutefish.Settings
import FishUI as FishUI

FishUI.Button {
    id: control

    property color backgroundColor: FishUI.Theme.darkMode ? "#363636" : "#FFFFFF"
    property color pressedColor: FishUI.Theme.darkMode ? Qt.lighter(backgroundColor, 1.1)
                                                       : Qt.darker(backgroundColor, 1.2)

    implicitHeight: Metrics.buttonHeight
    leftPadding: Metrics.cardHorizontalPadding
    rightPadding: Metrics.cardHorizontalPadding
    scale: 1

    background: FishUI.RoundedRectangle {
        radius: FishUI.Theme.mediumRadius
        color: control.enabled && control.pressed ? control.pressedColor
             : control.backgroundColor
    }
}
