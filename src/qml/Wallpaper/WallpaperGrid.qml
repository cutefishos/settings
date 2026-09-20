import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Qt5Compat.GraphicalEffects
import FishUI 1.0 as FishUI

ColumnLayout {
    id: control

    property string title
    property var wallpapers: []
    property string currentWallpaper
    property bool imageSelected: true
    property bool expanded: false
    property bool showAddWallpaper: false
    property bool removable: false
    readonly property var items: {
        var result = [];
        if (showAddWallpaper)
            result.push("");
        for (var index = 0; index < wallpapers.length; ++index)
            result.push(wallpapers[index]);
        return result;
    }

    signal wallpaperSelected(string path)
    signal addWallpaperRequested
    signal removeWallpaperRequested(string path)

    Layout.fillWidth: true
    spacing: FishUI.Units.largeSpacing

    RowLayout {
        Layout.fillWidth: true
        Layout.preferredHeight: 24

        Label {
            Layout.alignment: Qt.AlignVCenter
            text: control.title
            font.bold: true
        }

        Item {
            Layout.fillWidth: true
        }

        Button {
            id: showButton

            Layout.alignment: Qt.AlignVCenter
            visible: wallpaperView.count > wallpaperView.columnCount
            text: control.expanded ? qsTr("Show Less") : qsTr("Show All")
            hoverEnabled: true
            padding: 0

            background: Item {}
            contentItem: Label {
                text: showButton.text
                color: FishUI.Theme.highlightColor
                font.underline: showButton.hovered
                opacity: showButton.down ? 0.6 : 1.0
                verticalAlignment: Text.AlignVCenter
            }

            onClicked: control.expanded = !control.expanded
        }
    }

    GridView {
        id: wallpaperView

        readonly property int preferredThumbnailWidth: 160
        readonly property int minimumThumbnailWidth: 128
        readonly property int horizontalSpacing: FishUI.Units.largeSpacing
                                                 + FishUI.Units.smallSpacing
        readonly property int verticalSpacing: FishUI.Units.largeSpacing
        readonly property int baseColumnCount: Math.max(
                                                   Math.floor((width + horizontalSpacing)
                                                              / (preferredThumbnailWidth
                                                                 + horizontalSpacing)), 1)
        readonly property real additionalColumnWidth: (width
                                                       - baseColumnCount * horizontalSpacing)
                                                      / (baseColumnCount + 1)
        readonly property bool useAdditionalColumn: additionalColumnWidth
                                                    >= minimumThumbnailWidth
        readonly property int columnCount: baseColumnCount + (useAdditionalColumn ? 1 : 0)
        readonly property real thumbnailWidth: useAdditionalColumn
                                               ? additionalColumnWidth
                                               : Math.min(preferredThumbnailWidth, width)
        readonly property int thumbnailHeight: Math.round(thumbnailWidth * 0.625)
        readonly property int rowCount: control.expanded ? Math.ceil(count / columnCount) : Math.min(count, columnCount) > 0 ? 1 : 0

        Layout.fillWidth: true
        Layout.preferredHeight: Math.max(rowCount * cellHeight - verticalSpacing, 0)

        clip: true
        interactive: false
        model: control.items
        currentIndex: -1

        cellWidth: width / columnCount
        cellHeight: thumbnailHeight + verticalSpacing

        delegate: Item {
            id: wallpaperItem

            readonly property bool selected: control.imageSelected && modelData === control.currentWallpaper
            readonly property bool addWallpaper: control.showAddWallpaper && modelData === ""
            readonly property bool currentWallpaper: !addWallpaper && modelData === control.currentWallpaper
            readonly property int thumbnailInset: 4
            readonly property real thumbnailOffset: wallpaperView.columnCount > 1 ? (index % wallpaperView.columnCount) * (width - wallpaperView.thumbnailWidth) / (wallpaperView.columnCount - 1) : 0

            width: GridView.view.cellWidth
            height: GridView.view.cellHeight

            Rectangle {
                x: wallpaperItem.thumbnailOffset + wallpaperItem.thumbnailInset
                y: wallpaperItem.thumbnailInset
                width: wallpaperView.thumbnailWidth - wallpaperItem.thumbnailInset * 2
                height: wallpaperView.thumbnailHeight - wallpaperItem.thumbnailInset * 2
                radius: Math.max(FishUI.Theme.bigRadius - wallpaperItem.thumbnailInset, 0)
                color: FishUI.Theme.backgroundColor
                visible: !wallpaperItem.addWallpaper && wallpaperImage.status !== Image.Ready
            }

            FishUI.Icon {
                anchors.centerIn: thumbnail
                width: 32
                height: width
                name: "image"
                color: FishUI.Theme.darkMode ? "#FFFFFF" : "#C8CAD5"
                visible: !wallpaperItem.addWallpaper && wallpaperImage.status !== Image.Ready
            }

            Rectangle {
                x: wallpaperItem.thumbnailOffset + wallpaperItem.thumbnailInset
                y: wallpaperItem.thumbnailInset
                width: wallpaperView.thumbnailWidth - wallpaperItem.thumbnailInset * 2
                height: wallpaperView.thumbnailHeight - wallpaperItem.thumbnailInset * 2
                visible: wallpaperItem.addWallpaper
                radius: Math.max(FishUI.Theme.bigRadius - wallpaperItem.thumbnailInset, 0)
                color: FishUI.Theme.alternateBackgroundColor

                Column {
                    anchors.centerIn: parent
                    spacing: FishUI.Units.smallSpacing / 2

                    Item {
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: 18
                        height: 18

                        Rectangle {
                            anchors.centerIn: parent
                            width: 12
                            height: 1.5
                            radius: height / 2
                            color: FishUI.Theme.textColor
                        }

                        Rectangle {
                            anchors.centerIn: parent
                            width: 1.5
                            height: 12
                            radius: width / 2
                            color: FishUI.Theme.textColor
                        }
                    }

                    Label {
                        text: qsTr("Add Wallpaper")
                        color: FishUI.Theme.textColor
                        font.pointSize: Math.max(FishUI.Theme.fontSize - 1, 7)
                    }
                }

                MouseArea {
                    id: addMouseArea

                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: control.addWallpaperRequested()
                }
            }

            Rectangle {
                id: thumbnail

                x: wallpaperItem.thumbnailOffset
                width: wallpaperView.thumbnailWidth
                height: wallpaperView.thumbnailHeight
                visible: !wallpaperItem.addWallpaper
                color: "transparent"
                radius: FishUI.Theme.bigRadius

                Image {
                    id: wallpaperImage

                    anchors.fill: parent
                    anchors.margins: wallpaperItem.thumbnailInset
                    source: wallpaperItem.addWallpaper ? "" : "file://" + modelData
                    sourceSize: Qt.size(width, height)
                    fillMode: Image.PreserveAspectCrop
                    asynchronous: true
                    mipmap: true
                    cache: true
                    smooth: true

                    layer.enabled: true
                    layer.effect: OpacityMask {
                        maskSource: Item {
                            width: wallpaperImage.width
                            height: wallpaperImage.height

                            Rectangle {
                                anchors.fill: parent
                                radius: Math.max(FishUI.Theme.bigRadius - wallpaperItem.thumbnailInset, 0)
                            }
                        }
                    }
                }

                Rectangle {
                    anchors.fill: parent
                    color: "transparent"
                    radius: parent.radius
                    border.color: FishUI.Theme.highlightColor
                    border.width: wallpaperImage.status === Image.Ready && wallpaperItem.selected ? 2 : 0
                }

                HoverHandler {
                    id: thumbnailHover
                }

                MouseArea {
                    id: wallpaperMouseArea

                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton
                    hoverEnabled: true

                    onClicked: control.wallpaperSelected(modelData)
                }

                Rectangle {
                    anchors.top: parent.top
                    anchors.right: parent.right
                    anchors.margins: FishUI.Units.smallSpacing
                    width: 24
                    height: width
                    z: 2
                    visible: control.removable && !wallpaperItem.currentWallpaper && thumbnailHover.hovered
                    radius: width / 2
                    color: closeMouseArea.pressed ? Qt.rgba(0, 0, 0, 0.7) : Qt.rgba(0, 0, 0, 0.55)

                    Item {
                        anchors.centerIn: parent
                        width: 14
                        height: width

                        Rectangle {
                            anchors.centerIn: parent
                            width: 12
                            height: 1.5
                            radius: height / 2
                            rotation: 45
                            color: "#FFFFFF"
                        }

                        Rectangle {
                            anchors.centerIn: parent
                            width: 12
                            height: 1.5
                            radius: height / 2
                            rotation: -45
                            color: "#FFFFFF"
                        }
                    }

                    MouseArea {
                        id: closeMouseArea

                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: control.removeWallpaperRequested(modelData)
                    }
                }
            }
        }
    }
}
