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
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0

Item {
    id: _root
    implicitHeight: shown ? _content.implicitHeight : 0
    clip: true
    Layout.fillWidth: true

    Behavior on implicitHeight {
        NumberAnimation {
            duration: 125
            easing.type: Easing.InOutCubic
        }
    }

    property bool shown: false
    default property alias content: _content.data

    function show() {
        shown = true
    }

    function hide() {
        shown = false
    }

    function toggle() {
        shown = !shown
    }

    ColumnLayout {
        id: _content
        anchors.fill: parent
    }
}
