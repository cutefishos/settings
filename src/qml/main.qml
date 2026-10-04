// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import Cutefish.Settings
import FishUI as FishUI

FishUI.Window {
    id: rootWindow
    title: qsTr("Settings")
    // Shown once the sidebar's hardware entries have settled; see revealTimer.
    visible: false
    width: 900
    height: 610

    // Half of a 1280-wide screen, so the window can be split beside another.
    minimumWidth: 640
    minimumHeight: 600

    property alias stackView: _stackView
    property string pendingPageName: ""
    // An access point whose enterprise network the Wi-Fi page asks to join; see joinWifi().
    property string pendingWifiJoin: ""
    property bool revealed: false
    readonly property bool mockNetworkData: Qt.application.arguments.indexOf("--mock-network") >= 0

    // Entries are {page, rootPage, properties}; rootPage is the sidebar page a sub page belongs to.
    readonly property int maxHistoryLength: 50
    property var history: []
    property int historyIndex: -1
    property bool syncingSideBar: false
    readonly property bool canGoBack: historyStep(-1) >= 0
    readonly property bool canGoForward: historyStep(1) >= 0

    background.color: FishUI.Theme.backgroundColor
    // Inactive windows keep the blur and halve the see-through instead of going opaque.
    background.opacity: !blurAvailable ? 1
                        : FishUI.Theme.darkMode ? (active ? 0.85 : 0.92)
                                                : (active ? 0.8 : 0.9)
    header.height: headerInset
    contentTopMargin: 0

    // Room for the header buttons with as much space below them as above;
    // content starts under it and its frosted backdrop covers it.
    readonly property real headerInset: windowButtonsTopMargin * 2 + _headerRow.buttonSize

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
            onItemAdded: (name) => {
                if (rootWindow.pendingPageName === name)
                    rootWindow.switchPageFromName(name)
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

        // Pages run up under the window header, so its empty parts must not
        // reach them. Wheel events still pass through.
        MouseArea {
            anchors.left: sideBar.right
            anchors.right: parent.right
            anchors.top: parent.top
            height: rootWindow.headerInset
            acceptedButtons: Qt.AllButtons
            hoverEnabled: true
        }
    }

    headerItem: Item {
        RowLayout {
            id: _headerRow
            anchors.fill: parent
            anchors.leftMargin: sideBar.width + FishUI.Units.smallSpacing * 1.5
            anchors.rightMargin: FishUI.Units.smallSpacing
            spacing: rootWindow.windowButtonsSpacing

            readonly property int buttonSize: rootWindow.header.buttonSize
            // The title reads low on the sidebar title's baseline, so it sits
            // 1px above it; the buttons stay centred on that baseline's capitals.
            readonly property real titleRaise: 1
            readonly property real buttonTopMargin: _pageTitle.capCenter + titleRaise - buttonSize / 2

            NavigationButton {
                Layout.alignment: Qt.AlignTop
                Layout.topMargin: _headerRow.buttonTopMargin
                Layout.preferredWidth: _headerRow.buttonSize
                Layout.preferredHeight: _headerRow.buttonSize
                enabled: rootWindow.canGoBack
                iconName: rootWindow.LayoutMirroring.enabled ? "chevron-right" : "chevron-left"
                onClicked: rootWindow.goBack()
            }

            NavigationButton {
                Layout.alignment: Qt.AlignTop
                Layout.topMargin: _headerRow.buttonTopMargin
                Layout.preferredWidth: _headerRow.buttonSize
                Layout.preferredHeight: _headerRow.buttonSize
                enabled: rootWindow.canGoForward
                iconName: rootWindow.LayoutMirroring.enabled ? "chevron-left" : "chevron-right"
                onClicked: rootWindow.goForward()
            }

            FishUI.AlignedLabel {
                id: _pageTitle
                Layout.fillWidth: true
                Layout.leftMargin: FishUI.Units.smallSpacing
                // Header and sidebar share the window's top edge.
                textBaseline: sideBar.titleItem.y + sideBar.titleItem.baselineOffset - _headerRow.titleRaise
                leftPadding: 0
                elide: Text.ElideRight
                color: rootWindow.active ? FishUI.Theme.textColor : FishUI.Theme.disabledTextColor
                font.pointSize: Metrics.headerTitlePointSize
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

    // Entries that come with hardware are known only once their services answer; shown
    // before that, the first frames lack them and the entries below jump when they arrive.
    // A service that never answers holds the window no longer than this.
    Timer {
        id: revealTimer
        interval: 500
        running: true
        onTriggered: rootWindow.reveal()
    }

    Connections {
        target: sideBar
        function onReadyChanged() {
            if (sideBar.ready)
                Qt.callLater(rootWindow.reveal)
        }
    }

    function reveal() {
        if (revealed)
            return
        revealed = true
        revealTimer.stop()
        show()
        raise()
    }

    Shortcut {
        sequences: [StandardKey.Find]
        onActivated: sideBar.focusSearch()
    }

    // Deferred: sidebar items drop unavailable pages in their own onCompleted handlers.
    Component.onCompleted: Qt.callLater(function() {
        // The sidebar entries' own handlers run first, through Qt.callLater too.
        if (sideBar.ready)
            Qt.callLater(rootWindow.reveal)
        if (historyIndex < 0) {
            // A page asked for by name may still be waiting for its hardware entry.
            var pending = pendingPageName
            switchPageFromIndex(0)
            pendingPageName = pending
        }
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
        // A long session must not keep every step; the oldest go first.
        if (history.length > maxHistoryLength)
            history = history.slice(history.length - maxHistoryLength)
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

    function joinWifi(accessPoint) {
        pendingWifiJoin = accessPoint
        switchPageFromName("wlan")
    }

    function switchPageFromName(pageName) {
        // Hardware entries may join the sidebar after the request arrives.
        pendingPageName = pageName === "bluetooth" || pageName === "ethernet" ? pageName : ""
        for (var i = 0; i < sideBar.model.count; ++i) {
            if (pageName === sideBar.model.get(i).name) {
                switchPageFromIndex(i)
            }
        }

        // A sub page opens under its sidebar page, as if reached from there.
        sideBar.subPages.forEach(function(subPage) {
            if (subPage.name !== pageName)
                return
            for (var j = 0; j < sideBar.model.count; ++j) {
                var parentPage = sideBar.model.get(j)
                if (parentPage.name === subPage.parentName) {
                    pendingPageName = ""
                    navigate(subPage.page, parentPage.page, {})
                }
            }
        })

        // If the window is minimized, it needs to be displayed again.
        if (rootWindow.revealed) {
            rootWindow.show()
            rootWindow.raise()
        }
    }
}
