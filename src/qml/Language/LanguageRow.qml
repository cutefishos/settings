import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FishUI 1.0 as FishUI
import "../"

ItemDelegate {
    id: control

    property string languageName
    property string description
    property bool selected: false
    property bool showCheck: false
    property bool separatorVisible: false

    height: 52
    leftPadding: 12
    rightPadding: 12
    topPadding: 8
    bottomPadding: 8

    background: FishUI.RoundedRectangle {
        radius: FishUI.Theme.smallRadius
        color: control.selected ? Qt.alpha(FishUI.Theme.highlightColor, 0.14)
                                : control.hovered ? Qt.alpha(FishUI.Theme.textColor, 0.05)
                                                  : "transparent"
        border.width: control.activeFocus ? 1 : 0
        border.color: FishUI.Theme.highlightColor
    }

    Separator {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: control.leftPadding
        anchors.rightMargin: control.rightPadding
        visible: control.separatorVisible
    }

    contentItem: RowLayout {
        spacing: 12

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2

            FishUI.Label {
                id: primaryLabel
                Layout.fillWidth: true
                text: control.languageName
                textFormat: Text.PlainText
                color: FishUI.Theme.textColor
                elide: Text.ElideRight
            }

            FishUI.Label {
                Layout.fillWidth: true
                text: control.description
                textFormat: Text.PlainText
                color: FishUI.Theme.disabledTextColor
                font.pixelSize: Math.max(1, primaryLabel.font.pixelSize - 1)
                elide: Text.ElideRight
            }
        }

        FishUI.Icon {
            name: "check"
            color: FishUI.Theme.highlightColor
            visible: control.showCheck
        }
    }
}
