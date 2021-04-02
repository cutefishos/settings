import QtQuick 2.4
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import QtQuick.Window 2.3
import QtGraphicalEffects 1.0
import MeuiKit 1.0 as Meui

Meui.Window {
    id: rootWindow
    title: qsTr("Settings")
    visible: true
    width: 900
    height: 610

    minimumWidth: 900
    minimumHeight: 600
    headerBarHeight: 50

    headerBar: Item {
        Rectangle {
            id: leftArea
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            implicitWidth: sideBar.width
            color: Meui.Theme.secondBackgroundColor

            Behavior on color {
                ColorAnimation {
                    duration: 250
                    easing.type: Easing.Linear
                }
            }
        }

        RowLayout {
            anchors.fill: parent

            Label {
                text: rootWindow.title
                leftPadding: Meui.Units.largeSpacing + Meui.Units.smallSpacing
                font.pointSize: parent.height > 0 ? parent.height / 3 : 1
                Layout.preferredWidth: sideBar.width
                Layout.alignment: Qt.AlignBottom
            }

            Label {
                text: stackView.currentItem.headerTitle
                font.pointSize: parent.height > 0 ? parent.height / 3 : 1
                leftPadding: Meui.Units.largeSpacing + Meui.Units.smallSpacing
                Layout.alignment: Qt.AlignBottom
            }

            Item {
                Layout.fillWidth: true
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        SideBar {
            id: sideBar
            Layout.fillHeight: true

            onCurrentIndexChanged: {
                switchPageFromIndex(currentIndex)
            }
        }

        StackView {
            id: stackView
            Layout.fillWidth: true
            Layout.fillHeight: true
            initialItem: Qt.resolvedUrl(sideBar.model.get(0).page)
            clip: true

            pushEnter: Transition {}
            pushExit: Transition {}
        }
    }

    function switchPageFromIndex(index) {
        stackView.push(Qt.resolvedUrl(sideBar.model.get(index).page))
    }

    function switchPageFromName(pageName) {
        for (var i = 0; i < sideBar.model.count; ++i) {
            if (pageName === sideBar.model.get(i).name) {
                switchPageFromIndex(i)
                sideBar.view.currentIndex = i
            }
        }

        // If the window is minimized, it needs to be displayed agin.
        rootWindow.show()
        rootWindow.raise()
    }
}
