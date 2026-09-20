import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import FishUI 1.0 as FishUI
import Cutefish.Settings 1.0
import "../"

ItemPage {
    headerTitle: qsTr("Cursor Style")

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

            ColumnLayout {
                Layout.fillWidth: true
                spacing: FishUI.Units.smallSpacing

                SectionLabel {
                    text: qsTr("Cursor Theme")
                }

                RoundedItem {
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
                        cellWidth: itemWidth

                        currentIndex: cursorModel.themeIndex(cursorModel.currentTheme)

                        property int rowCount: Math.max(1, Math.floor(_view.width / itemWidth))
                        property int itemWidth: 120
                        property int itemHeight: 104

                        delegate: Item {
                            width: GridView.view.cellWidth
                            height: GridView.view.cellHeight

                            ImageSelectionItem {
                                anchors.left: parent.left
                                anchors.verticalCenter: parent.verticalCenter
                                width: implicitWidth
                                height: implicitHeight
                                iconSource: model.image
                                useIcon: true
                                iconSize: 24
                                previewWidth: 104
                                previewHeight: 64
                                previewInset: 0
                                text: model.name
                                checked: _view.currentIndex === index
                                onClicked: cursorModel.currentTheme = model.id
                            }
                        }
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: FishUI.Units.smallSpacing

                SectionLabel {
                    text: qsTr("Cursor Size")
                }

                RoundedItem {
                    RowLayout {
                        Slider {
                            Layout.fillWidth: true
                            from: 16
                            to: 64
                            stepSize: 8
                            value: cursorModel.cursorSize
                            onMoved: cursorModel.cursorSize = value
                        }

                        Label {
                            text: cursorModel.cursorSize
                        }
                    }
                }
            }

        }
    }
}
