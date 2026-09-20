import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FishUI 1.0 as FishUI
import Cutefish.Settings 1.0
import "../"

ItemPage {
    headerTitle: qsTr("Privacy")

    ScreenRecordingModel {
        id: screenRecording
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent

            SectionLabel {
                text: qsTr("Screen Recording")
            }

            RoundedItem {
                Label {
                    Layout.fillWidth: true
                    text: qsTr("Allow the applications below to record the contents of your screen, even while using other applications.")
                    wrapMode: Text.Wrap
                    color: FishUI.Theme.disabledTextColor
                }

                Label {
                    Layout.fillWidth: true
                    // rowCount() is a plain call; the repeater's count is a binding.
                    visible: applications.count === 0
                    text: qsTr("No application has asked to record the screen yet.")
                    wrapMode: Text.Wrap
                }

                Repeater {
                    id: applications
                    model: screenRecording

                    delegate: ColumnLayout {
                        Layout.fillWidth: true
                        spacing: FishUI.Units.largeSpacing

                        HorizontalDivider {
                            visible: index > 0
                            height: 1
                        }

                        RowLayout {
                            spacing: FishUI.Units.largeSpacing

                            FishUI.IconItem {
                                Layout.preferredWidth: 32
                                Layout.preferredHeight: 32
                                source: model.icon ? model.icon : "application-x-executable"
                            }

                            Label {
                                Layout.fillWidth: true
                                text: model.name
                                elide: Text.ElideRight
                                textFormat: Text.PlainText
                            }

                            Switch {
                                checked: model.allowed
                                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                                rightPadding: 0
                                rightInset: 0
                                onClicked: screenRecording.setAllowed(index, checked)
                            }
                        }
                    }
                }
            }
        }
    }
}
