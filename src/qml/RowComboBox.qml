// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import FishUI as FishUI

// A ListRow's choice: no fill until the pointer is over it or its list is open.
FishUI.ComboBox {
    id: control

    flat: true

    background: FishUI.RoundedRectangle {
        implicitHeight: FishUI.Units.iconSizes.medium + FishUI.Units.smallSpacing / 2

        radius: FishUI.Theme.smallRadius
        color: Qt.rgba(FishUI.Theme.textColor.r, FishUI.Theme.textColor.g, FishUI.Theme.textColor.b,
                       control.menuVisible ? (FishUI.Theme.darkMode ? 0.12 : 0.1)
                                           : (FishUI.Theme.darkMode ? 0.08 : 0.06))
        visible: control.enabled && (control.hovered || control.menuVisible)
    }
}
