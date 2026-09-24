import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cutefish.Settings 1.0
import FishUI 1.0 as FishUI
import "../"

// A titled card of the devices `includes` accepts. Empty, it hides, or shows
// `placeholderText` in the card's place when one is set.
ColumnLayout {
    id: deviceSection

    property string title
    property string placeholderText
    property bool busy: false
    property var model
    // (device) -> bool, given the device object.
    property var includes: function(item) { return true }
    property int shown: 0

    Layout.fillWidth: true
    spacing: Metrics.sectionLabelSpacing
    visible: shown > 0 || placeholderText.length > 0

    // Counts the rows' own verdicts, not their visibility: while the card is
    // hidden for being empty every row is invisible too.
    function recount() {
        var count = 0
        for (var i = 0; i < repeater.count; ++i) {
            var item = repeater.itemAt(i)
            if (item && item.included)
                ++count
        }
        shown = count
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: 0

        SectionLabel {
            Layout.fillWidth: false
            text: deviceSection.title
        }

        FishUI.BusyIndicator {
            Layout.preferredWidth: 14
            Layout.preferredHeight: 14
            visible: deviceSection.busy
            running: visible
        }

        Item {
            Layout.fillWidth: true
        }
    }

    FishUI.Label {
        Layout.fillWidth: true
        leftPadding: Metrics.cardHorizontalPadding
        rightPadding: Metrics.cardHorizontalPadding
        visible: deviceSection.shown === 0
        text: deviceSection.placeholderText
        color: FishUI.Theme.disabledTextColor
        wrapMode: Text.Wrap
    }

    ListCard {
        visible: deviceSection.shown > 0

        Repeater {
            id: repeater
            model: deviceSection.model

            delegate: DeviceItem {
                readonly property bool included: deviceSection.includes(device)
                visible: included
                onIncludedChanged: deviceSection.recount()
                Component.onCompleted: deviceSection.recount()
                Component.onDestruction: deviceSection.recount()
            }
        }
    }
}
