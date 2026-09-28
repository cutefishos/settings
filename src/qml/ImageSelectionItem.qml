import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Cutefish.Settings 1.0
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
    property size sourceSize: Qt.size(preview.width, preview.height)

    signal clicked

    implicitWidth: layout.implicitWidth
    implicitHeight: layout.implicitHeight

    ColumnLayout {
        id: layout

        anchors.fill: parent
        spacing: Metrics.s

        FishUI.RoundedRectangle {
            // The size goes through the layout's own hints: an explicit width
            // and height alone are read once, and a thumbnail that is resized
            // after the item was built keeps the stale size until something
            // else invalidates the layout.
            Layout.preferredWidth: control.previewWidth
            Layout.preferredHeight: control.previewHeight
            radius: FishUI.Theme.bigRadius
            color: "transparent"
            border.color: FishUI.Theme.highlightColor
            border.width: control.checked ? 2 : 0

            Image {
                id: preview

                anchors.fill: parent
                anchors.margins: control.previewInset
                visible: !control.useIcon
                sourceSize: control.sourceSize
                smooth: false

                layer.enabled: true
                layer.effect: FishUI.OpacityMask {
                    maskSource: Item {
                        width: preview.width
                        height: preview.height

                        FishUI.RoundedRectangle {
                            anchors.fill: parent
                            radius: FishUI.Theme.bigRadius
                            inset: control.previewInset
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
        }

        FishUI.Label {
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
        onClicked: control.clicked()
    }
}
