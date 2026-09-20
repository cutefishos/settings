import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import FishUI 1.0 as FishUI
import Cutefish.Settings 1.0
import "../"

ItemPage {
    headerTitle: qsTr("Input Method")

    InputMethod {
        id: inputMethod
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent
            spacing: FishUI.Units.largeSpacing

            ColumnLayout {
                Layout.fillWidth: true
                spacing: FishUI.Units.smallSpacing

                SectionLabel {
                    text: qsTr("Input method")
                }

                RoundedItem {
                    visible: !inputMethod.available

                    Label {
                        Layout.fillWidth: true
                        text: qsTr("Input methods can only be selected in a Cutefish Wayland session.")
                        color: FishUI.Theme.disabledTextColor
                        textFormat: Text.PlainText
                        wrapMode: Text.WordWrap
                    }
                }

                RoundedItem {
                    visible: inputMethod.available
                    horizontalPadding: FishUI.Units.smallSpacing
                    verticalPadding: FishUI.Units.smallSpacing
                    spacing: 2

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
            }

            Label {
                Layout.fillWidth: true
                visible: inputMethod.available && inputMethod.current !== "" && !inputMethod.running
                text: qsTr("The selected input method is not running.")
                color: FishUI.Theme.disabledTextColor
                textFormat: Text.PlainText
                wrapMode: Text.WordWrap
                leftPadding: FishUI.Units.largeSpacing * 1.5
                rightPadding: FishUI.Units.largeSpacing * 1.5
            }

            Label {
                Layout.fillWidth: true
                visible: inputMethod.available
                text: qsTr("Applications started before the input method may need to be restarted.")
                color: FishUI.Theme.disabledTextColor
                textFormat: Text.PlainText
                wrapMode: Text.WordWrap
                leftPadding: FishUI.Units.largeSpacing * 1.5
                rightPadding: FishUI.Units.largeSpacing * 1.5
                opacity: 0.8
            }
        }
    }
}
