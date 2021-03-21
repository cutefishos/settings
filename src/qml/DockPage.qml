import QtQuick 2.4
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import Cutefish.Settings 1.0
import MeuiKit 1.0 as Meui

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
                color: Meui.Theme.disabledTextColor
                bottomPadding: Meui.Units.smallSpacing
            }

            RowLayout {
                spacing: Meui.Units.largeSpacing * 2

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
            }

            HorizontalDivider {}

            Label {
                text: qsTr("Size")
                color: Meui.Theme.disabledTextColor
                bottomPadding: Meui.Units.smallSpacing
            }

            TabBar {
                id: dockSizeTabbar
                Layout.fillWidth: true
                bottomPadding: Meui.Units.smallSpacing

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

                    if (appearance.dockIconSize <= 48)
                        index = 0
                    else if (appearance.dockIconSize <= 64)
                        index = 1
                    else if (appearance.dockIconSize <= 88)
                        index = 2
                    else if (appearance.dockIconSize <= 128)
                        index = 3

                    return index
                }

                onCurrentIndexChanged: {
                    var iconSize = 0

                    switch (currentIndex) {
                    case 0:
                        iconSize = 48
                        break;
                    case 1:
                        iconSize = 64
                        break;
                    case 2:
                        iconSize = 88
                        break;
                    case 3:
                        iconSize = 128
                        break;
                    }

                    appearance.setDockIconSize(iconSize)
                }
            }

            HorizontalDivider {}

            GridLayout {
                columns: 2

                Label {
                    text: qsTr("Rounded window")
                    Layout.fillWidth: true
                }

                Switch {
                    Layout.fillHeight: true
                    checked: appearance.dockRoundedWindow

                    onCheckedChanged: {
                        appearance.setDockRoundedWindow(checked)
                    }
                }
            }

            Item {
                Layout.fillHeight: true
            }
        }
    }
}
