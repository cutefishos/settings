import QtQuick 2.4
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3

import Cutefish.Settings 1.0
import Cutefish.Appearance 1.0 as FrameworkAppearance
import FishUI 1.0 as FishUI
import "../"

ItemPage {
    headerTitle: qsTr("Fonts")

    FrameworkAppearance.Appearance {
        id: appearance
    }

    FontsModel {
        id: fontsModel
    }

    Fonts {
        id: fonts
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
        }
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent
            spacing: Metrics.sectionSpacing

            RoundedItem {
                GridLayout {
                    columns: 2
                    Layout.fillWidth: true

                    columnSpacing: Metrics.formColumnSpacing
                    rowSpacing: Metrics.formRowSpacing

                    FishUI.Label {
                        text: qsTr("General Font")
                    }

                    FishUI.ComboBox {
                        id: generalFontComboBox
                        model: fontsModel.generalFonts
                        Layout.fillWidth: true
                        topInset: 0
                        bottomInset: 0
                        onActivated: appearance.setFontFamily(currentText)
                    }

                    FishUI.Label {
                        text: qsTr("Fixed Font")
                    }

                    FishUI.ComboBox {
                        id: fixedFontComboBox
                        model: fontsModel.fixedFonts
                        Layout.fillWidth: true
                        topInset: 0
                        bottomInset: 0
                        onActivated: appearance.setFixedFontFamily(currentText)
                    }

                    FishUI.Label {
                        text: qsTr("Font Size")
                    }

                    FishUI.SegmentedControl {
                        Layout.fillWidth: true

                        FishUI.TabButton {
                            text: qsTr("Small")
                        }

                        FishUI.TabButton {
                            text: qsTr("Medium")
                        }

                        FishUI.TabButton {
                            text: qsTr("Large")
                        }

                        FishUI.TabButton {
                            text: qsTr("Huge")
                        }

                        currentIndex: {
                            var index = 0

                            if (appearance.fontPointSize <= 9)
                                index = 0
                            else if (appearance.fontPointSize <= 10)
                                index = 1
                            else if (appearance.fontPointSize <= 12)
                                index = 2
                            else if (appearance.fontPointSize <= 15)
                                index = 3

                            return index
                        }

                        onCurrentIndexChanged: {
                            var fontSize = 0

                            switch (currentIndex) {
                            case 0:
                                fontSize = 9
                                break;
                            case 1:
                                fontSize = 10
                                break;
                            case 2:
                                fontSize = 12
                                break;
                            case 3:
                                fontSize = 15
                                break;
                            }

                            appearance.setFontPointSize(fontSize)
                        }
                    }

                    FishUI.Label {
                        text: qsTr("Hinting")
                    }

                    FishUI.ComboBox {
                        model: fonts.hintingModel
                        textRole: "display"
                        Layout.fillWidth: true
                        currentIndex: fonts.hintingCurrentIndex
                        onCurrentIndexChanged: fonts.hintingCurrentIndex = currentIndex
                    }

                    FishUI.Label {
                        text: qsTr("Anti-Aliasing")
                    }

                    FishUI.Switch {
                        Layout.fillHeight: true
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                        checked: fonts.antiAliasing
                        rightPadding: 0
                        rightInset: 0
                        onCheckedChanged: fonts.antiAliasing = checked
                    }
                }
            }
        }
    }
}
