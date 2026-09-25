import QtQuick
import FishUI 1.0 as FishUI

// One device pixel at any scale. RoundedRectangle snaps it to the pixel grid;
// a plain Rectangle at 150% would be one or two pixels depending on where it lands.
FishUI.RoundedRectangle {
    height: 1 / FishUI.Dpi.ratio
    color: FishUI.Theme.disabledTextColor
    opacity: FishUI.Theme.darkMode ? 0.3 : 0.1
}
