import QtQuick 2.12
import FishUI 1.0 as FishUI

// A card whose rows are ListRows: no inner padding, so a row's hover
// background reaches the card's edge and its corners follow the card's.
RoundedItem {
    spacing: 0
    horizontalPadding: 0
    verticalPadding: 0
}
