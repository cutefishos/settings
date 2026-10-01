// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.4
import QtQuick.Layouts 1.3
import Cutefish.Settings 1.0
import FishUI 1.0 as FishUI

Item {
    id: control
    height: Metrics.m

    Layout.fillWidth: true

    Separator {
        anchors.centerIn: parent
        width: control.width
    }
}
