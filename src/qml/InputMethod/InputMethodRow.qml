import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import FishUI 1.0 as FishUI

Item {
    id: control

    property string text
    property string iconSource
    property bool checked: false

    signal clicked()

    readonly property real horizontalPadding: FishUI.Units.largeSpacing
    readonly property real iconSize: 24

    Layout.fillWidth: true
    implicitHeight: 44
    activeFocusOnTab: true

    Accessible.role: Accessible.RadioButton
    Accessible.name: text
    Accessible.checked: checked
    Accessible.onPressAction: clicked()

    Keys.onSpacePressed: clicked()
    Keys.onReturnPressed: clicked()
    Keys.onEnterPressed: clicked()

    Rectangle {
        anchors.fill: parent
        anchors.margins: 4
        radius: FishUI.Theme.smallRadius
        color: control.checked
               ? Qt.alpha(FishUI.Theme.highlightColor, FishUI.Theme.darkMode ? 0.22 : 0.12)
               : mouseArea.containsMouse
                 ? Qt.alpha(FishUI.Theme.textColor, FishUI.Theme.darkMode ? 0.1 : 0.06)
                 : "transparent"
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton
        onClicked: {
            control.forceActiveFocus()
            control.clicked()
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: control.horizontalPadding
        anchors.rightMargin: control.horizontalPadding
        spacing: FishUI.Units.largeSpacing

        Item {
            Layout.preferredWidth: control.iconSize
            Layout.preferredHeight: control.iconSize

            FishUI.IconItem {
                anchors.centerIn: parent
                width: 18
                height: 18
                source: control.iconSource ? control.iconSource : "input-keyboard"
                smooth: true
            }
        }

        Label {
            Layout.fillWidth: true
            text: control.text
            textFormat: Text.PlainText
            color: FishUI.Theme.textColor
            elide: Text.ElideRight
        }

        FishUI.Icon {
            Layout.preferredWidth: 18
            Layout.preferredHeight: 18
            name: "check"
            color: FishUI.Theme.highlightColor
            visible: control.checked
        }
    }
}
