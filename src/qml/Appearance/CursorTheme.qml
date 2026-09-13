import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import FishUI 1.0 as FishUI
import Cutefish.Settings 1.0
import "../"

ItemPage {
    headerTitle: qsTr("Cursor Theme")

    CursorThemeModel {
        id: cursorModel
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent
            spacing: FishUI.Units.largeSpacing * 2

            RoundedItem {
                RowLayout {
                    Label { text: qsTr("Cursor size") }
                    Slider {
                        Layout.fillWidth: true
                        from: 16
                        to: 64
                        stepSize: 8
                        value: cursorModel.cursorSize
                        onMoved: cursorModel.cursorSize = value
                    }
                    Label { text: cursorModel.cursorSize }
                }

                GridView {
                    id: _view
                    Layout.fillWidth: true
                    implicitHeight: Math.ceil(_view.count / rowCount) * cellHeight + FishUI.Units.largeSpacing
                    model: cursorModel
                    interactive: false
                    visible: _view.count > 0

                    leftMargin: 0
                    rightMargin: 0

                    cellHeight: itemHeight
                    cellWidth: calcExtraSpacing(itemWidth, _view.width) + itemWidth

                    currentIndex: cursorModel.themeIndex(cursorModel.currentTheme)

                    property int rowCount: Math.max(1, Math.floor(_view.width / itemWidth))
                    property int itemWidth: 128
                    property int itemHeight: 128

                    function calcExtraSpacing(cellSize, containerSize) {
                        var availableColumns = Math.floor(containerSize / cellSize)
                        var extraSpacing = 0
                        if (availableColumns > 0) {
                            var allColumnSize = availableColumns * cellSize
                            var extraSpace = Math.max(containerSize - allColumnSize, 0)
                            extraSpacing = extraSpace / availableColumns
                        }
                        return Math.floor(extraSpacing)
                    }

                    delegate: Item {
                        width: GridView.view.cellWidth
                        height: GridView.view.cellHeight
                        scale: _mouseArea.pressed ? 0.95 : 1.0

                        property bool isCurrent: _view.currentIndex === index

                        Behavior on scale {
                            NumberAnimation {
                                duration: 200
                                easing.type: Easing.OutSine
                            }
                        }

                        MouseArea {
                            id: _mouseArea
                            anchors.fill: parent
                            anchors.margins: FishUI.Units.smallSpacing * 1.5
                            onClicked: {
                                cursorModel.currentTheme = model.id
                            }
                        }

                        Rectangle {
                            anchors.fill: parent
                            anchors.margins: FishUI.Units.smallSpacing * 1.5
                            color: FishUI.Theme.darkMode ? "#3C3C3C" : "#FAFAFA"
                            radius: FishUI.Theme.mediumRadius
                            z: -1

                            border.width: isCurrent ? 3 : 0
                            border.color: FishUI.Theme.highlightColor
                        }

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: FishUI.Units.smallSpacing * 1.5

                            Item {
                                Layout.fillHeight: true
                            }

                            Item {
                                height: FishUI.Units.largeSpacing
                            }

                            FishUI.IconItem {
                                width: 22
                                height: 22
                                source: model.image
                                smooth: true
                                Layout.alignment: Qt.AlignHCenter
                            }

                            Item {
                                Layout.fillHeight: true
                            }

                            Label {
                                text: model.name
                                Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
                                bottomPadding: FishUI.Units.largeSpacing
                            }

                            Item {
                                Layout.fillHeight: true
                            }
                        }
                    }
                }
            }

            Item {
                height: FishUI.Units.smallSpacing
            }
        }
    }
}
