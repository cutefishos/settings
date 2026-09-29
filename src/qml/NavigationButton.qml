import QtQuick 2.12
import FishUI 1.0 as FishUI

// Same plate as the file manager's header buttons, so both headers line up.
FishUI.HeaderButton {
    iconSize: 16
    hoveredColor: Qt.rgba(tint.r, tint.g, tint.b, darkMode ? 0.21 : 0.17)
    pressedColor: Qt.rgba(tint.r, tint.g, tint.b, darkMode ? 0.15 : 0.26)
}
