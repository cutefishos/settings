import QtQuick 2.4
import QtQuick.Controls 2.4
import MeuiKit 1.0 as Meui

Flickable {
    id: root
    flickableDirection: Flickable.VerticalFlick
    clip: true

    topMargin: Meui.Units.largeSpacing
    leftMargin: Meui.Units.largeSpacing * 2
    rightMargin: Meui.Units.largeSpacing * 2

    contentWidth: width - (leftMargin + rightMargin)

    ScrollBar.vertical: ScrollBar {}
}
