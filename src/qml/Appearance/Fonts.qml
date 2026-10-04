import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Cutefish.Settings
import Cutefish.Appearance as FrameworkAppearance
import FishUI as FishUI
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
                        fontPreview: true
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
                        fontPreview: true
                        onActivated: appearance.setFixedFontFamily(currentText)
                    }
                }

                ListRow {
                    interactive: false
                    separatorVisible: false
                    minimumRowHeight: Metrics.controlRowHeight
                    text: qsTr("Font Size")

                    FishUI.ComboBox {
                        // ThemeManager's fallback size in cutefish-core.
                        readonly property int defaultSize: 9
                        readonly property var sizes: [8, 9, 10, 11, 12, 13, 14, 15, 16, 18, 20]

                        Layout.preferredWidth: Math.round(parent.width / 2)
                        model: sizes.map(size => size === defaultSize ? qsTr("%1pt (Default)").arg(size)
                                                                     : qsTr("%1pt").arg(size))
                        // The menu marks the default; the box shows only the size.
                        displayText: qsTr("%1pt").arg(currentIndex < 0 ? appearance.fontPointSize : sizes[currentIndex])
                        currentIndex: sizes.indexOf(appearance.fontPointSize)
                        onActivated: index => appearance.setFontPointSize(sizes[index])
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
