import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.0
import Cutefish.Settings 1.0
import MeuiKit 1.0 as Meui

ItemPage {
    headerTitle: qsTr("Wallpaper")

    Background {
        id: background
    }

    GridView {
        anchors.fill: parent
        anchors.bottomMargin: Meui.Units.largeSpacing
        leftMargin: Meui.Units.smallSpacing

        cellWidth: 320
        cellHeight: 180

        clip: true
        model: background.backgrounds

        currentIndex: -1

        ScrollBar.vertical: ScrollBar {}

        delegate: Item {
            id: item

            // required property variant modelData
            property bool isSelected: modelData === background.currentBackgroundPath

            width: GridView.view.cellWidth
            height: GridView.view.cellHeight

            Rectangle {
                anchors.fill: parent
                anchors.leftMargin: Meui.Units.largeSpacing
                anchors.rightMargin: Meui.Units.largeSpacing
                anchors.topMargin: Meui.Units.smallSpacing
                anchors.bottomMargin: Meui.Units.smallSpacing
                color: "transparent"
                radius: Meui.Theme.bigRadius + Meui.Units.smallSpacing / 2

                border.color: Meui.Theme.highlightColor
                border.width: image.status == Image.Ready & isSelected ? 3 : 0

                Image {
                    id: image
                    anchors.fill: parent
                    anchors.margins: Meui.Units.smallSpacing
                    source: "file://" + modelData
                    sourceSize: Qt.size(width, height)
                    fillMode: Image.PreserveAspectCrop
                    asynchronous: true
                    mipmap: true
                    cache: true
                    opacity: 1.0

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
                                radius: Meui.Theme.bigRadius
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
                }
            }
        }
    }
}
