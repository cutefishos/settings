import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import FishUI 1.0 as FishUI
import Cutefish.Settings 1.0
import "../"

ItemPage {
    id: page
    headerTitle: qsTr("Keyboard")

    // Slider stops, slowest first, in milliseconds.
    readonly property var repeatDelays: [1500, 1000, 750, 500, 400, 300, 250, 200, 150]
    readonly property var repeatIntervals: [500, 250, 100, 67, 50, 40, 33, 25, 20]

    function nearestStop(stops, value) {
        var best = 0
        for (var i = 1; i < stops.length; ++i) {
            if (Math.abs(stops[i] - value) < Math.abs(stops[best] - value))
                best = i
        }
        return best
    }

    InputMethod {
        id: inputMethod
    }

    Keyboard {
        id: keyboard
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent
            spacing: Metrics.sectionSpacing

            ColumnLayout {
                Layout.fillWidth: true
                spacing: Metrics.sectionLabelSpacing

                SectionLabel {
                    text: qsTr("Input method")
                }

                RoundedItem {
                    visible: !inputMethod.available

                    FishUI.Label {
                        Layout.fillWidth: true
                        text: qsTr("Input methods can only be selected in a Cutefish Wayland session.")
                        color: FishUI.Theme.disabledTextColor
                        textFormat: Text.PlainText
                        wrapMode: Text.WordWrap
                    }
                }

                ListCard {
                    visible: inputMethod.available

                    Repeater {
                        model: [{ "id": "", "name": qsTr("None"), "icon": "" }]
                               .concat(inputMethod.methods)

                        delegate: InputMethodRow {
                            text: modelData.name
                            iconSource: modelData.icon
                            checked: modelData.id === inputMethod.current
                            onClicked: inputMethod.select(modelData.id)
                        }
                    }
                }

                FishUI.Label {
                    Layout.fillWidth: true
                    visible: inputMethod.available && inputMethod.current !== "" && !inputMethod.running
                    text: qsTr("The selected input method is not running.")
                    color: FishUI.Theme.disabledTextColor
                    textFormat: Text.PlainText
                    wrapMode: Text.WordWrap
                    leftPadding: Metrics.cardHorizontalPadding
                    rightPadding: Metrics.cardHorizontalPadding
                }

                FishUI.Label {
                    Layout.fillWidth: true
                    visible: inputMethod.available
                    text: qsTr("Applications started before the input method may need to be restarted.")
                    color: FishUI.Theme.disabledTextColor
                    textFormat: Text.PlainText
                    wrapMode: Text.WordWrap
                    leftPadding: Metrics.cardHorizontalPadding
                    rightPadding: Metrics.cardHorizontalPadding
                    opacity: 0.8
                }
            }

            ListCard {
                visible: keyboard.available

                ListRow {
                    interactive: false
                    text: qsTr("Key repeat")

                    FishUI.Switch {
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                        checked: keyboard.repeatEnabled
                        rightPadding: 0
                        rightInset: 0
                        onToggled: keyboard.repeatEnabled = checked
                    }
                }

                ListRow {
                    enabled: keyboard.repeatEnabled
                    interactive: false
                    text: qsTr("Delay until repeat")

                    FishUI.Slider {
                        // The label fills the rest of the row.
                        Layout.preferredWidth: Math.round(parent.width / 2)
                        from: 0
                        to: page.repeatDelays.length - 1
                        stepSize: 1
                        value: page.nearestStop(page.repeatDelays, keyboard.repeatDelay)
                        onMoved: keyboard.repeatDelay = page.repeatDelays[Math.round(value)]
                    }
                }

                ListRow {
                    enabled: keyboard.repeatEnabled
                    interactive: false
                    text: qsTr("Repeat rate")

                    FishUI.Slider {
                        // The label fills the rest of the row.
                        Layout.preferredWidth: Math.round(parent.width / 2)
                        from: 0
                        to: page.repeatIntervals.length - 1
                        stepSize: 1
                        value: page.nearestStop(page.repeatIntervals, keyboard.repeatInterval)
                        onMoved: keyboard.repeatInterval = page.repeatIntervals[Math.round(value)]
                    }
                }
            }

            ListCard {
                NavigationItem {
                    visible: keyboard.available
                    text: qsTr("Layout")
                    value: keyboard.layoutName
                    onClicked: rootWindow.pushPage("qrc:/qml/Keyboard/LayoutPage.qml")
                }

                NavigationItem {
                    text: qsTr("Keyboard Shortcuts")
                    onClicked: rootWindow.pushPage("qrc:/qml/Shortcuts/Main.qml")
                }
            }
        }
    }
}
