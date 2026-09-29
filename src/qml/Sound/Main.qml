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

    readonly property bool showInputs: tabs.currentIndex === 1
    readonly property var outputDevice: outputs.defaultSink
    readonly property var inputDevice: inputs.defaultSource

    SinkModel { id: outputs }
    SourceModel { id: inputs }

    Connections {
        target: AudioService
        function onOperationFailed(message) {
            page.operationError = message
        }
        function onConnectedChanged() {
            page.operationError = ""
        }
    }

    function activePort(device) {
        var ports = device.ports
        var index = device.activePortIndex
        return index >= 0 && index < ports.length ? ports[index] : null
    }

    function deviceType(device) {
        if (device.virtualDevice)
            return qsTr("Virtual")
        if (device.bus === "bluetooth")
            return qsTr("Bluetooth")
        if (device.bus === "usb")
            return qsTr("USB")
        var port = activePort(device)
        if (port !== null && port.name.indexOf("hdmi") === 0)
            return qsTr("HDMI")
        return qsTr("Built-in")
    }

    // -1 is all left, 1 all right. Only a device with both front channels has one.
    function balance(device) {
        var left = device.channels.indexOf("FL")
        var right = device.channels.indexOf("FR")
        if (left < 0 || right < 0)
            return 0
        var volumes = device.channelVolumes
        var high = Math.max(volumes[left], volumes[right])
        if (high <= 0)
            return 0
        return volumes[left] >= volumes[right] ? volumes[right] / high - 1
                                               : 1 - volumes[left] / high
    }

    function setBalance(device, value) {
        var channels = device.channels
        var current = device.channelVolumes
        var volume = device.volume
        // A copy: assigning into the property's own list writes each element
        // back at once, and later reads return the device's not yet updated value.
        var volumes = []
        for (var i = 0; i < channels.length; ++i) {
            if (channels[i] === "FL")
                volumes.push(volume * Math.min(1, 1 - value))
            else if (channels[i] === "FR")
                volumes.push(volume * Math.min(1, 1 + value))
            else
                volumes.push(current[i])
        }
        device.channelVolumes = volumes
    }

    component DeviceRow: ListRow {
        id: deviceRow
        required property var device
        objectName: "device-" + device.index
        text: device.displayName
        checked: device.default

        Accessible.role: Accessible.RadioButton
        Accessible.checked: checked

        onClicked: {
            if (device.default)
                return
            page.operationError = ""
            device.default = true
        }

        FishUI.Label {
            text: page.deviceType(deviceRow.device)
            color: FishUI.Theme.disabledTextColor
        }

        // Keeps the type aligned whether or not the row is checked.
        Item {
            Layout.preferredWidth: 18
            Layout.preferredHeight: 18
            Layout.rightMargin: Metrics.trailingGlyphMargin("check", 18)

            FishUI.Icon {
                anchors.fill: parent
                name: "check"
                color: FishUI.Theme.highlightColor
                visible: deviceRow.checked
            }
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
                Layout.fillWidth: true
                spacing: Metrics.l

                FishUI.SegmentedControl {
                    id: tabs
                    objectName: "tabs"
                    Layout.fillWidth: true

                    FishUI.TabButton {
                        objectName: "output-tab"
                        text: qsTr("Output")
                    }

                    FishUI.TabButton {
                        objectName: "input-tab"
                        text: qsTr("Input")
                    }
                }

                ListCard {
                    Repeater {
                        model: page.showInputs ? inputs : outputs
                        delegate: DeviceRow {
                            required property var model
                            device: model.AudioObject
                        }
                    }

                    ListRow {
                        interactive: false
                        visible: AudioService.connected && (page.showInputs ? inputs.count === 0
                                                                            : outputs.count === 0)
                        text: page.showInputs ? qsTr("No input devices") : qsTr("No output devices")
                        enabled: false
                    }
                }

                ListCard {
                    // Bound to the device, not to the rows: a row's visible is
                    // false while its card is hidden, so the card could never show.
                    visible: page.showInputs ? page.inputDevice !== null
                                             : page.outputDevice !== null && page.outputDevice.hasVolume

                    SliderRow {
                        text: qsTr("Output volume")
                        leftIcon: "audio-volume-zero"
                        rightIcon: "audio-volume-high"
                        visible: !page.showInputs && page.outputDevice !== null && page.outputDevice.hasVolume
                        sliderName: page.outputDevice ? "volume-" + page.outputDevice.index : ""
                        from: PipeWire.MinimalVolume
                        to: PipeWire.NormalVolume
                        stepSize: 0.01
                        // With no mute switch, muting shows as zero here.
                        value: page.outputDevice && !page.outputDevice.muted ? page.outputDevice.volume : 0
                        sliderEnabled: page.outputDevice !== null && page.outputDevice.volumeWritable
                        onMoved: (newValue) => {
                            page.outputDevice.volume = newValue
                            page.outputDevice.muted = newValue === 0
                        }
                    }

                    SliderRow {
                        objectName: "balance"
                        text: qsTr("Balance")
                        leftText: qsTr("Left")
                        rightText: qsTr("Right")
                        visible: !page.showInputs && page.outputDevice !== null
                                 && page.outputDevice.channels.indexOf("FL") >= 0
                                 && page.outputDevice.channels.indexOf("FR") >= 0
                        from: -1
                        to: 1
                        stepSize: 0.02
                        value: page.outputDevice ? page.balance(page.outputDevice) : 0
                        sliderEnabled: page.outputDevice !== null && page.outputDevice.volumeWritable
                        onMoved: (newValue) => page.setBalance(page.outputDevice, newValue)
                    }

                    SliderRow {
                        text: qsTr("Input volume")
                        visible: page.showInputs && page.inputDevice !== null && page.inputDevice.hasVolume
                        sliderName: page.inputDevice ? "volume-" + page.inputDevice.index : ""
                        from: PipeWire.MinimalVolume
                        to: PipeWire.NormalVolume
                        stepSize: 0.01
                        value: page.inputDevice ? page.inputDevice.volume : 0
                        sliderEnabled: page.inputDevice !== null && page.inputDevice.volumeWritable
                        onMoved: (newValue) => {
                            page.inputDevice.volume = newValue
                            page.inputDevice.muted = false
                        }
                    }

                    SliderRow {
                        text: qsTr("Input level")
                        visible: page.showInputs && page.inputDevice !== null
                        // Peaks in dB over the bottom 50 dB, so speech fills about half.
                        meterLevel: inputMonitor.volume > 0
                                    ? Math.max(0, 1 + Math.log(inputMonitor.volume) / Math.LN10 * 20 / 50)
                                    : 0

                        VolumeMonitor {
                            id: inputMonitor
                            target: page.visible && page.showInputs ? page.inputDevice : null
                        }
                    }
                }
            }
        }
    }
}
