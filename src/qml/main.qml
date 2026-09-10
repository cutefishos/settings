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
import QtQuick.Window 2.3
import FishUI 1.0 as FishUI

FishUI.Window {
    id: rootWindow
    title: qsTr("Settings")
    visible: true
    width: 900
    height: 610

    minimumWidth: 900
    minimumHeight: 600

    property alias stackView: _stackView
    property string pendingPageName: ""

    background.color: FishUI.Theme.backgroundColor
    background.opacity: blurAvailable ? (FishUI.Theme.darkMode ? 0.7 : 0.5) : 1
    header.height: 40
    contentTopMargin: 0

    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    Item {
        anchors.fill: parent

        SideBar {
            id: sideBar
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            // Keep the sidebar edge and the content clip on the same physical pixel.
            readonly property real pixelRatio: FishUI.Dpi.ratio
            width: rootWindow.LayoutMirroring.enabled
                   ? parent.width - Math.round((parent.width - implicitWidth) * pixelRatio) / pixelRatio
                   : Math.round(implicitWidth * pixelRatio) / pixelRatio

            onCurrentIndexChanged: {
                if (!sideBar.updatingModel)
                    switchPageFromIndex(currentIndex)
            }
            onCurrentPageRemoved: switchPageFromIndex(currentIndex)
            onBluetoothItemAdded: {
                if (rootWindow.pendingPageName === "bluetooth")
                    rootWindow.switchPageFromName("bluetooth")
            }
        }

        StackView {
            id: _stackView
            anchors.left: sideBar.right
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            initialItem: Qt.resolvedUrl(sideBar.model.get(0).page)
            clip: true

            pushEnter: Transition {}
            pushExit: Transition {}
        }
    }

    function switchPageFromIndex(index) {
        if (index < 0 || index >= sideBar.model.count)
            return
        pendingPageName = ""
        _stackView.pop()
        _stackView.push(Qt.resolvedUrl(sideBar.model.get(index).page))
    }

    function switchPageFromName(pageName) {
        pendingPageName = pageName === "bluetooth" ? pageName : ""
        for (var i = 0; i < sideBar.model.count; ++i) {
            if (pageName === sideBar.model.get(i).name) {
                switchPageFromIndex(i)
                sideBar.view.currentIndex = i
            }
        }

        // If the window is minimized, it needs to be displayed again.
        rootWindow.show()
        rootWindow.raise()
    }
}
