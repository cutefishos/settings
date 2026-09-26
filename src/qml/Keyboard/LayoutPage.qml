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

    ColumnLayout {
        anchors.fill: parent
        // The window header floats over the top of the page.
        anchors.topMargin: rootWindow.headerInset + Metrics.pageTopMargin
        anchors.leftMargin: Metrics.pageMargin
        anchors.rightMargin: Metrics.pageMargin
        anchors.bottomMargin: Metrics.pageMargin
        spacing: Metrics.l

        SearchField {
            id: searchField
            placeholderText: qsTr("Search layouts")
            onTextChanged: layoutList.positionViewAtBeginning()
        }

        ListCard {
            Layout.fillHeight: true

            ListView {
                id: layoutList
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: page.filteredLayouts
                reuseItems: true
                boundsBehavior: Flickable.StopAtBounds
                FishUI.ScrollBar.vertical: FishUI.ScrollBar {}
                FishUI.WheelHandler { target: layoutList }

                delegate: CheckRow {
                    required property var modelData
                    required property int index

                    width: layoutList.width
                    viewIndex: index
                    text: modelData.name
                    checked: modelData.id === keyboard.layout
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
}
