import QtQuick 2.4
import QtQuick.Controls 2.4
import FishUI 1.0 as FishUI

Flickable {
    id: root
    flickableDirection: Flickable.VerticalFlick
    clip: true

    topMargin: FishUI.Units.largeSpacing
    leftMargin: FishUI.Units.largeSpacing * 2
    rightMargin: FishUI.Units.largeSpacing * 2

    contentWidth: width - (leftMargin + rightMargin)

    FishUI.WheelHandler {
        id: wheelHandler
        target: root
    }

    ScrollBar.vertical: ScrollBar {
        bottomPadding: FishUI.Theme.smallRadius
    }
}
