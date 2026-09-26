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
    // Shared by every slider row, so the sliders and the level meter line up.
    readonly property int sliderWidth: 330

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

    // A built-in card's node is named after the controller ("Ryzen HD Audio
    // Controller Analog Stereo"); its active port says what it is ("Speakers").
    function deviceName(device) {
        var port = activePort(device)
        if (port !== null && port.description.length > 0
                && !device.virtualDevice && device.bus !== "usb" && device.bus !== "bluetooth")
            return port.description
        return device.description
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
        text: page.deviceName(device)
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

            FishUI.Icon {
                anchors.fill: parent
                name: "check"
                color: FishUI.Theme.highlightColor
                visible: deviceRow.checked
            }
        }
    }

    component SliderRow: ListRow {
        id: sliderRow
        property alias sliderName: _slider.objectName
        property alias from: _slider.from
        property alias to: _slider.to
        property alias stepSize: _slider.stepSize
        property real value
        property alias sliderEnabled: _slider.enabled
        // Captions flush with the two ends of the track, each under a tick.
        property alias leftText: _leftLabel.text
        property alias rightText: _rightLabel.text
        // FishUI.Icon names at the two ends, inside the same span as a bare slider.
        property string leftIcon
        property string rightIcon
        // 0 to 1 shows a level meter in place of the slider, on the same track.
        property real meterLevel: -1
        readonly property color tickColor: FishUI.Theme.darkMode ? Qt.rgba(1, 1, 1, 0.55)
                                                                 : Qt.rgba(0, 0, 0, 0.35)
        signal moved(real value)
        interactive: false
        minimumRowHeight: Metrics.controlRowHeight

        // A handle drag assigns the slider's value and would break a plain
        // binding. This one also waits while the handle is held and until the
        // server has answered the last move, so the old value never flashes back.
        Binding {
            target: _slider
            property: "value"
            value: sliderRow.value
            when: !_slider.pressed && !_settle.running
            restoreMode: Binding.RestoreNone
        }

        Timer {
            id: _settle
            interval: 300
        }

        // The controls span exactly sliderWidth: the icons' outer edges, a bare
        // slider's track, the meter's dots and the captions all meet its ends.
        ColumnLayout {
            // A nested layout fills by default and would split the row with the label.
            Layout.fillWidth: false
            Layout.preferredWidth: page.sliderWidth
            // Room above to match the captions below, so the slider and not the
            // whole column is centred on the label.
            Layout.topMargin: _captions.visible ? _captions.implicitHeight : 0
            spacing: 0

            Item {
                Layout.fillWidth: true
                implicitHeight: _slider.implicitHeight

                FishUI.Icon {
                    id: _leftIcon
                    anchors.left: parent.left
                    // The speaker glyphs start 1px inside their box but end at its edge.
                    anchors.leftMargin: -1
                    anchors.verticalCenter: parent.verticalCenter
                    visible: sliderRow.leftIcon.length > 0
                    name: sliderRow.leftIcon
                }

                FishUI.Icon {
                    id: _rightIcon
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    visible: sliderRow.rightIcon.length > 0
                    name: sliderRow.rightIcon
                }

                FishUI.Slider {
                    id: _slider
                    // Where the visible track starts and ends inside the slider.
                    readonly property real trackInset: (thumbLength - sliderTrackHeight) / 2
                    anchors.left: _leftIcon.visible ? _leftIcon.right : parent.left
                    anchors.right: _rightIcon.visible ? _rightIcon.left : parent.right
                    // Negative: the handle overhangs the track ends by trackInset.
                    anchors.leftMargin: (_leftIcon.visible ? Metrics.s : 0) - leftPadding - trackInset
                    anchors.rightMargin: (_rightIcon.visible ? Metrics.s : 0) - rightPadding - trackInset
                    anchors.verticalCenter: parent.verticalCenter
                    visible: sliderRow.meterLevel < 0
                    Accessible.name: sliderRow.text
                    onMoved: {
                        _settle.restart()
                        sliderRow.moved(value)
                    }
                }

                Row {
                    id: _meter
                    readonly property int segments: 15
                    readonly property int lit: Math.round(Math.max(0, sliderRow.meterLevel) * segments)
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: (width - segments * 8) / (segments - 1)
                    visible: sliderRow.meterLevel >= 0
                    Accessible.role: Accessible.ProgressBar
                    Accessible.name: sliderRow.text

                    Repeater {
                        model: _meter.segments
                        delegate: Rectangle {
                            required property int index
                            width: 8
                            height: 8
                            radius: 4
                            color: index < _meter.lit ? FishUI.Theme.highlightColor
                                                      : FishUI.Theme.disabledTextColor
                            opacity: index < _meter.lit ? 1 : 0.3
                        }
                    }
                }
            }

            Item {
                id: _captions
                Layout.fillWidth: true
                implicitHeight: _leftTick.height + Metrics.xxs + _leftLabel.implicitHeight
                visible: _leftLabel.text.length > 0 || _rightLabel.text.length > 0

                FishUI.RoundedRectangle {
                    id: _leftTick
                    x: Math.round(_leftLabel.x + (_leftLabel.width - width) / 2)
                    width: 1
                    height: 3
                    color: sliderRow.tickColor
                    visible: _leftLabel.text.length > 0
                }

                FishUI.RoundedRectangle {
                    x: Math.round(_rightLabel.x + (_rightLabel.width - width) / 2)
                    width: 1
                    height: 3
                    color: sliderRow.tickColor
                    visible: _rightLabel.text.length > 0
                }

                FishUI.Label {
                    id: _leftLabel
                    y: _leftTick.height + Metrics.xxs
                    font.pixelSize: 11
                    color: FishUI.Theme.disabledTextColor
                }

                FishUI.Label {
                    id: _rightLabel
                    x: Math.round(parent.width - width)
                    y: _leftTick.height + Metrics.xxs
                    font.pixelSize: 11
                    color: FishUI.Theme.disabledTextColor
                }
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
                        onMoved: (value) => {
                            page.outputDevice.volume = value
                            page.outputDevice.muted = value === 0
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
                        onMoved: (value) => page.setBalance(page.outputDevice, value)
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
                        onMoved: (value) => {
                            page.inputDevice.volume = value
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
