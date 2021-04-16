import QtQuick 2.4
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import FishUI 1.0 as FishUI
import Cutefish.Settings 1.0 as Settings

ItemPage {
    headerTitle: qsTr("Language")

    Settings.Language {
        id: language
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.topMargin: FishUI.Units.smallSpacing

        ListView {
            id: listView

            Layout.fillWidth: true
            Layout.fillHeight: true

            model: language.languages
            clip: true

            topMargin: FishUI.Units.largeSpacing
            leftMargin: FishUI.Units.largeSpacing * 2
            rightMargin: FishUI.Units.largeSpacing * 2
            spacing: FishUI.Units.largeSpacing

            currentIndex: language.currentLanguage

            ScrollBar.vertical: ScrollBar {
                bottomPadding: FishUI.Theme.smallRadius
            }

            highlightFollowsCurrentItem: true
            highlightMoveDuration: 0
            highlightResizeDuration : 0
            highlight: Rectangle {
                color: FishUI.Theme.highlightColor
                radius: FishUI.Theme.smallRadius
            }

            delegate: MouseArea {
                property bool isSelected: index == listView.currentIndex

                id: item
                width: ListView.view.width - ListView.view.leftMargin - ListView.view.rightMargin
                height: 43
                hoverEnabled: true
                acceptedButtons: Qt.LeftButton

                onClicked: {
                    language.setCurrentLanguage(index)
                }

                Rectangle {
                    anchors.fill: parent
                    color: isSelected ? "transparent" : item.containsMouse ? FishUI.Theme.disabledTextColor : "transparent"
                    opacity: isSelected ? 1 : 0.1
                    radius: FishUI.Theme.smallRadius
                }

                Label {
                    anchors.fill: parent
                    anchors.leftMargin: FishUI.Units.smallSpacing
                    anchors.rightMargin: FishUI.Units.smallSpacing
                    color: isSelected ? FishUI.Theme.highlightedTextColor : FishUI.Theme.textColor
                    text: modelData
                }
            }
        }
    }
}
