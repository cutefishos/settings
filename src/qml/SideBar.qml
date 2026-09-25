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
            category: qsTr("Network")
        },
        {
            title: qsTr("Bluetooth"),
            name: "bluetooth",
            page: "qrc:/qml/Bluetooth/Main.qml",
            iconSource: "bluetooth.svg",
            iconColor: "#0067FF",
            category: qsTr("Network")
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

    ListModel {
        id: listModel

        ListElement {
            title: qsTr("Wi-Fi")
            name: "wlan"
            page: "qrc:/qml/WLAN/Main.qml"
            iconSource: "wlan.svg"
            iconColor: "#0067FF"
            category: qsTr("Network")
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
        }

        ListElement {
            title: qsTr("Appearance")
            name: "appearance"
            page: "qrc:/qml/Appearance/Main.qml"
            iconSource: "appearance.svg"
            iconColor: "#03B4CB"
            category: qsTr("Desktop")
        }

        ListElement {
            title: qsTr("Wallpaper")
            name: "background"
            page: "qrc:/qml/Wallpaper/Main.qml"
            iconSource: "wallpaper.svg"
            iconColor: "#34B4A7"
            category: qsTr("Desktop")
        }

        ListElement {
            title: qsTr("Dock")
            name: "dock"
            page: "qrc:/qml/Dock/Main.qml"
            iconSource: "dock.svg"
            iconColor: "#8585FC"
            category: qsTr("Desktop")
        }

        ListElement {
            title: qsTr("Notifications")
            name: "notifications"
            page: "qrc:/qml/Notification/Main.qml"
            iconSource: "notifications.svg"
            iconColor: "#F16884"
            category: qsTr("Desktop")
        }

        ListElement {
            title: qsTr("Display")
            name: "display"
            page: "qrc:/qml/Display/Main.qml"
            iconSource: "display.svg"
            iconColor: "#0087ED"
            category: qsTr("Devices")
        }

        ListElement {
            title: qsTr("Sound")
            name: "sound"
            page: "qrc:/qml/Sound/Main.qml"
            iconSource: "sound.svg"
            iconColor: "#F16884"
            category: qsTr("Devices")
        }

        ListElement {
            title: qsTr("Keyboard")
            name: "keyboard"
            page: "qrc:/qml/Keyboard/Main.qml"
            iconSource: "keyboard.svg"
            iconColor: "#5E6AD2"
            category: qsTr("Devices")
        }

        ListElement {
            title: qsTr("Mouse")
            name: "mouse"
            page: "qrc:/qml/Cursor/Main.qml"
            iconSource: "cursor.svg"
            iconColor: "#3385FF"
            category: qsTr("Devices")
        }

        ListElement {
            title: qsTr("Touchpad")
            name: "touchpad"
            page: "qrc:/qml/Touchpad/Main.qml"
            iconSource: "touchpad.svg"
            iconColor: "#999999"
            category: qsTr("Devices")
        }

        ListElement {
            title: qsTr("Users")
            name: "accounts"
            page: "qrc:/qml/User/Main.qml"
            iconSource: "accounts.svg"
            iconColor: "#DA7C43"
            category: qsTr("System")
        }

        ListElement {
            title: qsTr("Language")
            name: "language"
            page: "qrc:/qml/Language/LanguagePage.qml"
            iconSource: "language.svg"
            iconColor: "#20A7FF"
            category: qsTr("System")
        }

        ListElement {
            title: qsTr("Date & Time")
            name: "datetime"
            page: "qrc:/qml/DateTime/Main.qml"
            iconSource: "datetime.svg"
            iconColor: "#418CFF"
            category: qsTr("System")
        }

        ListElement {
            title: qsTr("Default Applications")
            name: "defaultapps"
            page: "qrc:/qml/DefaultApp/Main.qml"
            iconSource: "defaultapps.svg"
            iconColor: "#418CFF"
            category: qsTr("System")
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
            title: qsTr("Power")
            name: "power"
            page: "qrc:/qml/Power/Main.qml"
            iconSource: "power.svg"
            iconColor: "#FE8433"
            category: qsTr("System")
        }

        ListElement {
            title: qsTr("About")
            name: "about"
            page: "qrc:/qml/About/Main.qml"
            iconSource: "about.svg"
            iconColor: "#24A7FD"
            category: qsTr("System")
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 0
        spacing: 0

        FishUI.Label {
            text: rootWindow.title
            color: rootWindow.active ? FishUI.Theme.textColor : FishUI.Theme.disabledTextColor
            Layout.preferredHeight: rootWindow.header.height
            leftPadding: Metrics.sidebarMargin + Metrics.sidebarItemPadding
            rightPadding: Metrics.sidebarMargin + Metrics.sidebarItemPadding
            topPadding: rootWindow.windowButtonsTopMargin
            bottomPadding: 0
            font.pointSize: 13
        }

        ListView {
            id: listView
            Layout.fillHeight: true
            Layout.fillWidth: true
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
            highlight: FishUI.RoundedRectangle {
                radius: FishUI.Theme.mediumRadius
                color: Qt.rgba(FishUI.Theme.textColor.r,
                               FishUI.Theme.textColor.g,
                               FishUI.Theme.textColor.b, 0.05)
                smooth: true
            }

            section.property: "category"
            section.delegate: Item {
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

            delegate: Item {
                id: item
                width: ListView.view.width - ListView.view.leftMargin - ListView.view.rightMargin
                height: Metrics.sidebarItemHeight

                property bool isCurrent: listView.currentIndex === index

                Rectangle {
                    anchors.fill: parent

                    MouseArea {
                        id: mouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        acceptedButtons: Qt.LeftButton
                        onClicked: listView.currentIndex = index
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

                        Image {
                            id: icon
                            anchors.centerIn: parent
                            width: 16
                            height: width
                            source: "qrc:/images/sidebar/dark/" + model.iconSource
                            Layout.alignment: Qt.AlignVCenter
                            antialiasing: false
                            smooth: false
                        }
                    }

                    FishUI.Label {
                        id: itemTitle
                        text: model.title
                        color: FishUI.Theme.darkMode ? FishUI.Theme.textColor : "#363636"
                        font.pointSize: 8
                    }

                    Item {
                        Layout.fillWidth: true
                    }
                }
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

    Touchpad {
        id: _touchPad

        Component.onCompleted: {
            if (!_touchPad.available)
                removeItem("touchpad")
        }
    }
}
