import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cutefish.Settings
import FishUI as FishUI

FishUI.Label {
    Layout.fillWidth: true
    leftPadding: Metrics.cardHorizontalPadding
    rightPadding: Metrics.cardHorizontalPadding
    color: FishUI.Theme.disabledTextColor
    elide: Text.ElideRight
}
