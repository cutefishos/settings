/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     Reion Wong <reionwong@gmail.com>
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


import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Window 2.12
import QtQuick.Layouts 1.12
import Cutefish.Settings 1.0
import FishUI 1.0 as FishUI

FishUI.Window {
    id: control

    windowButtonsTopMargin: Metrics.dialogWindowButtonsMargin
    windowButtonsRightMargin: Metrics.dialogWindowButtonsMargin
    minimizeButtonVisible: false
    header.height: 48
    title: qsTr("Time Zone")
    width: 900
    height: 640
    minimumWidth: width
    minimumHeight: height
    maximumWidth: width
    maximumHeight: height
    visible: false
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint

    background.color: FishUI.Theme.secondBackgroundColor
    background.opacity: blurAvailable ? 0.5 : 1
    contentTopMargin: 0

    // The zone the pin and the Set button refer to.
    property string selectedZone: ""
    // Zones close enough to the last click to have been meant by it.
    property var nearbyZones: []
    // The zone a click would pick, previewed under the pointer.
    property string hoveredZone: ""

    // Bumped while open so the times shown keep up with the clock.
    property int tick: 0

    // Set by the user's own picks: opening the dialog puts the pin in place
    // without sliding it in from the corner.
    property bool animatePin: false

    // The difference in it is measured from the zone in use, so it is read
    // again when that changes, not only when the selection does.
    readonly property var selectedInfo: {
        control.tick
        timeZoneMap.currentTimeZoneId
        return control.selectedZone ? timeZoneMap.info(control.selectedZone) : ({})
    }
    readonly property bool changed: control.selectedZone !== ""
                                    && control.selectedZone !== timeZoneMap.currentTimeZoneId

    onVisibleChanged: {
        if (visible) {
            tick += 1
            animatePin = false
            searchField.clear()
            hoveredZone = ""
            selectZone(timeZoneMap.currentTimeZoneId)
            searchField.forceActiveFocus()
        } else {
            resultsPopup.close()
        }
    }

    function selectZone(zone) {
        if (!zone)
            return
        timeZoneMap.select(zone)
    }

    function pickResult(index) {
        if (index < 0 || index >= resultsView.count)
            return
        var zone = resultsView.model[index].id
        animatePin = true
        resultsPopup.close()
        searchField.text = ""
        selectZone(zone)
    }

    Timer {
        interval: 15000
        repeat: true
        running: control.visible
        onTriggered: control.tick += 1
    }

    Connections {
        target: timeZoneMap

        function onAvailableListChanged() {
            var zones = timeZoneMap.availableList
            control.nearbyZones = zones
            control.selectedZone = zones.length > 0 ? zones[0] : ""
        }
    }

    Item {
        z: -1
        anchors.fill: parent

        DragHandler {
            acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
            grabPermissions: PointerHandler.CanTakeOverFromItems | PointerHandler.CanTakeOverFromHandlersOfDifferentType | PointerHandler.ApprovesTakeOverByAnything
            onActiveChanged: if (active) control.helper.startSystemMove(control)
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Metrics.dialogPadding
        anchors.topMargin: 0
        spacing: Metrics.dialogSpacing

        // The title sits at the left and the search box in the middle of the
        // window, not in the middle of what the title leaves free.
        Item {
            // Shares the title bar's band, centred on the close button.
            Layout.fillWidth: true
            Layout.preferredHeight: control.header.height
            Layout.bottomMargin: Metrics.s - Metrics.dialogSpacing

            FishUI.Label {
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                text: control.title
                textFormat: Text.PlainText
                font.pixelSize: 14
                font.weight: Font.DemiBold
                color: FishUI.Theme.textColor
            }

            // Typing lists matching zones; Up/Down move through them, Enter picks.
            FishUI.TextField {
                id: searchField
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                width: 280
                height: 30
                leftPadding: searchIcon.x + searchIcon.width + Metrics.s
                rightPadding: Metrics.m
                topPadding: 0
                bottomPadding: 0
                placeholderText: qsTr("Search city or country")
                selectByMouse: true

                // FishUI's field in a pill, its fill see-through so it sits
                // lightly on the window; the text stays fully opaque.
                background: FishUI.RoundedRectangle {
                    readonly property color fill: Qt.alpha(FishUI.Theme.alternateBackgroundColor, 0.6)

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
                    x: Metrics.m
                    anchors.verticalCenter: parent.verticalCenter
                    width: 14
                    height: 14
                    name: "search"
                    color: FishUI.Theme.disabledTextColor
                }

                onTextChanged: {
                    resultsView.model = timeZoneMap.search(text)
                    resultsView.currentIndex = resultsView.count > 0 ? 0 : -1
                    if (text.trim().length > 0)
                        resultsPopup.open()
                    else
                        resultsPopup.close()
                }
                onAccepted: control.pickResult(resultsView.currentIndex)
                Keys.onDownPressed: {
                    if (resultsView.count > 0)
                        resultsView.currentIndex = (resultsView.currentIndex + 1) % resultsView.count
                }
                Keys.onUpPressed: {
                    if (resultsView.count > 0)
                        resultsView.currentIndex = (resultsView.currentIndex - 1 + resultsView.count) % resultsView.count
                }
                Keys.onEscapePressed: function(event) {
                    if (resultsPopup.opened) {
                        resultsPopup.close()
                        event.accepted = true
                    }
                }

                Popup {
                    id: resultsPopup
                    // Wider than the field: each result carries a time and a day.
                    x: (searchField.width - width) / 2
                    y: searchField.height + Metrics.xs
                    width: 360
                    height: resultsView.count > 0 ? Math.min(resultsView.contentHeight + padding * 2, 360)
                                                  : emptyLabel.implicitHeight + Metrics.l * 2
                    padding: Metrics.xs
                    focus: false
                    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

                    background: FishUI.RoundedRectangle {
                        radius: FishUI.Theme.mediumRadius
                        color: FishUI.Theme.secondBackgroundColor
                        border.width: 1
                        border.color: Qt.rgba(FishUI.Theme.textColor.r, FishUI.Theme.textColor.g,
                                              FishUI.Theme.textColor.b, 0.12)
                    }

                    FishUI.Label {
                        id: emptyLabel
                        anchors.centerIn: parent
                        visible: resultsView.count === 0
                        text: qsTr("No matching time zones")
                        color: FishUI.Theme.disabledTextColor
                    }

                    ListView {
                        id: resultsView
                        anchors.fill: parent
                        clip: true
                        boundsBehavior: Flickable.StopAtBounds
                        highlightMoveDuration: 0
                        onCurrentIndexChanged: positionViewAtIndex(currentIndex, ListView.Contain)
                        FishUI.ScrollBar.vertical: FishUI.ScrollBar {}
                        FishUI.WheelHandler { target: resultsView }

                        delegate: Item {
                            width: ListView.view.width
                            height: Metrics.rowHeight + Metrics.xs
                            readonly property bool current: ListView.isCurrentItem

                            FishUI.RoundedRectangle {
                                anchors.fill: parent
                                radius: FishUI.Theme.smallRadius
                                color: current ? FishUI.Theme.highlightColor
                                     : resultArea.containsMouse
                                       ? Qt.rgba(FishUI.Theme.textColor.r, FishUI.Theme.textColor.g,
                                                 FishUI.Theme.textColor.b, 0.08)
                                       : "transparent"
                            }

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: Metrics.m
                                anchors.rightMargin: Metrics.m
                                spacing: Metrics.rowSpacing

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 0

                                    FishUI.Label {
                                        Layout.fillWidth: true
                                        text: modelData.name
                                        textFormat: Text.PlainText
                                        color: current ? FishUI.Theme.highlightedTextColor : FishUI.Theme.textColor
                                        elide: Text.ElideRight
                                    }

                                    FishUI.Label {
                                        Layout.fillWidth: true
                                        text: modelData.region
                                        textFormat: Text.PlainText
                                        color: current ? FishUI.Theme.highlightedTextColor : FishUI.Theme.disabledTextColor
                                        opacity: current ? 0.8 : 1
                                        elide: Text.ElideRight
                                        font.pointSize: 8
                                    }
                                }

                                ColumnLayout {
                                    spacing: 0

                                    FishUI.Label {
                                        Layout.alignment: Qt.AlignRight
                                        text: modelData.time
                                        color: current ? FishUI.Theme.highlightedTextColor : FishUI.Theme.textColor
                                        font.features: { "tnum": 1 }
                                    }

                                    FishUI.Label {
                                        Layout.alignment: Qt.AlignRight
                                        text: [modelData.day, modelData.difference].filter(Boolean).join(" · ")
                                        color: current ? FishUI.Theme.highlightedTextColor : FishUI.Theme.disabledTextColor
                                        opacity: current ? 0.8 : 1
                                        font.pointSize: 8
                                    }
                                }
                            }

                            MouseArea {
                                id: resultArea
                                anchors.fill: parent
                                hoverEnabled: true
                                onClicked: control.pickResult(index)
                            }
                        }
                    }
                }
            }
        }

        FishUI.RoundedRectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: FishUI.Theme.bigRadius
            color: FishUI.Theme.darkMode ? Qt.rgba(1, 1, 1, 0.04) : Qt.rgba(0, 0, 0, 0.03)

            Item {
                id: mapArea
                anchors.fill: parent
                anchors.margins: Metrics.m
                clip: true

                // The rectangle the map is actually painted in: the aspect-fit
                // image leaves bars on two sides.
                readonly property real mapWidth: _worldMap.paintedWidth
                readonly property real mapHeight: _worldMap.paintedHeight
                readonly property real mapX: (width - mapWidth) / 2
                readonly property real mapY: (height - mapHeight) / 2

                function pointOf(zone) {
                    if (!zone || mapWidth <= 0)
                        return Qt.point(-1, -1)
                    var point = timeZoneMap.position(zone, mapWidth, mapHeight)
                    if (point.x < 0)
                        return point
                    return Qt.point(mapX + point.x, mapY + point.y)
                }

                readonly property point selectedPoint: pointOf(control.selectedZone)
                readonly property point hoveredPoint: pointOf(control.hoveredZone)

                // Land in the text colour and borders in the card's, both toned
                // down so the pins and the dots stay what the eye finds first.
                Image {
                    id: _worldMap
                    anchors.fill: parent
                    source: FishUI.Theme.darkMode ? "qrc:/images/dark/world.svg" : "qrc:/images/light/world.svg"
                    sourceSize: Qt.size(width, height)
                    fillMode: Image.PreserveAspectFit
                    opacity: FishUI.Theme.darkMode ? 0.24 : 0.18
                }

                // Every zone as a dot, so it is clear where a click lands; the
                // ones keeping the selected zone's clock right now stand out.
                Item {
                    id: dots
                    x: mapArea.mapX
                    y: mapArea.mapY
                    width: mapArea.mapWidth
                    height: mapArea.mapHeight

                    Repeater {
                        // Placed by fraction, so the map settling its size on
                        // opening moves the dots instead of building them again.
                        model: control.visible ? timeZoneMap.points() : []

                        Rectangle {
                            readonly property bool sameClock: control.selectedInfo.offsetSeconds === modelData.offsetSeconds
                            z: sameClock ? 1 : 0
                            x: modelData.x * dots.width - width / 2
                            y: modelData.y * dots.height - height / 2
                            width: sameClock ? 5 : 3
                            height: width
                            radius: width / 2
                            color: sameClock ? FishUI.Theme.highlightColor : FishUI.Theme.textColor
                            opacity: sameClock ? 0.55 : 0.18
                        }
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor

                    onPositionChanged: function(mouse) {
                        control.hoveredZone = timeZoneMap.nearest(mouse.x - mapArea.mapX, mouse.y - mapArea.mapY,
                                                                  mapArea.mapWidth, mapArea.mapHeight)
                    }
                    onExited: control.hoveredZone = ""
                    onClicked: function(mouse) {
                        control.animatePin = true
                        resultsPopup.close()
                        timeZoneMap.clicked(mouse.x - mapArea.mapX, mouse.y - mapArea.mapY,
                                            mapArea.mapWidth, mapArea.mapHeight)
                    }
                }

                // What a click here would pick, above the pin and its card so a
                // zone right next to the selection can still be seen.
                Item {
                    id: hoverMarker
                    z: 1
                    visible: control.hoveredZone !== "" && control.hoveredZone !== control.selectedZone
                             && mapArea.hoveredPoint.x >= 0

                    FishUI.RoundedRectangle {
                        x: mapArea.hoveredPoint.x - width / 2
                        y: mapArea.hoveredPoint.y - height / 2
                        width: 12
                        height: 12
                        radius: width / 2
                        continuousCorners: false
                        color: "transparent"
                        border.width: 2
                        border.color: FishUI.Theme.highlightColor
                        opacity: 0.8
                    }

                    FishUI.RoundedRectangle {
                        readonly property bool flip: mapArea.hoveredPoint.x + Metrics.s + width > mapArea.width
                        x: flip ? mapArea.hoveredPoint.x - Metrics.s - width : mapArea.hoveredPoint.x + Metrics.s
                        y: Math.max(0, mapArea.hoveredPoint.y - height - Metrics.xs)
                        width: hoverLabel.implicitWidth + Metrics.m
                        height: hoverLabel.implicitHeight + Metrics.xs
                        radius: height / 2
                        color: FishUI.Theme.darkMode ? Qt.rgba(0, 0, 0, 0.6) : Qt.rgba(1, 1, 1, 0.9)

                        FishUI.Label {
                            id: hoverLabel
                            anchors.centerIn: parent
                            text: control.hoveredZone ? timeZoneMap.localeTimeZoneName(control.hoveredZone) : ""
                            textFormat: Text.PlainText
                            font.pointSize: 8
                            color: FishUI.Theme.textColor
                        }
                    }
                }

                // The selected zone: a pin with a slow ripple, and a card
                // naming it.
                Item {
                    id: pin
                    visible: mapArea.selectedPoint.x >= 0
                    x: mapArea.selectedPoint.x
                    y: mapArea.selectedPoint.y

                    Behavior on x {
                        enabled: control.animatePin
                        NumberAnimation { duration: 250; easing.type: Easing.OutCubic }
                    }
                    Behavior on y {
                        enabled: control.animatePin
                        NumberAnimation { duration: 250; easing.type: Easing.OutCubic }
                    }

                    Rectangle {
                        id: ripple
                        anchors.centerIn: parent
                        width: 14
                        height: 14
                        radius: width / 2
                        color: FishUI.Theme.highlightColor

                        opacity: 0

                        // A few rings for each new selection, then still: a
                        // looping ripple would redraw the window every frame.
                        SequentialAnimation {
                            id: rippleAnimation
                            loops: 3

                            ParallelAnimation {
                                NumberAnimation { target: ripple; property: "scale"; from: 1; to: 3.2; duration: 1600; easing.type: Easing.OutCubic }
                                NumberAnimation { target: ripple; property: "opacity"; from: 0.45; to: 0; duration: 1600; easing.type: Easing.OutCubic }
                            }
                            PauseAnimation { duration: 400 }
                        }

                        Connections {
                            target: control

                            function onSelectedZoneChanged() { rippleAnimation.restart() }
                            function onVisibleChanged() {
                                if (control.visible)
                                    rippleAnimation.restart()
                                else
                                    rippleAnimation.stop()
                            }
                        }
                    }

                    FishUI.RoundedRectangle {
                        anchors.centerIn: parent
                        width: 14
                        height: 14
                        radius: width / 2
                        continuousCorners: false
                        color: FishUI.Theme.highlightColor
                        border.width: 2
                        border.color: "white"
                    }

                    FishUI.RoundedRectangle {
                        id: callout
                        // Right of the pin and above it, turned back when that
                        // would leave the map.
                        readonly property bool flipX: pin.x + Metrics.m + width > mapArea.width
                        readonly property bool flipY: pin.y - Metrics.m - height < 0
                        x: flipX ? -Metrics.m - width : Metrics.m
                        y: flipY ? Metrics.m : -Metrics.m - height
                        width: calloutLayout.implicitWidth + Metrics.m * 2
                        height: calloutLayout.implicitHeight + Metrics.s * 2
                        radius: FishUI.Theme.mediumRadius
                        color: FishUI.Theme.secondBackgroundColor
                        border.width: 1
                        border.color: Qt.rgba(FishUI.Theme.textColor.r, FishUI.Theme.textColor.g,
                                              FishUI.Theme.textColor.b, 0.1)

                        // Keeps clicks and hovers on the card from reaching the
                        // map underneath and picking whatever zone it covers.
                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            onEntered: control.hoveredZone = ""
                        }

                        ColumnLayout {
                            id: calloutLayout
                            anchors.centerIn: parent
                            spacing: 0

                            FishUI.Label {
                                text: control.selectedInfo.name || ""
                                textFormat: Text.PlainText
                                font.weight: Font.DemiBold
                                color: FishUI.Theme.textColor
                            }

                            FishUI.Label {
                                visible: text.length > 0 && text !== control.selectedInfo.name
                                text: control.selectedInfo.region || ""
                                textFormat: Text.PlainText
                                font.pointSize: 8
                                color: FishUI.Theme.disabledTextColor
                            }

                            // More than one zone lies under the click: the card
                            // offers them all, floating, so nothing below moves.
                            Row {
                                Layout.topMargin: Metrics.s
                                visible: control.nearbyZones.length > 1
                                spacing: Metrics.xs

                                Repeater {
                                    model: control.nearbyZones

                                    FishUI.RoundedRectangle {
                                        readonly property bool current: modelData === control.selectedZone
                                        width: nearbyLabel.implicitWidth + Metrics.m * 2
                                        height: nearbyLabel.implicitHeight + Metrics.xs * 2
                                        radius: height / 2
                                        color: current ? FishUI.Theme.highlightColor
                                             : nearbyArea.containsMouse
                                               ? Qt.rgba(FishUI.Theme.textColor.r, FishUI.Theme.textColor.g,
                                                         FishUI.Theme.textColor.b, 0.12)
                                               : Qt.rgba(FishUI.Theme.textColor.r, FishUI.Theme.textColor.g,
                                                         FishUI.Theme.textColor.b, 0.06)

                                        FishUI.Label {
                                            id: nearbyLabel
                                            anchors.centerIn: parent
                                            text: timeZoneMap.localeTimeZoneName(modelData)
                                            textFormat: Text.PlainText
                                            font.pointSize: 8
                                            color: parent.current ? FishUI.Theme.highlightedTextColor : FishUI.Theme.textColor
                                        }

                                        MouseArea {
                                            id: nearbyArea
                                            anchors.fill: parent
                                            hoverEnabled: true
                                            onClicked: {
                                                control.animatePin = true
                                                control.selectedZone = modelData
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: Metrics.l

            // The selection spelled out: its clock, where it is, and how it
            // differs from the zone in use. Always two lines, so the bar never
            // changes height.
            FishUI.Label {
                Layout.alignment: Qt.AlignVCenter
                text: control.selectedInfo.time || ""
                font.pixelSize: 26
                font.weight: Font.Light
                font.features: { "tnum": 1 }
                color: FishUI.Theme.textColor
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                spacing: Metrics.xxs

                FishUI.Label {
                    Layout.fillWidth: true
                    text: {
                        var info = control.selectedInfo
                        if (!info.name)
                            return qsTr("Click the map or search to choose a time zone")
                        return info.region && info.region !== info.name ? info.name + ", " + info.region : info.name
                    }
                    textFormat: Text.PlainText
                    elide: Text.ElideRight
                    font.weight: control.selectedInfo.name ? Font.DemiBold : Font.Normal
                    color: control.selectedInfo.name ? FishUI.Theme.textColor : FishUI.Theme.disabledTextColor
                }

                FishUI.Label {
                    Layout.fillWidth: true
                    text: {
                        if (!control.selectedInfo.name)
                            return ""
                        if (!control.changed)
                            return qsTr("Current time zone")
                        var info = control.selectedInfo
                        return [info.day, info.difference, qsTr("Currently %1").arg(timeZoneMap.currentTimeZone)]
                                .filter(Boolean).join("  ·  ")
                    }
                    textFormat: Text.PlainText
                    elide: Text.ElideRight
                    font.pointSize: 8
                    color: FishUI.Theme.disabledTextColor
                }
            }

            RowLayout {
                Layout.alignment: Qt.AlignVCenter
                spacing: Metrics.dialogButtonSpacing

                FishUI.ConfirmDialogButton {
                    text: qsTr("Cancel")
                    onClicked: control.close()
                }

                FishUI.ConfirmDialogButton {
                    text: qsTr("Set")
                    primary: true
                    // Only a change is worth applying.
                    enabled: control.changed
                    onClicked: {
                        timeZoneMap.setTimeZone(control.selectedZone)
                        control.close()
                    }
                }
            }
        }
    }
}
