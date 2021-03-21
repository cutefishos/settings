import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.0
import Cutefish.Settings 1.0
import MeuiKit 1.0 as Meui

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
            anchors.topMargin: Meui.Units.smallSpacing
            spacing: Meui.Units.largeSpacing

            DesktopPreview {
                Layout.alignment: Qt.AlignHCenter
                width: 500
                height: 300
            }

            RowLayout {
                spacing: Meui.Units.largeSpacing * 2

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

        ListView {
            id: _view
            Layout.fillWidth: true
            spacing: Meui.Units.smallSpacing
            orientation: Qt.Horizontal

            height: 150
            clip: true
            model: background.backgrounds
            currentIndex: -1

            delegate: Item {
                id: item

                property bool isSelected: modelData === background.currentBackgroundPath

                width: 200
                height: _view.height

                Rectangle {
                    anchors.fill: parent
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

                width: _colorView.itemSize + Meui.Units.largeSpacing
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
