import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cutefish.Settings
import FishUI as FishUI
import "../"

// A titled card of the devices in `model`. Empty, it hides, or shows
// `placeholderText` in the card's place when one is set.
ColumnLayout {
    id: deviceSection

    property string title
    property string placeholderText
    property bool busy: false
    property var model
    readonly property int shown: repeater.count

    Layout.fillWidth: true
    spacing: Metrics.sectionLabelSpacing
    visible: shown > 0 || placeholderText.length > 0

    RowLayout {
        Layout.fillWidth: true
        spacing: 0

        SectionLabel {
            Layout.fillWidth: false
            rightPadding: Metrics.m
            text: deviceSection.title
        }

        FishUI.BusyIndicator {
            Layout.preferredWidth: 14
            Layout.preferredHeight: 14
            visible: deviceSection.busy
            running: visible
            duration: 1600
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

            delegate: DeviceItem {}
        }
    }
}
