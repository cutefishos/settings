import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cutefish.Settings
import FishUI as FishUI
import "../"

FishUI.DialogWindow {
    id: control

    property var adapter

    title: adapter ? adapter.name : ""

    width: 400
    contentHeight: detailsLayout.implicitHeight + 32
    // The page colour, so the cards stand out as they do on a page.
    background.color: FishUI.Theme.backgroundColor
    // The background item reaches a grown window a frame or two after its buffer does.
    color: background.color

    // Created per click, so each one goes away with its window.
    onClosing: destroy()

    function rename() {
        if (nameField.text.length > 0 && nameField.text !== adapter.name)
            adapter.name = nameField.text
    }

    ColumnLayout {
        id: detailsLayout
        // Not filled to the window: the window height follows this layout's implicit
        // height a polish later, and a filled layout would be spread over the old height.
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: Metrics.dialogPadding
        spacing: Metrics.dialogSpacing

        ListCard {
            ListRow {
                interactive: false
                minimumRowHeight: Metrics.controlRowHeight
                text: qsTr("Name")

                FishUI.TextField {
                    id: nameField
                    Layout.preferredWidth: 200
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                    text: control.adapter ? control.adapter.name : ""
                    maximumLength: 248
                    selectByMouse: true
                    onAccepted: control.rename()
                }
            }

            InfoRow {
                text: qsTr("Address")
                value: control.adapter ? control.adapter.address : ""
            }
        }

        ListCard {
            ListRow {
                interactive: false
                text: qsTr("Powered")

                FishUI.Switch {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                    rightPadding: 0
                    rightInset: 0
                    checked: !!control.adapter && control.adapter.powered
                    onClicked: control.adapter.powered = checked
                }
            }

            ListRow {
                interactive: false
                text: qsTr("Visible to other devices")

                FishUI.Switch {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                    rightPadding: 0
                    rightInset: 0
                    enabled: !!control.adapter && control.adapter.powered
                    checked: !!control.adapter && control.adapter.discoverable
                    onClicked: control.adapter.discoverable = checked
                }
            }

            ListRow {
                interactive: false
                text: qsTr("Allow incoming pairing")

                FishUI.Switch {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                    rightPadding: 0
                    rightInset: 0
                    enabled: !!control.adapter && control.adapter.powered
                    checked: !!control.adapter && control.adapter.pairable
                    onClicked: control.adapter.pairable = checked
                }
            }
        }

        RowLayout {
            spacing: Metrics.dialogButtonSpacing
            Layout.alignment: Qt.AlignRight

            FishUI.ConfirmDialogButton {
                text: qsTr("Close")
                onClicked: {
                    control.rename()
                    control.close()
                }
            }
        }
    }
}
