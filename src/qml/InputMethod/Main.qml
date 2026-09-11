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
            spacing: FishUI.Units.largeSpacing * 2

            RoundedItem {
                Label {
                    text: qsTr("Input method")
                    color: FishUI.Theme.disabledTextColor
                }

                Label {
                    Layout.fillWidth: true
                    visible: !inputMethod.available
                    text: qsTr("Input methods can only be selected in a Cutefish Wayland session.")
                    wrapMode: Text.WordWrap
                }

                Repeater {
                    model: inputMethod.available
                           ? [{ "id": "", "name": qsTr("None"), "icon": "" }]
                             .concat(inputMethod.methods)
                           : []

                    delegate: MouseArea {
                        id: item

                        property bool isSelected: modelData.id === inputMethod.current

                        Layout.fillWidth: true
                        implicitHeight: Math.max(36, row.implicitHeight + FishUI.Units.largeSpacing)
                        hoverEnabled: true
                        acceptedButtons: Qt.LeftButton
                        onClicked: inputMethod.select(modelData.id)

                        Rectangle {
                            anchors.fill: parent
                            radius: FishUI.Theme.smallRadius
                            color: item.isSelected ? FishUI.Theme.highlightColor
                                                   : item.containsMouse ? FishUI.Theme.disabledTextColor
                                                                        : "transparent"
                            opacity: item.isSelected ? 1 : 0.1
                        }

                        RowLayout {
                            id: row
                            anchors.fill: parent
                            anchors.leftMargin: FishUI.Units.smallSpacing
                            anchors.rightMargin: FishUI.Units.largeSpacing
                            spacing: FishUI.Units.largeSpacing

                            FishUI.IconItem {
                                width: 24
                                height: 24
                                Layout.preferredWidth: 24
                                Layout.preferredHeight: 24
                                source: modelData.icon ? modelData.icon : "input-keyboard"
                                smooth: true
                            }

                            Label {
                                Layout.fillWidth: true
                                text: modelData.name
                                elide: Text.ElideRight
                                color: item.isSelected ? FishUI.Theme.highlightedTextColor
                                                       : FishUI.Theme.textColor
                            }

                            Image {
                                width: 16
                                height: 16
                                sourceSize: Qt.size(16, 16)
                                source: "qrc:/images/dark/checked.svg"
                                visible: item.isSelected
                                smooth: false
                            }
                        }
                    }
                }

                Label {
                    Layout.fillWidth: true
                    visible: inputMethod.available && inputMethod.current !== "" && !inputMethod.running
                    text: qsTr("The selected input method is not running.")
                    color: FishUI.Theme.disabledTextColor
                    wrapMode: Text.WordWrap
                }
            }

            Label {
                Layout.fillWidth: true
                visible: inputMethod.available
                text: qsTr("Applications started before the input method may need to be restarted.")
                color: FishUI.Theme.disabledTextColor
                wrapMode: Text.WordWrap
                leftPadding: FishUI.Units.largeSpacing * 1.5
                rightPadding: FishUI.Units.largeSpacing * 1.5
            }
        }
    }
}
