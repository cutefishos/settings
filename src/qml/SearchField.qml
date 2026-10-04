import QtQuick
import QtQuick.Layouts
import FishUI as FishUI

// A search field above a ListCard, on the page colour.
FishUI.TextField {
    id: control

    Layout.fillWidth: true
    selectByMouse: true

    // The default fill is drawn for card surfaces and all but disappears on
    // the page colour.
    Binding {
        target: control.background
        property: "color"
        value: FishUI.Theme.secondBackgroundColor
    }
}
