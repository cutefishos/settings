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
import Cutefish.Settings 1.0
import FishUI 1.0 as FishUI

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
