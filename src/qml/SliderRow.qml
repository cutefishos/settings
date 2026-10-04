import QtQuick
import QtQuick.Layouts
import Cutefish.Settings
import FishUI as FishUI

// A labelled slider row for a ListCard, with optional end icons, captions or a level meter.
ListRow {
    id: sliderRow
    property alias sliderName: _slider.objectName
    property alias from: _slider.from
    property alias to: _slider.to
    property alias stepSize: _slider.stepSize
    property real value
    property alias sliderEnabled: _slider.enabled
    readonly property alias pressed: _slider.pressed
    // Captions flush with the two ends of the track, each under a tick.
    property alias leftText: _captions.leftText
    property alias rightText: _captions.rightText
    // FishUI.Icon names at the two ends, inside the same span as a bare slider.
    property string leftIcon
    property string rightIcon
    // 0 to 1 shows a level meter in place of the slider, on the same track.
    property real meterLevel: -1
    signal moved(real newValue)
    interactive: false
    minimumRowHeight: Metrics.controlRowHeight
    // Centres the label on the slider rather than on the slider and its captions.
    labelBottomMargin: _captions.visible ? _captions.implicitHeight : 0

    // Set for the 300 ms after a move rather than read off the timer: its
    // restart stops it first, which would let the binding put the old
    // value back before the move goes out.
    property bool settling: false

    // A handle drag assigns the slider's value and would break a plain
    // binding. This one also waits while the handle is held and until the
    // server has answered the last move, so the old value never flashes back.
    Binding {
        target: _slider
        property: "value"
        value: sliderRow.value
        when: !_slider.pressed && !sliderRow.settling
        restoreMode: Binding.RestoreNone
    }

    Timer {
        id: _settle
        interval: 300
        onTriggered: sliderRow.settling = false
    }

    // The controls span half the row, the column other pages give their
    // controls: the icons' outer edges, a bare slider's track, the meter's
    // dots and the captions all meet its ends.
    ColumnLayout {
        // A nested layout fills by default and would split the row with the label.
        Layout.fillWidth: false
        Layout.preferredWidth: Math.round(parent.width / 2)
        // The captions make the row too tall for the plain row padding to look even.
        Layout.topMargin: _captions.visible ? Metrics.m : 0
        Layout.bottomMargin: _captions.visible ? Metrics.m : 0
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
                    sliderRow.settling = true
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

        SliderCaptions {
            id: _captions
        }
    }
}
