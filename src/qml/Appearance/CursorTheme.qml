import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import FishUI as FishUI
import Cutefish.Settings
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
            spacing: Metrics.sectionSpacing

            ColumnLayout {
                Layout.fillWidth: true
                spacing: Metrics.sectionLabelSpacing

                SectionLabel {
                    text: qsTr("Cursor theme")
                }

                RoundedItem {
                    GridView {
                        id: _view
                        Layout.fillWidth: true
                        implicitHeight: Math.ceil(_view.count / rowCount) * cellHeight + Metrics.m
                        model: cursorModel
                        interactive: false
                        visible: _view.count > 0

                        leftMargin: 0
                        rightMargin: 0

                        cellHeight: itemHeight
                        cellWidth: itemWidth

                        currentIndex: cursorModel.themeIndex(cursorModel.currentTheme)

                        property int rowCount: Math.max(1, Math.floor(_view.width / itemWidth))
                        property int itemWidth: 104 + Metrics.tileSpacing
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
                spacing: Metrics.sectionLabelSpacing

                SectionLabel {
                    text: qsTr("Cursor size")
                }

                RoundedItem {
                    RowLayout {
                        spacing: Metrics.rowSpacing

                        RowSlider {
                            Layout.fillWidth: true
                            from: 16
                            to: 64
                            stepSize: 8
                            stepped: true
                            value: cursorModel.cursorSize
                            onMoved: cursorModel.cursorSize = value
                        }

                        FishUI.Label {
                            text: cursorModel.cursorSize
                        }
                    }
                }
            }

        }
    }
}
