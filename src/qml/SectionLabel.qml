import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import FishUI 1.0 as FishUI

Label {
    Layout.fillWidth: true
    leftPadding: FishUI.Units.largeSpacing * 1.5
    rightPadding: FishUI.Units.largeSpacing * 1.5
    bottomPadding: FishUI.Units.smallSpacing
    color: FishUI.Theme.disabledTextColor
    elide: Text.ElideRight
}
