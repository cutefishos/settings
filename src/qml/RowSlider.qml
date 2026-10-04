import QtQuick
import QtQuick.Layouts
import FishUI as FishUI

// A slider whose visible track, not its handle's travel, spans its place in a
// row, as on the Sound page: the track ends where switches and combo boxes do,
// and the handle overhangs those ends.
FishUI.Slider {
    //! The width the track spans when the slider does not fill the row.
    property real trackWidth
    readonly property real overhang: leftPadding + (thumbLength - sliderTrackHeight) / 2

    Layout.preferredWidth: trackWidth + overhang * 2
    Layout.leftMargin: -overhang
    Layout.rightMargin: -overhang
}
