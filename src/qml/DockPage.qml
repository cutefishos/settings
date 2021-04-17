import QtQuick 2.4
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import Cutefish.Settings 1.0
import FishUI 1.0 as FishUI

ItemPage {
    headerTitle: qsTr("Dock")

    Appearance {
        id: appearance
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent

            // Dock
            Label {
                text: qsTr("Position on screen")
                color: FishUI.Theme.disabledTextColor
                bottomPadding: FishUI.Units.smallSpacing
            }

            RowLayout {
                spacing: FishUI.Units.largeSpacing * 2

                IconCheckBox {
                    source: "qrc:/images/dock_left.svg"
                    text: qsTr("Left")
                    checked: appearance.dockDirection === 0
                    onClicked: appearance.setDockDirection(0)
                }

                IconCheckBox {
                    source: "qrc:/images/dock_bottom.svg"
                    text: qsTr("Bottom")
                    checked: appearance.dockDirection === 1
                    onClicked: appearance.setDockDirection(1)
                }

                IconCheckBox {
                    source: "qrc:/images/dock_right.svg"
                    text: qsTr("Right")
                    checked: appearance.dockDirection === 2
                    onClicked: appearance.setDockDirection(2)
                }
            }

            HorizontalDivider {}

            Label {
                text: qsTr("Size")
                color: FishUI.Theme.disabledTextColor
                bottomPadding: FishUI.Units.smallSpacing
            }

            TabBar {
                id: dockSizeTabbar
                Layout.fillWidth: true
                bottomPadding: FishUI.Units.smallSpacing

                TabButton {
                    text: qsTr("Small")
                }

                TabButton {
                    text: qsTr("Medium")
                }

                TabButton {
                    text: qsTr("Large")
                }

                TabButton {
                    text: qsTr("Huge")
                }

                currentIndex: {
                    var index = 0

                    if (appearance.dockIconSize <= 50)
                        index = 0
                    else if (appearance.dockIconSize <= 65)
                        index = 1
                    else if (appearance.dockIconSize <= 90)
                        index = 2
                    else if (appearance.dockIconSize <= 130)
                        index = 3

                    return index
                }

                onCurrentIndexChanged: {
                    var iconSize = 0

                    switch (currentIndex) {
                    case 0:
                        iconSize = 50
                        break;
                    case 1:
                        iconSize = 65
                        break;
                    case 2:
                        iconSize = 90
                        break;
                    case 3:
                        iconSize = 130
                        break;
                    }

                    appearance.setDockIconSize(iconSize)
                }
            }

            Item {
                Layout.fillHeight: true
            }
        }
    }
}
