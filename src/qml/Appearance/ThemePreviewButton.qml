import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Qt5Compat.GraphicalEffects
import FishUI 1.0 as FishUI

Item {
    id: control

    property alias source: preview.source
    property alias text: label.text
    property bool checked: false
    property real previewWidth: 120
    property real previewHeight: previewWidth * 0.68
    readonly property real previewInset: 4

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
                sourceSize: Qt.size(width, height)
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
        }

        Label {
            id: label

            Layout.alignment: Qt.AlignHCenter
            color: control.checked ? FishUI.Theme.highlightColor : FishUI.Theme.textColor
            visible: text
        }
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton
        onClicked: control.clicked()
    }
}
