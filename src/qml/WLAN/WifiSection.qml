import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Cutefish.Settings 1.0
import FishUI 1.0 as FishUI
import "../"

// A titled card of the networks `includes` accepts; hidden while none does.
// (The id avoids "section", which the network model exposes as a role.)
ColumnLayout {
    id: wifiSection

    property string title
    property var model
    // (item) -> bool, given the delegate's model roles.
    property var includes: function(item) { return true }
    // (connectionState) -> the state the rows show.
    property var displayedState: function(state) { return state }
    property int shown: 0

    Layout.fillWidth: true
    spacing: Metrics.sectionLabelSpacing
    visible: shown > 0

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

    SectionLabel {
        text: wifiSection.title
    }

    ListCard {
        Repeater {
            id: repeater
            model: wifiSection.model

            delegate: WifiItem {
                readonly property bool included: wifiSection.includes(model)
                displayedState: wifiSection.displayedState(model.connectionState)
                visible: included
                onIncludedChanged: wifiSection.recount()
                Component.onCompleted: wifiSection.recount()
                Component.onDestruction: wifiSection.recount()
            }
        }
    }
}
