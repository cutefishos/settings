import QtQuick 2.4
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import Cutefish.Settings 1.0
import Cutefish.Screen 1.0 as CS
import MeuiKit 1.0 as Meui

ItemPage {
    headerTitle: qsTr("Display")

    Appearance {
        id: appearance
    }

    Brightness {
        id: brightness
    }

    CS.Screen {
        id: screen
    }

    Timer {
        id: brightnessTimer
        interval: 100
        onTriggered: {
            brightness.setValue(brightnessSlider.value)
        }
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent

            Label {
                text: qsTr("Brightness")
                color: Meui.Theme.disabledTextColor
                bottomPadding: Meui.Units.largeSpacing
                visible: brightness.enabled
            }

            SwipeView {
                Layout.fillWidth: true
                Layout.fillHeight: true

                Repeater {
                    model: screen.outputModel

                    ComboBox {
                        model: model.resolutions
                    }
                }
            }


            RowLayout {
                spacing: Meui.Units.largeSpacing
                visible: brightness.enabled

                Image {
                    width: brightnessSlider.height
                    height: width
                    sourceSize.width: width
                    sourceSize.height: height
                    source: "qrc:/images/" + (Meui.Theme.darkMode ? "dark" : "light") + "/display-brightness-low-symbolic.svg"
                }

                Slider {
                    id: brightnessSlider
                    Layout.fillWidth: true
                    value: brightness.value
                    from: 0
                    to: 100
                    stepSize: 1
                    onMoved: brightnessTimer.start()
                }

                Image {
                    width: brightnessSlider.height
                    height: width
                    sourceSize.width: width
                    sourceSize.height: height
                    source: "qrc:/images/" + (Meui.Theme.darkMode ? "dark" : "light") + "/display-brightness-symbolic.svg"
                }
            }

            Item {
                height: Meui.Units.largeSpacing
                visible: brightness.enabled
            }

            HorizontalDivider {
                visible: brightness.enabled
            }

            Label {
                text: qsTr("Scale")
                color: Meui.Theme.disabledTextColor
                bottomPadding: Meui.Units.smallSpacing
            }


            TabBar {
                id: dockSizeTabbar
                Layout.fillWidth: true

                TabButton {
                    text: "100%"
                }

                TabButton {
                    text: "125%"
                }

                TabButton {
                    text: "150%"
                }

                TabButton {
                    text: "175%"
                }

                TabButton {
                    text: "200%"
                }

                currentIndex: {
                    var index = 0

                    if (appearance.devicePixelRatio <= 1.0)
                        index = 0
                    else if (appearance.devicePixelRatio <= 1.25)
                        index = 1
                    else if (appearance.devicePixelRatio <= 1.50)
                        index = 2
                    else if (appearance.devicePixelRatio <= 1.75)
                        index = 3
                    else if (appearance.devicePixelRatio <= 2.0)
                        index = 4

                    return index
                }

                onCurrentIndexChanged: {
                    var value = 1.0

                    switch (currentIndex) {
                    case 0:
                        value = 1.0
                        break;
                    case 1:
                        value = 1.25
                        break;
                    case 2:
                        value = 1.50
                        break;
                    case 3:
                        value = 1.75
                        break;
                    case 4:
                        value = 2.0
                        break;
                    }

                    appearance.setDevicePixelRatio(value)
                }
            }

            Item {
                Layout.fillHeight: true
            }
        }
    }
}
