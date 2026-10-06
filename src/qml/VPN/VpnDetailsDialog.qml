import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import Cutefish.Settings
import FishUI as FishUI
import Cutefish.Network as NM
import "../"

// One VPN's type and state, and the way to remove it.
FishUI.Window {
    id: control
    title: row.name || ""

    parentWindow: rootWindow
    x: parentWindow ? parentWindow.x + Math.round((parentWindow.width - width) / 2) : 0
    y: parentWindow ? parentWindow.y + Math.round((parentWindow.height - height) / 2) : 0
    width: 400
    property int contentHeight: detailsLayout.implicitHeight + header.height + 32
    height: contentHeight
    minimumWidth: width
    maximumWidth: width
    minimumHeight: contentHeight
    maximumHeight: contentHeight
    flags: Qt.Dialog | Qt.FramelessWindowHint
    modality: Qt.WindowModal
    visible: false
    minimizeButtonVisible: false
    header.height: 44
    contentTopMargin: header.height
    // The page colour, so the cards stand out as they do on a page.
    background.color: FishUI.Theme.backgroundColor
    // The background item reaches a grown window a frame or two after its buffer does.
    color: background.color

    headerItem: FishUI.Label {
        anchors.left: parent.left
        anchors.leftMargin: Metrics.dialogPadding
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width - 64
        text: control.title
        textFormat: Text.PlainText
        font.pixelSize: 14
        font.weight: Font.DemiBold
        color: FishUI.Theme.textColor
        elide: Text.ElideRight
    }

    // The VPN shown, found by its connection while the list changes around it.
    property alias listModel: tracker.model
    property alias connectionPath: tracker.value
    readonly property var row: tracker.row || ({})

    TrackedRow {
        id: tracker
        role: "connectionPath"
        property bool found: false

        onRowChanged: {
            if (!row) {
                if (found)
                    control.close()
                return
            }

            if (!found && row.vpnUsesUserName && row.vpnUserName.length === 0)
                Qt.callLater(userNameField.forceActiveFocus)
            found = true
        }
    }

    // Created per click, so each one goes away with its window.
    onClosing: {
        if (row.vpnUsesUserName)
            handler.setVpnUserName(control.connectionPath, userNameField.text)
        destroy()
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
            visible: !!row.vpnUsesUserName

            ListRow {
                interactive: false
                text: qsTr("User name")

                FishUI.TextField {
                    id: userNameField
                    Layout.preferredWidth: 200
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                    text: row.vpnUserName || ""
                    placeholderText: qsTr("Required")
                    inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase
                    onAccepted: control.close()
                }
            }
        }

        ListCard {
            InfoRow {
                text: qsTr("Type")
                value: row.vpnType
            }

            InfoRow {
                text: qsTr("Status")
                value: row.connectionState === NM.Enums.Activated ? qsTr("Connected")
                       : row.connectionState === NM.Enums.Activating ? qsTr("Connecting…")
                       : qsTr("Not connected")
            }

            InfoRow {
                visible: row.connectionState === NM.Enums.Activated && !!row.ipAddress
                text: qsTr("IPv4 address")
                value: row.ipAddress
            }
        }

        FishUI.Label {
            Layout.fillWidth: true
            visible: !!row.statusMessage
            text: row.statusMessage || ""
            textFormat: Text.PlainText
            color: FishUI.Theme.disabledTextColor
            wrapMode: Text.Wrap
        }

        RowLayout {
            spacing: Metrics.dialogButtonSpacing
            Layout.alignment: Qt.AlignRight

            FishUI.ConfirmDialogButton {
                text: qsTr("Close")
                onClicked: control.close()
            }

            FishUI.ConfirmDialogButton {
                text: qsTr("Remove VPN")
                primary: true
                destructive: true
                onClicked: {
                    handler.removeConnection(control.connectionPath)
                    control.close()
                }
            }
        }
    }
}
