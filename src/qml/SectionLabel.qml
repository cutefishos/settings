import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Cutefish.Settings 1.0
import FishUI 1.0 as FishUI

FishUI.Label {
    Layout.fillWidth: true
    leftPadding: Metrics.cardHorizontalPadding
    rightPadding: Metrics.cardHorizontalPadding
    color: FishUI.Theme.disabledTextColor
    elide: Text.ElideRight
}
