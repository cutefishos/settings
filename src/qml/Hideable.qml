// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.4
import QtQuick.Layouts 1.3

Item {
    id: _root
    implicitHeight: shown ? _contentLayout.implicitHeight : 0
    Layout.fillWidth: true
    clip: true

    default property alias content: _contentLayout.data
    property bool shown: false
    property alias spacing: _contentLayout.spacing

    Behavior on implicitHeight {
        NumberAnimation {
            duration: 200
            easing.type: Easing.OutSine
        }
    }

    ColumnLayout {
        id: _contentLayout
        anchors.fill: parent
    }

    function show() {
        shown = true
    }

    function hide() {
        shown = false
    }

    function toggle() {
        shown = !shown
    }
}
