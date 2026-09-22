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

import QtQuick 2.15
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
    readonly property bool mockNetworkData: Qt.application.arguments.indexOf("--mock-network") >= 0

    // Entries are {page, rootPage, properties}; rootPage is the sidebar page a sub page belongs to.
    property var history: []
    property int historyIndex: -1
    property bool syncingSideBar: false
    readonly property bool canGoBack: historyStep(-1) >= 0
    readonly property bool canGoForward: historyStep(1) >= 0

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
                if (!sideBar.updatingModel && !rootWindow.syncingSideBar)
                    switchPageFromIndex(currentIndex)
            }
            onCurrentPageRemoved: switchPageFromIndex(currentIndex)
            onBluetoothItemAdded: {
                if (rootWindow.pendingPageName === "bluetooth")
                    rootWindow.switchPageFromName("bluetooth")
            }
        }

        FishUI.StackView {
            id: _stackView
            anchors.left: sideBar.right
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            clip: true

            pushEnter: Transition {}
            pushExit: Transition {}
            replaceEnter: Transition {}
            replaceExit: Transition {}
        }
    }

    headerItem: Item {
        RowLayout {
            id: _headerRow
            anchors.fill: parent
            anchors.leftMargin: sideBar.width + FishUI.Units.smallSpacing * 1.5
            anchors.rightMargin: FishUI.Units.smallSpacing
            spacing: FishUI.Units.smallSpacing / 2

            readonly property int buttonSize: 31

            NavigationButton {
                Layout.alignment: rootWindow.windowButtonsAlignment
                Layout.topMargin: rootWindow.windowButtonsTopMargin
                Layout.preferredWidth: _headerRow.buttonSize
                Layout.preferredHeight: _headerRow.buttonSize
                enabled: rootWindow.canGoBack
                iconName: "chevron-left"
                onClicked: rootWindow.goBack()
            }

            NavigationButton {
                Layout.alignment: rootWindow.windowButtonsAlignment
                Layout.topMargin: rootWindow.windowButtonsTopMargin
                Layout.preferredWidth: _headerRow.buttonSize
                Layout.preferredHeight: _headerRow.buttonSize
                enabled: rootWindow.canGoForward
                iconName: "chevron-right"
                onClicked: rootWindow.goForward()
            }

            FishUI.Label {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.leftMargin: FishUI.Units.smallSpacing / 2
                leftPadding: rootWindow.windowButtonsTopMargin
                topPadding: rootWindow.windowButtonsTopMargin / 2
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
                color: rootWindow.active ? FishUI.Theme.textColor : FishUI.Theme.disabledTextColor
                font.pointSize: 11
                text: _stackView.currentItem && _stackView.currentItem.headerTitle !== undefined
                      ? _stackView.currentItem.headerTitle : ""
            }
        }
    }

    Shortcut {
        sequences: [StandardKey.Back]
        onActivated: rootWindow.goBack()
    }

    Shortcut {
        sequences: [StandardKey.Forward]
        onActivated: rootWindow.goForward()
    }

    // Deferred: sidebar items drop unavailable pages in their own onCompleted handlers.
    Component.onCompleted: Qt.callLater(function() {
        if (historyIndex < 0)
            switchPageFromIndex(0)
    })

    function sideBarIndexOf(page) {
        for (var i = 0; i < sideBar.model.count; ++i) {
            if (sideBar.model.get(i).page === page)
                return i
        }
        return -1
    }

    // Skips entries whose sidebar page has since been removed, e.g. Bluetooth.
    function historyStep(direction) {
        sideBar.model.count // Re-evaluate bindings when sidebar pages change.
        for (var i = historyIndex + direction; i >= 0 && i < history.length; i += direction) {
            if (sideBarIndexOf(history[i].rootPage) >= 0)
                return i
        }
        return -1
    }

    function navigate(page, rootPage, properties) {
        var current = history[historyIndex]
        if (current && current.page === page && current.rootPage === rootPage
                && Object.keys(properties).length === 0)
            return
        history = history.slice(0, historyIndex + 1).concat([{
            page: page,
            rootPage: rootPage,
            properties: properties
        }])
        showHistoryEntry(history.length - 1)
    }

    // Opens a sub page of the current sidebar page.
    function pushPage(page, properties) {
        navigate(page, history[historyIndex].rootPage, properties || {})
    }

    function goBack() {
        var index = historyStep(-1)
        if (index >= 0)
            showHistoryEntry(index)
    }

    function goForward() {
        var index = historyStep(1)
        if (index >= 0)
            showHistoryEntry(index)
    }

    function showHistoryEntry(index) {
        var entry = history[index]
        historyIndex = index
        _stackView.replace(null, Qt.resolvedUrl(entry.page), entry.properties, FishUI.StackView.Immediate)

        var sideBarIndex = sideBarIndexOf(entry.rootPage)
        if (sideBar.currentIndex !== sideBarIndex) {
            syncingSideBar = true
            sideBar.currentIndex = sideBarIndex
            syncingSideBar = false
        }
    }

    function switchPageFromIndex(index) {
        if (index < 0 || index >= sideBar.model.count)
            return
        pendingPageName = ""
        var page = sideBar.model.get(index).page
        navigate(page, page, {})
    }

    function switchPageFromName(pageName) {
        pendingPageName = pageName === "bluetooth" ? pageName : ""
        for (var i = 0; i < sideBar.model.count; ++i) {
            if (pageName === sideBar.model.get(i).name) {
                switchPageFromIndex(i)
            }
        }

        // If the window is minimized, it needs to be displayed again.
        rootWindow.show()
        rootWindow.raise()
    }
}
