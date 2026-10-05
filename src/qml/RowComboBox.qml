// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts
import Cutefish.Settings
import FishUI as FishUI

// A ListRow's choice: no fill until the pointer is over it or its list is open.
FishUI.ComboBox {
    id: control

    flat: true
    // Smaller than a framed box's arrow, and nearer the fill's end: the arrow's
    // drawing ends where a switch does, with the hover fill short of the card's edge.
    indicator: FishUI.Icon {
        x: control.mirrored ? FishUI.Units.smallSpacing : control.width - width - FishUI.Units.smallSpacing
        y: Math.round(control.topPadding + (control.availableHeight - height) / 2)
        // Above 12 px, so the glyph keeps its thin drawing rather than the heavier small one.
        width: 14
        height: 14
        name: "chevron-down"
        color: control.enabled ? FishUI.Theme.textColor : FishUI.Theme.disabledTextColor
        // Faded as far as FishUI.Label, so a disabled box matches its row's label.
        opacity: control.enabled ? 1 : 0.6
    }
    leftPadding: mirrored ? FishUI.Units.smallSpacing + indicator.width + spacing : FishUI.Units.largeSpacing
    rightPadding: mirrored ? FishUI.Units.largeSpacing : FishUI.Units.smallSpacing + indicator.width + spacing
    Layout.rightMargin: Metrics.trailingGlyphMargin("chevron-down", indicator.width, FishUI.Units.smallSpacing)

    background: FishUI.RoundedRectangle {
        // Fits a plain row, so a choice does not make its row taller than a switch does.
        implicitHeight: FishUI.Units.iconSizes.medium

        radius: FishUI.Theme.smallRadius
        color: Qt.rgba(FishUI.Theme.textColor.r, FishUI.Theme.textColor.g, FishUI.Theme.textColor.b,
                       control.menuVisible ? (FishUI.Theme.darkMode ? 0.12 : 0.1)
                                           : (FishUI.Theme.darkMode ? 0.08 : 0.06))
        visible: control.enabled && (control.hovered || control.menuVisible)
    }
}
