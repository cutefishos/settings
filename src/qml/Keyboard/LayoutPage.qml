import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import FishUI 1.0 as FishUI
import Cutefish.Settings 1.0
import "../"

ItemPage {
    id: page
    headerTitle: qsTr("Layout")

    Keyboard {
        id: keyboard
    }

    readonly property var filteredLayouts: {
        var query = searchField.text.trim().toLowerCase()
        if (query.length === 0)
            return keyboard.layouts
        return keyboard.layouts.filter(function(entry) {
            return entry.name.toLowerCase().indexOf(query) >= 0
                    || entry.id.toLowerCase().indexOf(query) >= 0
        })
    }

    FishUI.RoundedRectangle {
        anchors.fill: parent
        // The window header floats over the top of the page.
        anchors.topMargin: rootWindow.header.height + Metrics.pageTopMargin
        anchors.leftMargin: Metrics.pageMargin
        anchors.rightMargin: Metrics.pageMargin
        anchors.bottomMargin: Metrics.pageMargin
        color: FishUI.Theme.secondBackgroundColor
        radius: FishUI.Theme.bigRadius

        // Text fields are drawn for card surfaces; on the page background
        // they all but disappear.
        FishUI.TextField {
            id: searchField
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: Metrics.m
            placeholderText: qsTr("Search layouts")
            selectByMouse: true
            onTextChanged: layoutList.positionViewAtBeginning()
        }

        ListView {
            id: layoutList
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: searchField.bottom
            anchors.bottom: parent.bottom
            anchors.margins: Metrics.xs
            anchors.topMargin: Metrics.s
            clip: true
            model: page.filteredLayouts
            reuseItems: true
            boundsBehavior: Flickable.StopAtBounds
            FishUI.ScrollBar.vertical: FishUI.ScrollBar {}
            FishUI.WheelHandler { target: layoutList }

            delegate: ItemDelegate {
                id: row

                required property var modelData
                required property int index
                readonly property bool current: modelData.id === keyboard.layout

                width: ListView.view.width
                height: Metrics.rowHeight
                // The list is inset by Metrics.xs; this puts the text on the card's edge.
                leftPadding: Metrics.cardHorizontalPadding - Metrics.xs
                rightPadding: leftPadding

                Accessible.role: Accessible.RadioButton
                Accessible.name: modelData.name
                Accessible.checked: current

                background: FishUI.RoundedRectangle {
                    radius: FishUI.Theme.smallRadius
                    color: row.hovered ? Qt.alpha(FishUI.Theme.textColor, 0.05) : "transparent"
                }

                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    anchors.leftMargin: row.leftPadding
                    anchors.rightMargin: row.rightPadding
                    height: 1
                    color: FishUI.Theme.disabledTextColor
                    opacity: FishUI.Theme.darkMode ? 0.3 : 0.1
                    visible: row.index < layoutList.count - 1
                }

                contentItem: RowLayout {
                    spacing: Metrics.rowSpacing

                    FishUI.Label {
                        Layout.fillWidth: true
                        text: row.modelData.name
                        textFormat: Text.PlainText
                        color: FishUI.Theme.textColor
                        elide: Text.ElideRight
                    }

                    FishUI.Icon {
                        Layout.preferredWidth: 18
                        Layout.preferredHeight: 18
                        name: "check"
                        color: FishUI.Theme.highlightColor
                        visible: row.current
                    }
                }

                onClicked: keyboard.selectLayout(modelData.id)
            }

            FishUI.Label {
                anchors.centerIn: parent
                visible: layoutList.count === 0
                text: qsTr("No layouts found")
                color: FishUI.Theme.disabledTextColor
            }

            Component.onCompleted: {
                for (var i = 0; i < count; ++i) {
                    if (page.filteredLayouts[i].id === keyboard.layout) {
                        positionViewAtIndex(i, ListView.Center)
                        break
                    }
                }
            }
        }
    }
}
