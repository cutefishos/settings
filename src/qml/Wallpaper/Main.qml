import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Dialogs
import QtQuick.Layouts 1.12
import Cutefish.Settings 1.0
import Cutefish.Appearance 1.0 as FrameworkAppearance
import FishUI 1.0 as FishUI

import "../"

ItemPage {
    headerTitle: qsTr("Wallpaper")

    FrameworkAppearance.Wallpaper {
        id: background
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout

            anchors.fill: parent
            spacing: FishUI.Units.largeSpacing * 2

            RoundedItem {
                spacing: FishUI.Units.largeSpacing * 2

                WallpaperGrid {
                    title: qsTr("Your Wallpapers")
                    wallpapers: background.userWallpapers
                    currentWallpaper: background.path
                    imageSelected: background.type === 0
                    showAddWallpaper: true
                    removable: true

                    onWallpaperSelected: function (path) {
                        background.type = 0;
                        background.path = path;
                    }
                    onAddWallpaperRequested: wallpaperDialog.open()
                    onRemoveWallpaperRequested: function (path) {
                        background.removeUserWallpaper(path);
                    }
                }

                WallpaperGrid {
                    title: qsTr("System")
                    wallpapers: background.systemWallpapers
                    currentWallpaper: background.path
                    imageSelected: background.type === 0

                    onWallpaperSelected: function (path) {
                        background.type = 0;
                        background.path = path;
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: FishUI.Units.largeSpacing

                    Label {
                        text: qsTr("Colors")
                        font.bold: true
                    }

                    GridView {
                        id: colorView

                        readonly property int itemSize: 32
                        readonly property int columnCount: Math.max(Math.floor(width / cellWidth), 1)

                        Layout.fillWidth: true
                        Layout.preferredHeight: Math.ceil(count / columnCount) * cellHeight

                        cellWidth: 50
                        cellHeight: 50
                        interactive: false
                        model: ListModel {
                            ListElement {
                                bgColor: "#2B8ADA"
                            }
                            ListElement {
                                bgColor: "#4DA4ED"
                            }
                            ListElement {
                                bgColor: "#B7E786"
                            }
                            ListElement {
                                bgColor: "#F2BB73"
                            }
                            ListElement {
                                bgColor: "#EE72EB"
                            }
                            ListElement {
                                bgColor: "#F0905A"
                            }
                            ListElement {
                                bgColor: "#595959"
                            }
                            ListElement {
                                bgColor: "#000000"
                            }
                        }

                        delegate: Rectangle {
                            readonly property bool selected: background.type === 1 && Qt.colorEqual(background.color, bgColor)
                            readonly property color currentColor: bgColor

                            width: colorView.itemSize + FishUI.Units.largeSpacing
                            height: width
                            color: "transparent"
                            radius: width / 2
                            border.color: colorMouseArea.pressed ? Qt.rgba(currentColor.r, currentColor.g, currentColor.b, 0.6) : Qt.rgba(currentColor.r, currentColor.g, currentColor.b, 0.4)
                            border.width: selected ? 3 : colorMouseArea.containsMouse ? 2 : 0

                            Rectangle {
                                anchors.centerIn: parent
                                width: colorView.itemSize
                                height: width
                                color: currentColor
                                radius: width / 2
                            }

                            MouseArea {
                                id: colorMouseArea

                                anchors.fill: parent
                                hoverEnabled: true
                                onClicked: {
                                    background.type = 1;
                                    background.color = bgColor;
                                }
                            }
                        }
                    }
                }
            }

        }
    }

    FileDialog {
        id: wallpaperDialog

        title: qsTr("Add Wallpaper")
        nameFilters: [qsTr("Image files (*.jpg *.jpeg *.png *.webp)")]

        onAccepted: {
            var path = decodeURIComponent(selectedFile.toString().replace("file://", ""));
            background.type = 0;
            background.path = path;
        }
    }
}
