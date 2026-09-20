import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Qt5Compat.GraphicalEffects
import FishUI 1.0 as FishUI

Item {
    id: control

    property alias source: preview.source
    property alias text: label.text
    property alias fillMode: preview.fillMode
    property alias asynchronous: preview.asynchronous
    property alias mipmap: preview.mipmap
    property alias cache: preview.cache
    property alias smooth: preview.smooth
    property alias imageStatus: preview.status
    property alias iconSource: iconPreview.source
    property bool checked: false
    property bool useIcon: false
    property real iconSize: 36
    property real previewWidth: 96
    property real previewHeight: 96
    property real previewInset: 4
    property real sourceSizeScale: 1
    readonly property bool hovered: mouseArea.containsMouse

    signal clicked

    implicitWidth: layout.implicitWidth
    implicitHeight: layout.implicitHeight

    ColumnLayout {
        id: layout

        anchors.fill: parent

        Rectangle {
            width: control.previewWidth
            height: control.previewHeight
            radius: FishUI.Theme.bigRadius
            color: "transparent"
            border.color: FishUI.Theme.highlightColor
            border.width: control.checked ? 2 : 0

            Image {
                id: preview

                anchors.fill: parent
                anchors.margins: control.previewInset
                visible: !control.useIcon
                sourceSize: Qt.size(width * control.sourceSizeScale,
                                    height * control.sourceSizeScale)
                smooth: false

                layer.enabled: true
                layer.effect: OpacityMask {
                    maskSource: Item {
                        width: preview.width
                        height: preview.height

                        Rectangle {
                            anchors.fill: parent
                            radius: Math.max(FishUI.Theme.bigRadius - control.previewInset, 0)
                            antialiasing: true
                        }
                    }
                }
            }

            FishUI.IconItem {
                id: iconPreview

                anchors.centerIn: parent
                width: control.iconSize
                height: control.iconSize
                visible: control.useIcon
            }

            Rectangle {
                anchors.fill: parent
                anchors.margins: control.previewInset
                visible: control.useIcon || preview.status === Image.Ready
                radius: Math.max(FishUI.Theme.bigRadius - control.previewInset, 0)
                color: "#FFFFFF"
                opacity: control.hovered ? 0.08 : 0
            }
        }

        Label {
            id: label

            Layout.alignment: Qt.AlignHCenter
            color: control.checked ? FishUI.Theme.highlightColor : FishUI.Theme.textColor
            visible: text
        }
    }

    MouseArea {
        id: mouseArea

        anchors.fill: parent
        acceptedButtons: Qt.LeftButton
        hoverEnabled: true
        onClicked: control.clicked()
    }
}
