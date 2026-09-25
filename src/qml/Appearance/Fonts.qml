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

            ListCard {
                ListRow {
                    interactive: false
                    separatorVisible: false
                    minimumRowHeight: Metrics.controlRowHeight
                    text: qsTr("General Font")

                    FishUI.ComboBox {
                        id: generalFontComboBox
                        // The label fills the rest of the row.
                        Layout.preferredWidth: Math.round(parent.width / 2)
                        model: fontsModel.generalFonts
                        onActivated: appearance.setFontFamily(currentText)
                    }
                }

                ListRow {
                    interactive: false
                    separatorVisible: false
                    minimumRowHeight: Metrics.controlRowHeight
                    text: qsTr("Fixed Font")

                    FishUI.ComboBox {
                        id: fixedFontComboBox
                        Layout.preferredWidth: Math.round(parent.width / 2)
                        model: fontsModel.fixedFonts
                        onActivated: appearance.setFixedFontFamily(currentText)
                    }
                }

                ListRow {
                    interactive: false
                    separatorVisible: false
                    minimumRowHeight: Metrics.controlRowHeight
                    text: qsTr("Font Size")

                    FishUI.SegmentedControl {
                        Layout.preferredWidth: Math.round(parent.width / 2)

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
                }

                ListRow {
                    interactive: false
                    separatorVisible: false
                    minimumRowHeight: Metrics.controlRowHeight
                    text: qsTr("Hinting")

                    FishUI.ComboBox {
                        Layout.preferredWidth: Math.round(parent.width / 2)
                        model: fonts.hintingModel
                        textRole: "display"
                        currentIndex: fonts.hintingCurrentIndex
                        onCurrentIndexChanged: fonts.hintingCurrentIndex = currentIndex
                    }
                }

                ListRow {
                    interactive: false
                    separatorVisible: false
                    text: qsTr("Anti-Aliasing")

                    FishUI.Switch {
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                        rightPadding: 0
                        rightInset: 0
                        checked: fonts.antiAliasing
                        onCheckedChanged: fonts.antiAliasing = checked
                    }
                }
            }
        }
    }
}
