import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cutefish.Settings
import FishUI as FishUI
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
    // A last row that is always there, e.g. for joining a hidden network.
    property string footerText
    signal footerClicked()

    Layout.fillWidth: true
    spacing: Metrics.sectionLabelSpacing
    visible: shown > 0 || footerText.length > 0

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
                listModel: repeater.model
                readonly property bool included: wifiSection.includes(model)
                displayedState: wifiSection.displayedState(model.connectionState)
                visible: included
                onIncludedChanged: wifiSection.recount()
                Component.onCompleted: wifiSection.recount()
                Component.onDestruction: wifiSection.recount()
            }
        }

        // Declared after the Repeater, which inserts its rows before itself.
        ListRow {
            visible: wifiSection.footerText.length > 0
            text: wifiSection.footerText
            onClicked: wifiSection.footerClicked()

            // Lines the text up with the network names beside their icons.
            leading: Item {
                width: 18
                height: width
            }
        }
    }
}
