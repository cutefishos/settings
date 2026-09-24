import QtQuick 2.12
import FishUI 1.0 as FishUI

// A card whose rows are ListRows: no inner padding, so a row's hover
// background reaches the card's edge and its corners follow the card's.
RoundedItem {
    // Off for cards whose rows already stand apart, such as rows of combo
    // boxes, where lines between them only add noise.
    property bool separatorsVisible: true

    spacing: 0
    horizontalPadding: 0
    verticalPadding: 0
}
