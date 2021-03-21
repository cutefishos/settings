import QtQuick 2.4
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import MeuiKit 1.0 as Meui
import Cutefish.Settings 1.0 as Settings

ItemPage {
    headerTitle: qsTr("Language")

    Settings.Language {
        id: language
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.topMargin: Meui.Units.smallSpacing

        ListView {
            id: listView

            Layout.fillWidth: true
            Layout.fillHeight: true

            model: language.languages
            clip: true

            topMargin: Meui.Units.largeSpacing
            leftMargin: Meui.Units.largeSpacing * 2
            rightMargin: Meui.Units.largeSpacing * 2
            spacing: Meui.Units.largeSpacing

            currentIndex: language.currentLanguage

            ScrollBar.vertical: ScrollBar {
                bottomPadding: Meui.Theme.smallRadius
            }

            highlightFollowsCurrentItem: true
            highlightMoveDuration: 0
            highlightResizeDuration : 0
            highlight: Rectangle {
                color: Meui.Theme.highlightColor
                radius: Meui.Theme.smallRadius
            }

            delegate: MouseArea {
                property bool isSelected: index == listView.currentIndex

                id: item
                width: ListView.view.width - ListView.view.leftMargin - ListView.view.rightMargin
                height: 50
                hoverEnabled: true
                acceptedButtons: Qt.LeftButton

                onClicked: {
                    language.setCurrentLanguage(index)
                }

                Rectangle {
                    anchors.fill: parent
                    color: isSelected ? "transparent" : item.containsMouse ? Meui.Theme.disabledTextColor : "transparent"
                    opacity: isSelected ? 1 : 0.1
                    radius: Meui.Theme.smallRadius
                }

                Label {
                    anchors.fill: parent
                    anchors.leftMargin: Meui.Units.smallSpacing
                    anchors.rightMargin: Meui.Units.smallSpacing
                    color: isSelected ? Meui.Theme.highlightedTextColor : Meui.Theme.textColor
                    text: modelData
                }
            }
        }
    }
}
