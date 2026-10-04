import QtQuick
import QtQuick.Layouts
import Cutefish.Settings
import FishUI as FishUI

// Captions under a slider, flush with the two ends of its track, each under a tick.
Item {
    id: captions
    property alias leftText: _leftLabel.text
    property alias rightText: _rightLabel.text
    readonly property color tickColor: FishUI.Theme.darkMode ? Qt.rgba(1, 1, 1, 0.55)
                                                             : Qt.rgba(0, 0, 0, 0.35)

    Layout.fillWidth: true
    implicitHeight: _leftTick.height + Metrics.xxs + _leftLabel.implicitHeight
    visible: _leftLabel.text.length > 0 || _rightLabel.text.length > 0

    FishUI.RoundedRectangle {
        id: _leftTick
        x: Math.round(_leftLabel.x + (_leftLabel.width - width) / 2)
        width: 1
        height: 6
        color: captions.tickColor
        visible: _leftLabel.text.length > 0
    }

    FishUI.RoundedRectangle {
        x: Math.round(_rightLabel.x + (_rightLabel.width - width) / 2)
        width: 1
        height: 6
        color: captions.tickColor
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
