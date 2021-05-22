import QtQuick 2.12
import FishUI 1.0 as FishUI
import Cutefish.Screen 1.0 as CS
import "../"

IconCheckBox {
    id: control

    property int value
    property var rot

    checked: element.rotation === rot

    onClicked: {
        if (element.rotation === rot) {
            return;
        }

        element.rotation = rot
        // screen.resetTotalSize()
        screen.save()
    }

    Component.onCompleted: {
        switch(value) {
        case 90:
            rot = CS.Output.Left
            control.source = "qrc:/images/rot90.svg";
            break;
        case 180:
            control.source = "qrc:/images/rot180.svg";
            rot = CS.Output.Inverted;
            break;
        case 270:
            control.source = "qrc:/images/rot270.svg";
            rot = CS.Output.Right;
            break;
        case 0:
        default:
            control.source = "qrc:/images/rotnormal.svg";
            rot = CS.Output.None;
        }
    }
}
