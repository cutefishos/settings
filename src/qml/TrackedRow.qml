// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQml

// The row of a model whose role equals a value, followed across re-sorts. A sorting proxy
// changes the data under its rows in place, so a delegate's model can turn into another row.
Instantiator {
    id: control

    property string role
    property var value
    // The row's roles, or null while the model holds no such row.
    property var row: null

    delegate: QtObject {
        readonly property var key: model[control.role]
        readonly property var roles: model

        onKeyChanged: control.update()
    }

    onObjectAdded: update()
    onObjectRemoved: update()
    onValueChanged: update()
    // Found at once, so a dialog fills in its switches before it is shown, not animated after.
    Component.onCompleted: find()

    function update() {
        Qt.callLater(control.find)
    }

    function find() {
        for (var i = 0; i < control.count; ++i) {
            var object = control.objectAt(i)
            if (object && object.key === control.value) {
                control.row = object.roles
                return
            }
        }
        control.row = null
    }
}
