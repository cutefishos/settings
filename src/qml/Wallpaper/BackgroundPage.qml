import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.0
import Cutefish.Settings 1.0
import FishUI 1.0 as FishUI

import "../"

ItemPage {
    headerTitle: qsTr("Background")

    Background {
        id: background
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent
            anchors.topMargin: FishUI.Units.smallSpacing
            spacing: FishUI.Units.largeSpacing

            // DesktopPreview {
            //     Layout.alignment: Qt.AlignHCenter
            //     width: 500
            //     height: 300
            // }

            RowLayout {
                spacing: FishUI.Units.largeSpacing * 2

                Label {
                    text: qsTr("Background type")
                }

                TabBar {
                    Layout.fillWidth: true
                    onCurrentIndexChanged: {
                        background.backgroundType = currentIndex
                    }

                    Component.onCompleted: {
                        currentIndex = background.backgroundType
                    }

                    TabButton {
                        text: qsTr("Picture")
                    }

                    TabButton {
                        text: qsTr("Color")
                    }
                }
            }

            Loader {
                Layout.fillWidth: true
                height: item.height
                sourceComponent: background.backgroundType === 0 ? imageView : colorView
            }
        }
    }

    Component {
        id: imageView

        GridView {
            id: _view
            // spacing: FishUI.Units.smallSpacing
            // orientation: Qt.Horizontal

            property int rowCount: _view.width / itemWidth

            implicitHeight: Math.ceil(_view.count / rowCount) * (itemHeight + FishUI.Units.largeSpacing * 2)

            clip: true
            model: background.backgrounds
            currentIndex: -1
            interactive: false

            cellHeight: calcExtraSpacing(itemHeight, _view.height) + itemHeight
            cellWidth: calcExtraSpacing(itemWidth, _view.width) + itemWidth

            property int itemWidth: 250
            property int itemHeight: 170

            delegate: Item {
                id: item

                property bool isSelected: modelData === background.currentBackgroundPath

                width: GridView.view.cellWidth
                height: GridView.view.cellHeight
                scale: 1.0

                Behavior on scale {
                    NumberAnimation {
                        duration: 100
                    }
                }

                Rectangle {
                    anchors.fill: parent
                    color: "transparent"
                    radius: FishUI.Theme.bigRadius + FishUI.Units.smallSpacing / 2

                    border.color: FishUI.Theme.highlightColor
                    border.width: image.status == Image.Ready & isSelected ? 3 : 0

                    Image {
                        id: image
                        anchors.fill: parent
                        anchors.margins: FishUI.Units.smallSpacing
                        source: "file://" + modelData
                        sourceSize: Qt.size(width, height)
                        fillMode: Image.PreserveAspectCrop
                        asynchronous: true
                        mipmap: true
                        cache: true
                        opacity: 1.0
                        smooth: false

                        Behavior on opacity {
                            NumberAnimation {
                                duration: 100
                                easing.type: Easing.InOutCubic
                            }
                        }

                        layer.enabled: true
                        layer.effect: OpacityMask {
                            maskSource: Item {
                                width: image.width
                                height: image.height

                                Rectangle {
                                    anchors.fill: parent
                                    radius: FishUI.Theme.bigRadius
                                }
                            }
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        acceptedButtons: Qt.LeftButton
                        hoverEnabled: true

                        onClicked: {
                            background.setBackground(modelData)
                        }

                        onEntered: function() {
                            image.opacity = 0.7
                        }
                        onExited: function() {
                            image.opacity = 1.0
                        }

                        onPressedChanged: item.scale = pressed ? 0.97 : 1.0
                    }
                }
            }

            function calcExtraSpacing(cellSize, containerSize) {
                var availableColumns = Math.floor(containerSize / cellSize)
                var extraSpacing = 0
                if (availableColumns > 0) {
                    var allColumnSize = availableColumns * cellSize
                    var extraSpace = Math.max(containerSize - allColumnSize, FishUI.Units.largeSpacing)
                    extraSpacing = extraSpace / availableColumns
                }
                return Math.floor(extraSpacing)
            }
        }
    }

    Component {
        id: colorView

        GridView {
            id: _colorView
            Layout.fillWidth: true
            height: _colorView.count * cellHeight

            cellWidth: 50
            cellHeight: 50

            interactive: false
            model: ListModel {}

            property var itemSize: 32

            Component.onCompleted: {
                model.append({"bgColor": "#2B8ADA"})
                model.append({"bgColor": "#4DA4ED"})
                model.append({"bgColor": "#B7E786"})
                model.append({"bgColor": "#F2BB73"})
                model.append({"bgColor": "#EE72EB"})
                model.append({"bgColor": "#F0905A"})
                model.append({"bgColor": "#595959"})
            }

            delegate: Rectangle {
                property bool checked: Qt.colorEqual(background.backgroundColor, bgColor)
                property color currentColor: bgColor

                width: _colorView.itemSize + FishUI.Units.largeSpacing
                height: width
                color: "transparent"
                radius: width / 2
                border.color: _mouseArea.pressed ? Qt.rgba(currentColor.r,
                                                           currentColor.g,
                                                           currentColor.b, 0.6)
                                                 : Qt.rgba(currentColor.r,
                                                           currentColor.g,
                                                           currentColor.b, 0.4)
                border.width: checked ? 3 : _mouseArea.containsMouse ? 2 : 0

                MouseArea {
                    id: _mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: background.backgroundColor = bgColor
                }

                Rectangle {
                    width: 32
                    height: width
                    anchors.centerIn: parent
                    color: currentColor
                    radius: width / 2
                }
            }
        }
    }
}
