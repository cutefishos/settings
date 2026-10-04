// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FishUI as FishUI

FishUI.Page {
    id: page

    property string headerTitle

    background: Rectangle {
        color: rootWindow.background.color
    }

    // No header of its own: the title is drawn by the window header next to the
    // navigation buttons, which floats over the page content.
}
