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
import FishUI 1.0 as FishUI
import Cutefish.Settings 1.0
import Cutefish.Network 1.0 as NM
import Cutefish.Bluetooth 1.0 as Bluetooth

Item {
    id: sideBar
    implicitWidth: 230

    property int itemRadiusV: 8

    property alias view: listView
    property alias model: listModel
    property alias currentIndex: listView.currentIndex
    property bool updatingModel: false
    readonly property bool searching: searchField.text.trim().length > 0
    signal currentPageRemoved()
    signal itemAdded(string name)

    // Entries that come and go with the hardware, in sidebar order.
    readonly property var hardwareItems: [
        {
            title: qsTr("Ethernet"),
            name: "ethernet",
            page: "qrc:/qml/Wired/Main.qml",
            iconSource: "network.svg",
            iconColor: "#0067FF",
            category: qsTr("Network"),
            keywords: qsTr("wired, LAN, network, internet, cable, IP address")
        },
        {
            title: qsTr("Bluetooth"),
            name: "bluetooth",
            page: "qrc:/qml/Bluetooth/Main.qml",
            iconSource: "bluetooth.svg",
            iconColor: "#0067FF",
            category: qsTr("Network"),
            keywords: qsTr("wireless, devices, pairing, headphones")
        }
    ]

    // Pages opened from inside a sidebar page; search lists them after it.
    readonly property var subPages: [
        {
            title: qsTr("Fonts"),
            page: "qrc:/qml/Appearance/Fonts.qml",
            parentName: "appearance",
            iconSource: "fonts.svg",
            iconColor: "#FE8433",
            keywords: qsTr("general font, fixed font, font size, anti-aliasing, hinting")
        },
        {
            title: qsTr("Icon Theme"),
            page: "qrc:/qml/Appearance/IconTheme.qml",
            parentName: "appearance",
            iconSource: "icontheme.svg",
            iconColor: "#2EC347",
            keywords: qsTr("icons")
        },
        {
            title: qsTr("Cursor Style"),
            page: "qrc:/qml/Appearance/CursorTheme.qml",
            parentName: "appearance",
            iconSource: "cursor.svg",
            iconColor: "#3385FF",
            keywords: qsTr("cursor theme, cursor size, pointer")
        },
        {
            title: qsTr("Keyboard Layout"),
            page: "qrc:/qml/Keyboard/LayoutPage.qml",
            parentName: "keyboard",
            iconSource: "keyboard.svg",
            iconColor: "#5E6AD2",
            keywords: qsTr("input source")
        },
        {
            title: qsTr("Keyboard Shortcuts"),
            page: "qrc:/qml/Shortcuts/Main.qml",
            parentName: "keyboard",
            iconSource: "keyboard.svg",
            iconColor: "#5E6AD2",
            keywords: qsTr("hotkeys, key bindings")
        }
    ]

    Component.onCompleted: {
        syncEthernetItem()
        syncBluetoothItem()
    }

    Connections {
        target: Bluetooth.Manager
        function onAvailableChanged() {
            Qt.callLater(sideBar.syncBluetoothItem)
        }
    }

    // The Ethernet page lists only connections on a present wired device;
    // without one it would be blank.
    Repeater {
        id: wiredConnections
        model: NM.AppletProxyModel {
            type: NM.AppletProxyModel.WiredType
            sourceModel: rootWindow.mockNetworkData ? mockNetworkModel : networkModel
        }
        delegate: Item {}
        onCountChanged: Qt.callLater(sideBar.syncEthernetItem)
    }

    NM.NetworkModel {
        id: networkModel
    }

    NetworkMockModel {
        id: mockNetworkModel
    }

    // Separate functions: Qt.callLater folds calls to one function into one.
    function syncEthernetItem() {
        syncItem("ethernet", wiredConnections.count > 0)
    }

    function syncBluetoothItem() {
        syncItem("bluetooth", Bluetooth.Manager.available)
    }

    function indexOfItem(name) {
        for (var i = 0; i < listModel.count; ++i) {
            if (listModel.get(i).name === name)
                return i
        }
        return -1
    }

    function syncItem(name, available) {
        var itemIndex = indexOfItem(name)
        if (available === (itemIndex >= 0))
            return

        var selectedPage = listView.currentIndex >= 0
                ? listModel.get(listView.currentIndex).page : ""
        var selectedRemoved = itemIndex >= 0 && listView.currentIndex === itemIndex
        updatingModel = true
        if (available) {
            // After Wi-Fi and whichever hardware entries precede this one.
            var order = hardwareItems.map(function(item) { return item.name })
            var predecessors = ["wlan"].concat(order.slice(0, order.indexOf(name)))
            var insertIndex = 0
            while (insertIndex < listModel.count
                   && predecessors.indexOf(listModel.get(insertIndex).name) >= 0)
                ++insertIndex
            listModel.insert(insertIndex, hardwareItems[order.indexOf(name)])
        } else {
            listModel.remove(itemIndex)
        }
        if (selectedRemoved) {
            listView.currentIndex = 0
        } else {
            for (var j = 0; j < listModel.count; ++j) {
                if (listModel.get(j).page === selectedPage) {
                    listView.currentIndex = j
                    break
                }
            }
        }
        updatingModel = false
        if (selectedRemoved)
            currentPageRemoved()
        else if (available)
            itemAdded(name)
    }

    Rectangle {
        anchors.fill: parent
        color: FishUI.Theme.darkMode ? Qt.lighter(FishUI.Theme.backgroundColor, 1.5)
                                     : Qt.darker(FishUI.Theme.backgroundColor, 1.05)
        opacity: 0.7
    }

    // keywords: comma-separated search terms; a translation may add local synonyms.
    ListModel {
        id: listModel

        ListElement {
            title: qsTr("Wi-Fi")
            name: "wlan"
            page: "qrc:/qml/WLAN/Main.qml"
            iconSource: "qrc:/fishui/kit/images/icons/network-wireless-connected-100.svg"
            iconColor: "#0067FF"
            category: qsTr("Network")
            keywords: qsTr("wireless, WLAN, network, internet, password, hotspot, DNS, IP address")
        }

//        ListElement {
//            title: qsTr("Hotspot")
//            name: "hotspot"
//            page: "qrc:/qml/Hotspot/Main.qml"
//            iconSource: "hotspot.svg"
//            iconColor: "#0067FF"
//            category: qsTr("Network")
//        }

//        ListElement {
//            title: qsTr("VPN")
//            name: "vpn"
//            page: "qrc:/qml/VPN/Main.qml"
//            iconSource: "bluetooth.svg"
//            iconColor: "#0067FF"
//            category: qsTr("Network")
//        }

        ListElement {
            title: qsTr("Proxy")
            name: "proxy"
            page: "qrc:/qml/Proxy/Main.qml"
            iconSource: "proxy.svg"
            iconColor: "#0067FF"
            category: qsTr("Network")
            keywords: qsTr("HTTP, HTTPS, SOCKS, FTP, PAC, network")
        }

        ListElement {
            title: qsTr("Appearance")
            name: "appearance"
            page: "qrc:/qml/Appearance/Main.qml"
            iconSource: "appearance.svg"
            iconColor: "#03B4CB"
            category: qsTr("Desktop")
            keywords: qsTr("theme, dark mode, light mode, accent color, dim wallpaper")
        }

        ListElement {
            title: qsTr("Wallpaper")
            name: "background"
            page: "qrc:/qml/Wallpaper/Main.qml"
            iconSource: "wallpaper.svg"
            iconColor: "#34B4A7"
            category: qsTr("Desktop")
            keywords: qsTr("background, desktop picture, image")
        }

        ListElement {
            title: qsTr("Dock")
            name: "dock"
            page: "qrc:/qml/Dock/Main.qml"
            iconSource: "dock.svg"
            iconColor: "#8585FC"
            category: qsTr("Desktop")
            keywords: qsTr("taskbar, position, size, auto hide, glass effect")
        }

        ListElement {
            title: qsTr("Notifications")
            name: "notifications"
            page: "qrc:/qml/Notification/Main.qml"
            iconSource: "notifications.svg"
            iconColor: "#F16884"
            category: qsTr("Desktop")
            keywords: qsTr("do not disturb, banners, alerts")
        }

        ListElement {
            title: qsTr("Display")
            name: "display"
            page: "qrc:/qml/Display/Main.qml"
            iconSource: "display.svg"
            iconColor: "#0087ED"
            category: qsTr("Devices")
            keywords: qsTr("monitor, screen, resolution, refresh rate, scaling, brightness, rotation")
        }

        ListElement {
            title: qsTr("Sound")
            name: "sound"
            page: "qrc:/qml/Sound/Main.qml"
            iconSource: "sound.svg"
            iconColor: "#F16884"
            category: qsTr("Devices")
            keywords: qsTr("audio, volume, speakers, headphones, microphone, input, output")
        }

        ListElement {
            title: qsTr("Keyboard")
            name: "keyboard"
            page: "qrc:/qml/Keyboard/Main.qml"
            iconSource: "keyboard.svg"
            iconColor: "#5E6AD2"
            category: qsTr("Devices")
            keywords: qsTr("input method, key repeat, keyboard style")
        }

        ListElement {
            title: qsTr("Mouse")
            name: "mouse"
            page: "qrc:/qml/Cursor/Main.qml"
            iconSource: "cursor.svg"
            iconColor: "#3385FF"
            category: qsTr("Devices")
            keywords: qsTr("pointer speed, acceleration, natural scrolling, left-handed")
        }

        ListElement {
            title: qsTr("Touchpad")
            name: "touchpad"
            page: "qrc:/qml/Touchpad/Main.qml"
            iconSource: "touchpad.svg"
            iconColor: "#999999"
            category: qsTr("Devices")
            keywords: qsTr("trackpad, tap to click, natural scrolling, secondary click, pointer speed")
        }

        ListElement {
            title: qsTr("Touchscreen")
            name: "touchscreen"
            page: "qrc:/qml/Touchscreen/Main.qml"
            iconSource: "touchscreen.svg"
            iconColor: "#999999"
            category: qsTr("Devices")
            keywords: qsTr("touch, touch screen, touch panel")
        }

        ListElement {
            title: qsTr("Users")
            name: "accounts"
            page: "qrc:/qml/User/Main.qml"
            iconSource: "accounts.svg"
            iconColor: "#DA7C43"
            category: qsTr("System")
            keywords: qsTr("accounts, password, login, administrator, avatar")
        }

        ListElement {
            title: qsTr("Language")
            name: "language"
            page: "qrc:/qml/Language/LanguagePage.qml"
            iconSource: "language.svg"
            iconColor: "#20A7FF"
            category: qsTr("System")
            keywords: qsTr("region, locale, system language")
        }

        ListElement {
            title: qsTr("Date & Time")
            name: "datetime"
            page: "qrc:/qml/DateTime/Main.qml"
            iconSource: "datetime.svg"
            iconColor: "#418CFF"
            category: qsTr("System")
            keywords: qsTr("clock, time zone, 24-hour time, NTP, sync")
        }

        ListElement {
            title: qsTr("Default Applications")
            name: "defaultapps"
            page: "qrc:/qml/DefaultApp/Main.qml"
            iconSource: "defaultapps.svg"
            iconColor: "#418CFF"
            category: qsTr("System")
            keywords: qsTr("web browser, email client, terminal")
        }

//        ListElement {
//            title: qsTr("Application")
//            name: "application"
//            page: "qrc:/qml/Application/Main.qml"
//            iconSource: "accounts.svg"
//            iconColor: "#DA7C43"
//            category: qsTr("System")
//        }

        ListElement {
            title: qsTr("Battery")
            name: "battery"
            page: "qrc:/qml/Battery/Main.qml"
            iconSource: "battery.svg"
            iconColor: "#2EC347"
            category: qsTr("System")
            keywords: qsTr("charge, charging limit, battery health, capacity, history, percentage")
        }

        ListElement {
            title: qsTr("Power")
            name: "power"
            page: "qrc:/qml/Power/Main.qml"
            iconSource: "power.svg"
            iconColor: "#FE8433"
            category: qsTr("System")
            keywords: qsTr("energy, performance, power saving, sleep, turn off display")
        }

        ListElement {
            title: qsTr("About")
            name: "about"
            page: "qrc:/qml/About/Main.qml"
            iconSource: "about.svg"
            iconColor: "#24A7FD"
            category: qsTr("System")
            keywords: qsTr("system, version, kernel, processor, memory, storage, hardware")
        }
    }

    // Search results in sidebar order; each word of the query must match the
    // title, the category or a keyword.
    ListModel {
        id: resultsModel
    }

    Connections {
        target: listModel
        function onCountChanged() {
            if (sideBar.searching)
                Qt.callLater(sideBar.updateResults)
        }
    }

    // Back/Forward and hardware changes move the page without a click in the results.
    Connections {
        target: rootWindow
        function onHistoryIndexChanged() {
            if (sideBar.searching)
                resultsView.currentIndex = sideBar.resultIndexOfCurrentPage()
        }
    }

    // Updated in place: rebuilding the model recreates every row, and each
    // new row blanks its icon until it loads.
    function updateResults() {
        var words = searchField.text.toLowerCase().replace(/-/g, "").split(/[\s,&()]+/).filter(function(word) {
            return word.length > 0
        })
        // Latin words match from their start, so "fo" finds Fonts but not Performance.
        // Chinese, Japanese and Korean put no spaces between words, so they match anywhere.
        var isCjk = function(word) {
            for (var i = 0; i < word.length; ++i) {
                var code = word.charCodeAt(i)
                if ((code >= 0x3040 && code <= 0x30ff)          // Kana
                        || (code >= 0x3400 && code <= 0x9fff)   // CJK ideographs
                        || (code >= 0xac00 && code <= 0xd7af))  // Hangul
                    return true
            }
            return false
        }
        var matches = function(fields) {
            var text = fields.join(" ").toLowerCase().replace(/[,&()]/g, " ")
            text = " " + text.replace(/-/g, " ") + " " + text.replace(/-/g, "")
            return words.every(function(word) {
                var needle = isCjk(word) ? word : " " + word
                return text.indexOf(needle) >= 0
            })
        }

        var results = []
        for (var i = 0; i < listModel.count; ++i) {
            var item = listModel.get(i)
            if (matches([item.title, item.category, item.keywords])) {
                results.push({
                    title: item.title,
                    page: item.page,
                    rootPage: item.page,
                    iconSource: item.iconSource,
                    iconColor: item.iconColor,
                    category: item.category
                })
            }
            subPages.forEach(function(subPage) {
                if (subPage.parentName !== item.name
                        || !matches([subPage.title, item.title, item.category, subPage.keywords]))
                    return
                results.push({
                    title: subPage.title,
                    page: subPage.page,
                    rootPage: item.page,
                    iconSource: subPage.iconSource,
                    iconColor: subPage.iconColor,
                    category: item.category
                })
            })
        }

        // Both lists follow sidebar order, so one pass drops stale rows and inserts new ones.
        var pages = results.map(function(result) { return result.page })
        for (var row = 0; row < results.length; ++row) {
            while (row < resultsModel.count && pages.indexOf(resultsModel.get(row).page) < 0)
                resultsModel.remove(row)
            if (row >= resultsModel.count || resultsModel.get(row).page !== results[row].page)
                resultsModel.insert(row, results[row])
        }
        if (resultsModel.count > results.length)
            resultsModel.remove(results.length, resultsModel.count - results.length)

        var current = resultIndexOfCurrentPage()
        resultsView.currentIndex = current >= 0 ? current : (resultsModel.count > 0 ? 0 : -1)
    }

    function resultIndexOfCurrentPage() {
        var entry = rootWindow.history[rootWindow.historyIndex]
        if (!entry)
            return -1
        for (var i = 0; i < resultsModel.count; ++i) {
            if (resultsModel.get(i).page === entry.page)
                return i
        }
        return -1
    }

    function openRow(view, row) {
        if (view === resultsView) {
            var result = resultsModel.get(row)
            resultsView.currentIndex = row
            rootWindow.pendingPageName = ""
            rootWindow.navigate(result.page, result.rootPage, {})
        } else {
            listView.currentIndex = row
        }
    }

    function focusSearch() {
        searchField.forceActiveFocus()
        searchField.selectAll()
    }

    // A press anywhere outside the search field takes the focus off it. A PointHandler
    // only watches: it neither takes the press nor, without a cursorShape, the cursor.
    Item {
        parent: rootWindow.contentItem
        anchors.fill: parent
        z: 1000

        PointHandler {
            enabled: searchField.activeFocus
            acceptedButtons: Qt.AllButtons
            onActiveChanged: {
                if (active && !searchField.contains(searchField.mapFromItem(null, point.scenePressPosition)))
                    searchField.focus = false
            }
        }
    }

    Component {
        id: highlightComponent

        FishUI.RoundedRectangle {
            radius: FishUI.Theme.mediumRadius
            color: Qt.rgba(FishUI.Theme.textColor.r,
                           FishUI.Theme.textColor.g,
                           FishUI.Theme.textColor.b, 0.05)
            smooth: true
        }
    }

    Component {
        id: sectionDelegate

        Item {
            width: ListView.view.width - ListView.view.leftMargin - ListView.view.rightMargin
            // fontMetrics.height is fractional; unrounded, every row below a
            // header lands off the pixel grid and its icon tile blurs.
            height: Math.ceil(FishUI.Units.fontMetrics.height) + Metrics.sidebarCategorySpacing + Metrics.xs

            Text {
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.leftMargin: Qt.application.layoutDirection === Qt.RightToLeft ? 0 : Metrics.sidebarItemPadding
                anchors.rightMargin: Metrics.sidebarItemPadding
                anchors.topMargin: Metrics.sidebarCategorySpacing
                color: FishUI.Theme.disabledTextColor
                font.pointSize: 8
                text: section
            }
        }
    }

    Component {
        id: itemDelegate

        Item {
            id: item
            width: ListView.view.width - ListView.view.leftMargin - ListView.view.rightMargin
            height: Metrics.sidebarItemHeight

            property bool isCurrent: ListView.view.currentIndex === index

            Rectangle {
                anchors.fill: parent

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    acceptedButtons: Qt.LeftButton
                    onClicked: sideBar.openRow(item.ListView.view, index)
                }

                radius: FishUI.Theme.mediumRadius
                color: mouseArea.pressed ? Qt.rgba(FishUI.Theme.textColor.r,
                                                   FishUI.Theme.textColor.g,
                                                   FishUI.Theme.textColor.b, FishUI.Theme.darkMode ? 0.05 : 0.1) :
                       mouseArea.containsMouse || isCurrent ? Qt.rgba(FishUI.Theme.textColor.r,
                                                                      FishUI.Theme.textColor.g,
                                                                      FishUI.Theme.textColor.b, FishUI.Theme.darkMode ? 0.1 : 0.05) :
                                                              "transparent"

                smooth: true
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: Metrics.sidebarItemPadding
                spacing: Metrics.s

                Rectangle {
                    id: iconRect
                    width: 24
                    height: 24
                    Layout.alignment: Qt.AlignVCenter
                    radius: 6
                    color: model.iconColor

                    gradient: Gradient {
                        GradientStop { position: 0.0; color: Qt.lighter(model.iconColor, 1.15) }
                        GradientStop { position: 1.0; color: model.iconColor }
                    }

                    FishUI.IconItem {
                        id: icon
                        anchors.centerIn: parent
                        width: 16
                        height: width
                        source: model.iconSource.startsWith("qrc:") ? model.iconSource
                                                                    : "qrc:/images/sidebar/dark/" + model.iconSource
                        // The sidebar's own glyphs are white already; FishUI's shared ones need tinting.
                        color: "white"
                    }
                }

                FishUI.Label {
                    id: itemTitle
                    text: model.title
                    // 8pt text reads lighter than its colour, so the inactive grey leans towards textColor.
                    color: rootWindow.active ? FishUI.Theme.textColor
                                             : Qt.tint(FishUI.Theme.disabledTextColor, Qt.alpha(FishUI.Theme.textColor, 0.35))
                    font.pointSize: 8
                }

                Item {
                    Layout.fillWidth: true
                }
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 0
        spacing: 0

        FishUI.Label {
            text: rootWindow.title
            color: rootWindow.active ? FishUI.Theme.textColor : FishUI.Theme.disabledTextColor
            Layout.preferredHeight: rootWindow.header.height + Metrics.m
            leftPadding: Metrics.sidebarMargin + Metrics.sidebarItemPadding
            rightPadding: Metrics.sidebarMargin + Metrics.sidebarItemPadding
            topPadding: 0
            bottomPadding: 0
            verticalAlignment: Text.AlignVCenter
            font.pointSize: 13
        }

        // Typing lists matching pages in place of the sidebar; Up/Down move
        // through them, Enter opens one, Escape clears the search.
        FishUI.TextField {
            id: searchField

            readonly property bool mirrored: Qt.application.layoutDirection === Qt.RightToLeft
            readonly property real iconPadding: Metrics.s + searchIcon.width + Metrics.xs
            readonly property real clearPadding: clearButton.visible ? Metrics.xs + clearButton.width + Metrics.s
                                                                     : Metrics.s

            Layout.fillWidth: true
            Layout.preferredHeight: 28
            Layout.topMargin: Metrics.xs
            Layout.leftMargin: Metrics.sidebarMargin
            Layout.rightMargin: Metrics.sidebarMargin
            leftPadding: mirrored ? clearPadding : iconPadding
            rightPadding: mirrored ? iconPadding : clearPadding
            topPadding: 0
            bottomPadding: 0
            placeholderText: qsTr("Search")
            selectByMouse: true
            Accessible.name: qsTr("Search settings")

            // See-through so the field sits on the sidebar's frosted backdrop.
            background: FishUI.RoundedRectangle {
                // Without blur the backdrop is the flat sidebar tint, where the frosted fill washes out;
                // a neutral text-colour wash keeps the pill visible.
                readonly property color fill: rootWindow.active ? Qt.alpha(FishUI.Theme.alternateBackgroundColor, 0.6)
                                                                : Qt.alpha(FishUI.Theme.textColor, FishUI.Theme.darkMode ? 0.06 : 0.04)

                // Pill: radius is always half the height.
                radius: height / 2
                continuousCorners: false
                color: fill
                border.width: 1
                border.color: searchField.activeFocus ? FishUI.Theme.highlightColor : fill

                FishUI.RoundedRectangle {
                    anchors.fill: parent
                    anchors.margins: -2
                    // The field's radius; inset grows it into a concentric ring.
                    radius: parent.height / 2
                    continuousCorners: false
                    inset: -2
                    color: "transparent"
                    border.width: 2
                    border.color: FishUI.Theme.highlightColor
                    opacity: 0.25
                    visible: searchField.activeFocus
                }
            }

            FishUI.Icon {
                id: searchIcon
                anchors.left: parent.left
                anchors.leftMargin: Metrics.s
                anchors.verticalCenter: parent.verticalCenter
                width: 14
                height: 14
                name: "search"
                color: FishUI.Theme.disabledTextColor
            }

            FishUI.Icon {
                id: clearButton
                anchors.right: parent.right
                anchors.rightMargin: Metrics.s
                anchors.verticalCenter: parent.verticalCenter
                width: 14
                height: 14
                visible: searchField.length > 0
                name: "clear"
                color: FishUI.Theme.disabledTextColor

                MouseArea {
                    anchors.fill: parent
                    // Easier to hit than the glyph alone.
                    anchors.margins: -Metrics.xs
                    onClicked: searchField.clear()
                }
            }

            onTextChanged: {
                if (text.trim().length > 0)
                    sideBar.updateResults()
                else
                    resultsModel.clear()
            }
            onAccepted: {
                if (sideBar.searching && resultsView.currentIndex >= 0)
                    sideBar.openRow(resultsView, resultsView.currentIndex)
            }
            Keys.onDownPressed: {
                if (resultsView.count > 0)
                    resultsView.currentIndex = Math.min(resultsView.currentIndex + 1, resultsView.count - 1)
            }
            Keys.onUpPressed: {
                if (resultsView.count > 0)
                    resultsView.currentIndex = Math.max(resultsView.currentIndex - 1, 0)
            }
            Keys.onEscapePressed: {
                if (searchField.length > 0)
                    searchField.clear()
                else
                    searchField.focus = false
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ListView {
                id: listView
                anchors.fill: parent
                visible: !sideBar.searching
                clip: true
                model: listModel

                spacing: Metrics.sidebarItemSpacing
                leftMargin: Metrics.sidebarMargin
                rightMargin: Metrics.sidebarMargin
                topMargin: 0
                bottomMargin: Metrics.sidebarMargin

                FishUI.ScrollBar.vertical: FishUI.ScrollBar {}

                highlightFollowsCurrentItem: true
                highlightMoveDuration: 0
                highlightResizeDuration : 0
                highlight: highlightComponent

                section.property: "category"
                section.delegate: sectionDelegate

                FishUI.WheelHandler {
                    target: listView
                }

                // Rows joining or leaving the top of the list, such as hardware entries, move
                // originY but not contentY, which scrolls the first rows out of view.
                property real lastOriginY: 0
                onOriginYChanged: {
                    if (contentY <= lastOriginY - topMargin + 0.5)
                        contentY = originY - topMargin
                    lastOriginY = originY
                }

                delegate: itemDelegate
            }

            ListView {
                id: resultsView
                anchors.fill: parent
                visible: sideBar.searching
                clip: true
                model: resultsModel
                boundsBehavior: Flickable.StopAtBounds

                spacing: Metrics.sidebarItemSpacing
                leftMargin: Metrics.sidebarMargin
                rightMargin: Metrics.sidebarMargin
                topMargin: 0
                bottomMargin: Metrics.sidebarMargin

                FishUI.ScrollBar.vertical: FishUI.ScrollBar {}

                highlightFollowsCurrentItem: true
                highlightMoveDuration: 0
                highlightResizeDuration : 0
                highlight: highlightComponent
                onCurrentIndexChanged: positionViewAtIndex(currentIndex, ListView.Contain)

                section.property: "category"
                section.delegate: sectionDelegate

                FishUI.WheelHandler {
                    target: resultsView
                }

                delegate: itemDelegate
            }

            FishUI.Label {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.topMargin: Metrics.sidebarCategorySpacing
                visible: resultsView.visible && resultsView.count === 0
                text: qsTr("No Results")
                color: FishUI.Theme.disabledTextColor
            }
        }
    }

    function removeItem(name) {
        for (var i = 0; i < listModel.count; ++i) {
            if (name === listModel.get(i).name) {
                listModel.remove(i)
                break
            }
        }
    }

    NM.EnabledConnections {
        id: nmEnabledConnections

        Component.onCompleted: {
            if (!nmEnabledConnections.wirelessHwEnabled && !rootWindow.mockNetworkData)
                removeItem("wlan")
        }
    }

    Battery {
        id: _battery

        Component.onCompleted: {
            if (!_battery.available)
                removeItem("battery")
        }
    }

    Touchpad {
        id: _touchPad

        Component.onCompleted: {
            if (!_touchPad.available)
                removeItem("touchpad")
        }
    }

    Touchscreen {
        id: _touchscreen

        Component.onCompleted: {
            if (!_touchscreen.available)
                removeItem("touchscreen")
        }
    }
}
