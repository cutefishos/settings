// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts
import Cutefish.Settings
import FishUI as FishUI

Item {
    id: control
    height: Metrics.m

    Layout.fillWidth: true

    Separator {
        anchors.centerIn: parent
        width: control.width
    }
}
