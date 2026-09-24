import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cutefish.Settings 1.0
import FishUI 1.0 as FishUI
import Cutefish.Audio 1.0
import "../"

ItemPage {
    id: page
    headerTitle: qsTr("Sound")
    property string operationError: ""

    SinkModel { id: outputs }
    SourceModel { id: inputs }
    CardModel { id: cards }

    Connections {
        target: AudioService
        function onOperationFailed(message) {
            page.operationError = message
        }
        function onConnectedChanged() {
            page.operationError = ""
        }
    }

    component VolumeControl: RowLayout {
        id: volumeControl
        required property var device
        spacing: Metrics.rowSpacing

        FishUI.Slider {
            objectName: "volume-" + volumeControl.device.index
            Layout.fillWidth: true
            from: PipeWire.MinimalVolume
            to: PipeWire.NormalVolume
            stepSize: 0.01
            value: volumeControl.device.muted ? 0 : volumeControl.device.volume
            enabled: volumeControl.device.volumeWritable
            Accessible.name: qsTr("Volume for %1").arg(volumeControl.device.description)
            onMoved: {
                volumeControl.device.volume = value
                volumeControl.device.muted = value === 0
            }
        }

        FishUI.Label {
            Layout.preferredWidth: 48
            horizontalAlignment: Text.AlignRight
            text: qsTr("%1%").arg(volumeControl.device.muted ? 0 : Math.round(volumeControl.device.volume * 100))
        }
    }

    component DeviceCard: RoundedItem {
        id: deviceCard
        required property var device
        property bool input: false
        objectName: "device-" + device.index

        RowLayout {
            Layout.fillWidth: true
            spacing: Metrics.rowSpacing

            FishUI.Label {
                Layout.fillWidth: true
                text: deviceCard.device.description
                textFormat: Text.PlainText
                wrapMode: Text.Wrap
            }

            FishUI.Button {
                objectName: "default-" + deviceCard.device.index
                Layout.preferredWidth: 88
                Layout.preferredHeight: 28
                leftPadding: 10
                rightPadding: 10
                topPadding: 3
                bottomPadding: 3
                font.pixelSize: 12
                text: deviceCard.device.default ? qsTr("Default") : qsTr("Set default")
                enabled: !deviceCard.device.default
                Accessible.name: deviceCard.input ? qsTr("Use as default input") : qsTr("Use as default output")
                onClicked: {
                    page.operationError = ""
                    deviceCard.device.default = true
                }
            }
        }

        VolumeControl {
            Layout.fillWidth: true
            device: deviceCard.device
            visible: deviceCard.device.hasVolume
        }

        RowLayout {
            Layout.fillWidth: true
            visible: deviceCard.device.ports.length > 1
            spacing: Metrics.rowSpacing

            FishUI.Label {
                text: qsTr("Port")
            }

            FishUI.ComboBox {
                Layout.fillWidth: true
                model: deviceCard.device.ports
                textRole: "description"
                currentIndex: deviceCard.device.activePortIndex
                Accessible.name: qsTr("Port for %1").arg(deviceCard.device.description)
                onActivated: {
                    page.operationError = ""
                    deviceCard.device.activePortIndex = currentIndex
                }
                delegate: ItemDelegate {
                    required property var modelData
                    width: parent ? parent.width : implicitWidth
                    text: modelData.description
                    enabled: modelData.availability !== AudioObject.Unavailable
                }
            }
        }

        RowLayout {
            spacing: Metrics.rowSpacing
            Layout.fillWidth: true
            visible: deviceCard.input && deviceCard.device.default

            FishUI.Label {
                text: qsTr("Input level")
                color: FishUI.Theme.disabledTextColor
            }

            FishUI.ProgressBar {
                Layout.fillWidth: true
                from: 0
                to: 1
                value: inputMonitor.volume
                Accessible.name: qsTr("Microphone input level")
            }
        }

        VolumeMonitor {
            id: inputMonitor
            target: page.visible && deviceCard.input && deviceCard.device.default
                    ? deviceCard.device : null
        }
    }

    Scrollable {
        id: scroll
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            width: scroll.contentWidth
            spacing: Metrics.sectionSpacing

            RoundedItem {
                visible: !AudioService.connected

                FishUI.Label {
                    Layout.fillWidth: true
                    text: qsTr("Connecting to the audio service…")
                    wrapMode: Text.Wrap
                }
            }

            RoundedItem {
                visible: page.operationError.length > 0

                FishUI.Label {
                    Layout.fillWidth: true
                    text: qsTr("Could not apply the audio change: %1").arg(page.operationError)
                    textFormat: Text.PlainText
                    wrapMode: Text.Wrap
                }

                FishUI.Button {
                    text: qsTr("Dismiss")
                    onClicked: page.operationError = ""
                }
            }

            ColumnLayout {
                objectName: "output-section"
                Layout.fillWidth: true
                spacing: Metrics.sectionLabelSpacing

                SectionLabel { text: qsTr("Output devices") }

                FishUI.Label {
                    objectName: "no-outputs"
                    Layout.fillWidth: true
                    visible: AudioService.connected && outputs.count === 0
                    text: qsTr("No output devices connected")
                    color: FishUI.Theme.disabledTextColor
                    wrapMode: Text.Wrap
                }

                Repeater {
                    model: outputs
                    delegate: DeviceCard {
                        required property var model
                        device: model.AudioObject
                    }
                }
            }

            ColumnLayout {
                objectName: "input-section"
                Layout.fillWidth: true
                spacing: Metrics.sectionLabelSpacing

                SectionLabel { text: qsTr("Input devices") }

                FishUI.Label {
                    objectName: "no-inputs"
                    Layout.fillWidth: true
                    visible: AudioService.connected && inputs.count === 0
                    text: qsTr("No input devices connected")
                    color: FishUI.Theme.disabledTextColor
                    wrapMode: Text.Wrap
                }

                Repeater {
                    model: inputs
                    delegate: DeviceCard {
                        required property var model
                        device: model.AudioObject
                        input: true
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: Metrics.sectionLabelSpacing
                visible: cards.count > 0

                SectionLabel { text: qsTr("Device profiles") }

                RoundedItem {
                    Repeater {
                        model: cards
                        delegate: ColumnLayout {
                            id: profileCard
                            required property var model
                            required property int index
                            readonly property var device: model.AudioObject
                            Layout.fillWidth: true
                            spacing: Metrics.cardSpacing
                            visible: device.profiles.length > 0

                            FishUI.Label {
                                Layout.fillWidth: true
                                text: profileCard.device.description
                                textFormat: Text.PlainText
                                wrapMode: Text.Wrap
                            }

                            FishUI.ComboBox {
                                Layout.fillWidth: true
                                model: profileCard.device.profiles
                                textRole: "description"
                                currentIndex: profileCard.device.activeProfileIndex
                                Accessible.name: qsTr("Profile for %1").arg(profileCard.device.description)
                                onActivated: {
                                    page.operationError = ""
                                    profileCard.device.activeProfileIndex = currentIndex
                                }
                                delegate: ItemDelegate {
                                    required property var modelData
                                    width: parent ? parent.width : implicitWidth
                                    text: modelData.description
                                    enabled: modelData.availability !== AudioObject.Unavailable
                                }
                            }

                            HorizontalDivider {
                                visible: profileCard.index < cards.count - 1
                            }
                        }
                    }
                }
            }
        }
    }
}
