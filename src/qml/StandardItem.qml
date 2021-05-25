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
import FishUI 1.0 as FishUI

Item {
    id: control

    height: mainLayout.implicitHeight + FishUI.Theme.smallRadius * 2

    property alias key: keyLabel.text
    property alias value: valueLabel.text

    Layout.fillWidth: true

    Rectangle {
        id: background
        anchors.fill: parent
        color: "transparent"
        radius: FishUI.Theme.smallRadius
    }

    RowLayout {
        id: mainLayout
        anchors.fill: parent

        Label {
            id: keyLabel
            color: FishUI.Theme.textColor
        }

        Item {
            Layout.fillWidth: true
        }

        Label {
            id: valueLabel
            color: FishUI.Theme.disabledTextColor
        }
    }
}
