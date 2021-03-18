import QtQuick 2.4
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0

import Cutefish.Settings 1.0
import MeuiKit 1.0 as Meui

ItemPage {
    headerTitle: qsTr("Appearance")

    FontsModel {
        id: fontsModel
    }

    Appearance {
        id: appearance
    }

    Connections {
        target: fontsModel

        function onLoadFinished() {
            for (var i in fontsModel.generalFonts) {
                if (fontsModel.systemGeneralFont === fontsModel.generalFonts[i]) {
                    generalFontComboBox.currentIndex = i
                    break;
                }
            }

            for (i in fontsModel.fixedFonts) {
                if (fontsModel.systemFixedFont === fontsModel.fixedFonts[i]) {
                    fixedFontComboBox.currentIndex = i
                    break;
                }
            }

            console.log("fonts load finished")
        }
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent
            anchors.bottomMargin: Meui.Units.largeSpacing

            Label {
                text: qsTr("Theme")
                color: Meui.Theme.disabledTextColor
                bottomPadding: Meui.Units.smallSpacing
            }

            // Light Mode and Dark Mode
            RowLayout {
                spacing: Meui.Units.largeSpacing * 2

                IconCheckBox {
                    source: "qrc:/images/light_mode.svg"
                    text: qsTr("Light")
                    checked: !Meui.Theme.darkMode
                    onClicked: appearance.switchDarkMode(false)
                }

                IconCheckBox {
                    source: "qrc:/images/dark_mode.svg"
                    text: qsTr("Dark")
                    checked: Meui.Theme.darkMode
                    onClicked: appearance.switchDarkMode(true)
                }
            }

            Item {
                height: Meui.Units.largeSpacing
            }

            RowLayout {
                spacing: Meui.Units.largeSpacing

                Label {
                    id: dimsTipsLabel
                    text: qsTr("Dim the wallpaper in dark theme")
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                }

                Item {
                    Layout.fillWidth: true
                }

                Switch {
                    checked: appearance.dimsWallpaper
                    height: dimsTipsLabel.height
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                    onCheckedChanged: appearance.setDimsWallpaper(checked)
                }
            }

            HorizontalDivider {}

            Label {
                text: qsTr("Accent color")
                color: Meui.Theme.disabledTextColor
                bottomPadding: Meui.Units.smallSpacing
            }

            GridView {
                id: accentColorView
                height: itemSize + Meui.Units.largeSpacing * 2
                Layout.fillWidth: true
                cellWidth: height
                cellHeight: height
                interactive: false
                model: ListModel {}

                property var itemSize: 32

                Component.onCompleted: {
                    model.append({"accentColor": String(Meui.Theme.blueColor)})
                    model.append({"accentColor": String(Meui.Theme.redColor)})
                    model.append({"accentColor": String(Meui.Theme.greenColor)})
                    model.append({"accentColor": String(Meui.Theme.purpleColor)})
                    model.append({"accentColor": String(Meui.Theme.pinkColor)})
                    model.append({"accentColor": String(Meui.Theme.orangeColor)})
                }

                delegate: Rectangle {
                    property bool checked: Qt.colorEqual(Meui.Theme.highlightColor, accentColor)
                    property color currentColor: accentColor

                    width: accentColorView.itemSize + Meui.Units.largeSpacing
                    height: width
                    color: "transparent"
                    radius: width / 2
                    border.color: _mouseArea.pressed ? Qt.rgba(currentColor.r,
                                                               currentColor.g,
                                                               currentColor.b, 0.6)
                                                     : Qt.rgba(currentColor.r,
                                                               currentColor.g,
                                                               currentColor.b, 0.4)
                    border.width: checked ? 3 : _mouseArea.containsMouse ? 2 : 0

                    MouseArea {
                        id: _mouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: appearance.setAccentColor(index)
                    }

                    Rectangle {
                        width: 32
                        height: width
                        anchors.centerIn: parent
                        color: currentColor
                        radius: width / 2

                        Image {
                            anchors.centerIn: parent
                            width: parent.height * 0.5
                            height: width
                            sourceSize: Qt.size(width, height)
                            source: "qrc:/images/checked.svg"
                            visible: checked

                            ColorOverlay {
                                anchors.fill: parent
                                source: parent
                                color: Meui.Theme.highlightedTextColor
                                opacity: 1
                                visible: true
                            }
                        }
                    }
                }
            }

            HorizontalDivider {}

            // Font
            Label {
                text: qsTr("Font")
                color: Meui.Theme.disabledTextColor
                bottomPadding: Meui.Units.smallSpacing
            }

            GridLayout {
                rows: 3
                columns: 2

                columnSpacing: Meui.Units.largeSpacing * 2

                Label {
                    text: qsTr("General Font")
                    bottomPadding: Meui.Units.smallSpacing
                }

                ComboBox {
                    id: generalFontComboBox
                    model: fontsModel.generalFonts
                    enabled: true
                    Layout.fillWidth: true
                    onActivated: appearance.setGenericFontFamily(currentText)
                }

                Label {
                    text: qsTr("Fixed Font")
                    bottomPadding: Meui.Units.smallSpacing
                }

                ComboBox {
                    id: fixedFontComboBox
                    model: fontsModel.fixedFonts
                    enabled: true
                    Layout.fillWidth: true
                    onActivated: appearance.setFixedFontFamily(currentText)
                }

                Label {
                    text: qsTr("Font Size")
                    bottomPadding: Meui.Units.smallSpacing
                }

                TabBar {
                    Layout.fillWidth: true

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

                        if (appearance.fontPointSize <= 11)
                            index = 0
                        else if (appearance.fontPointSize <= 13)
                            index = 1
                        else if (appearance.fontPointSize <= 15)
                            index = 2
                        else if (appearance.fontPointSize <= 18)
                            index = 3

                        return index
                    }

                    onCurrentIndexChanged: {
                        var fontSize = 0

                        switch (currentIndex) {
                        case 0:
                            fontSize = 11
                            break;
                        case 1:
                            fontSize = 13
                            break;
                        case 2:
                            fontSize = 15
                            break;
                        case 3:
                            fontSize = 18
                            break;
                        }

                        appearance.setFontPointSize(fontSize)
                    }
                }
            }

            Item {
                Layout.fillHeight: true
            }
        }
    }
}
