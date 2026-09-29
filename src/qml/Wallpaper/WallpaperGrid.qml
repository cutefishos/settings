import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Cutefish.Settings 1.0
import FishUI 1.0 as FishUI
import "../"

ColumnLayout {
    id: control

    property string title
    // A FishUI.Icon glyph drawn before the title, for sections that stand for
    // something other than the wallpapers themselves.
    property string iconName
    property bool closable: false
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
    signal closeRequested

    Layout.fillWidth: true
    spacing: Metrics.cardSpacing

    RowLayout {
        Layout.fillWidth: true
        Layout.preferredHeight: 24
        spacing: Metrics.controlSpacing

        RowLayout {
            Layout.alignment: Qt.AlignVCenter
            Layout.fillHeight: true
            spacing: Metrics.controlSpacing

            HoverHandler {
                id: sectionHover

                enabled: control.closable
            }

            FishUI.Icon {
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredWidth: 16
                Layout.preferredHeight: 16
                visible: control.iconName !== ""
                name: control.iconName
                color: FishUI.Theme.textColor
            }

            FishUI.Label {
                id: titleLabel

                Layout.alignment: Qt.AlignVCenter
                text: control.title
                elide: Text.ElideRight
            }

            FishUI.Label {
                Layout.alignment: Qt.AlignVCenter
                visible: control.wallpapers.length > 0
                text: "(" + control.wallpapers.length + ")"
                color: FishUI.Theme.disabledTextColor
                font.pointSize: Math.max(titleLabel.font.pointSize - 1, 1)
            }

            Item {
                Layout.alignment: Qt.AlignVCenter
                implicitWidth: 20
                implicitHeight: 20
                // Stays laid out while hidden so the row height and the
                // centred title do not move when the button appears.
                visible: control.closable
                opacity: sectionHover.hovered ? 1.0 : 0.0

                Rectangle {
                    anchors.fill: parent
                    radius: width / 2
                    color: FishUI.Theme.textColor
                    opacity: closeSectionMouseArea.pressed ? 0.2 : closeSectionMouseArea.containsMouse ? 0.1 : 0.0
                }

                Item {
                    anchors.centerIn: parent
                    width: 10
                    height: width

                    Rectangle {
                        anchors.centerIn: parent
                        width: 10
                        height: 1.5
                        radius: height / 2
                        rotation: 45
                        color: FishUI.Theme.textColor
                    }

                    Rectangle {
                        anchors.centerIn: parent
                        width: 10
                        height: 1.5
                        radius: height / 2
                        rotation: -45
                        color: FishUI.Theme.textColor
                    }
                }

                MouseArea {
                    id: closeSectionMouseArea

                    anchors.fill: parent
                    enabled: sectionHover.hovered
                    hoverEnabled: true
                    onClicked: control.closeRequested()
                }
            }
        }

        Item {
            Layout.fillWidth: true
        }

        FishUI.Button {
            id: showButton

            Layout.alignment: Qt.AlignVCenter
            visible: wallpaperView.count > wallpaperView.columnCount
            text: control.expanded ? qsTr("Show Less") : qsTr("Show All")
            // Button sets its side paddings itself; padding alone would leave them.
            padding: 0
            leftPadding: 0
            rightPadding: 0

            background: Item {}
            contentItem: FishUI.Label {
                text: showButton.text
                color: FishUI.Theme.highlightColor
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
        readonly property int horizontalSpacing: Metrics.tileSpacing
        readonly property int verticalSpacing: Metrics.tileSpacing
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
        // The thumbnail width follows the window but never exceeds the preferred
        // width; a source size bound to it re-requests every image on each resize
        // step. Qt applies the device pixel ratio to it before asking the provider.
        readonly property size thumbnailSourceSize: Qt.size(preferredThumbnailWidth,
                                                            Math.round(preferredThumbnailWidth * 0.625))
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
            readonly property int thumbnailInset: 4
            readonly property real thumbnailOffset: wallpaperView.columnCount > 1 ? (index % wallpaperView.columnCount) * (width - wallpaperView.thumbnailWidth) / (wallpaperView.columnCount - 1) : 0

            width: GridView.view.cellWidth
            height: GridView.view.cellHeight

            FishUI.RoundedRectangle {
                x: wallpaperItem.thumbnailOffset + wallpaperItem.thumbnailInset
                y: wallpaperItem.thumbnailInset
                width: wallpaperView.thumbnailWidth - wallpaperItem.thumbnailInset * 2
                height: wallpaperView.thumbnailHeight - wallpaperItem.thumbnailInset * 2
                radius: FishUI.Theme.bigRadius
                inset: wallpaperItem.thumbnailInset
                color: FishUI.Theme.backgroundColor
                visible: !wallpaperItem.addWallpaper && thumbnail.imageStatus !== Image.Ready
            }

            FishUI.Icon {
                anchors.centerIn: thumbnail
                width: 32
                height: width
                name: "image"
                color: FishUI.Theme.darkMode ? "#FFFFFF" : "#C8CAD5"
                visible: !wallpaperItem.addWallpaper && thumbnail.imageStatus !== Image.Ready
            }

            FishUI.RoundedRectangle {
                x: wallpaperItem.thumbnailOffset + wallpaperItem.thumbnailInset
                y: wallpaperItem.thumbnailInset
                width: wallpaperView.thumbnailWidth - wallpaperItem.thumbnailInset * 2
                height: wallpaperView.thumbnailHeight - wallpaperItem.thumbnailInset * 2
                visible: wallpaperItem.addWallpaper
                radius: FishUI.Theme.bigRadius
                inset: wallpaperItem.thumbnailInset
                color: FishUI.Theme.alternateBackgroundColor

                Column {
                    anchors.centerIn: parent
                    spacing: Metrics.xs

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

                    FishUI.Label {
                        text: qsTr("Add Wallpaper")
                        color: FishUI.Theme.textColor
                        font.pointSize: Math.max(FishUI.Theme.fontSize - 1, 7)
                    }
                }

                MouseArea {
                    id: addMouseArea

                    anchors.fill: parent
                    onClicked: control.addWallpaperRequested()
                }
            }

            ImageSelectionItem {
                id: thumbnail

                x: wallpaperItem.thumbnailOffset
                width: wallpaperView.thumbnailWidth
                height: wallpaperView.thumbnailHeight
                visible: !wallpaperItem.addWallpaper
                previewWidth: width
                previewHeight: height
                previewInset: wallpaperItem.thumbnailInset
                source: wallpaperItem.addWallpaper
                        ? ""
                        : "image://thumbnailer/" + Qt.resolvedUrl("file://" + modelData)
                sourceSize: wallpaperView.thumbnailSourceSize
                fillMode: Image.PreserveAspectCrop
                asynchronous: true
                mipmap: true
                cache: true
                smooth: true
                checked: imageStatus === Image.Ready && wallpaperItem.selected
                onClicked: control.wallpaperSelected(modelData)

                HoverHandler {
                    id: thumbnailHover

                    enabled: control.removable
                }

                Rectangle {
                    anchors.top: parent.top
                    anchors.right: parent.right
                    anchors.margins: Metrics.s
                    width: 20
                    height: width
                    z: 2
                    visible: control.removable && thumbnailHover.hovered
                    radius: width / 2
                    color: closeMouseArea.pressed ? Qt.rgba(0, 0, 0, 0.7) : Qt.rgba(0, 0, 0, 0.55)

                    Item {
                        anchors.centerIn: parent
                        width: 12
                        height: width

                        Rectangle {
                            anchors.centerIn: parent
                            width: 10
                            height: 1.5
                            radius: height / 2
                            rotation: 45
                            color: "#FFFFFF"
                        }

                        Rectangle {
                            anchors.centerIn: parent
                            width: 10
                            height: 1.5
                            radius: height / 2
                            rotation: -45
                            color: "#FFFFFF"
                        }
                    }

                    MouseArea {
                        id: closeMouseArea

                        anchors.fill: parent
                        onClicked: control.removeWallpaperRequested(modelData)
                    }
                }
            }
        }
    }
}
