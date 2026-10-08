import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import Cutefish.Settings
import FishUI as FishUI
import Cutefish.Network as NM
import "../"

// One VPN's settings, state, and the ways to export or remove it. Changes are kept
// with Save; a password shown empty is one that is asked for when connecting.
FishUI.Window {
    id: control
    title: row.name || ""

    parentWindow: rootWindow
    x: parentWindow ? parentWindow.x + Math.round((parentWindow.width - width) / 2) : 0
    y: parentWindow ? parentWindow.y + Math.round((parentWindow.height - height) / 2) : 0
    width: 460
    property int contentHeight: detailsLayout.implicitHeight + header.height + 2 * Metrics.dialogPadding
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

    // A long form scrolls instead of outgrowing the main window.
    readonly property int maxFormHeight: (parentWindow ? parentWindow.height : 600) - 2 * Metrics.xxl - header.height
                                         - 2 * Metrics.dialogPadding - _statusCard.implicitHeight - _buttons.implicitHeight
                                         - (_messageLabel.visible ? _messageLabel.implicitHeight + Metrics.dialogSpacing : 0)
                                         - 2 * Metrics.dialogSpacing

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

    property bool saving: false
    property string error

    TrackedRow {
        id: tracker
        role: "connectionPath"
        property bool found: false

        onRowChanged: {
            if (row)
                found = true
            else if (found)
                control.close()
        }
    }

    Component.onCompleted: {
        _editor.load(handler.vpnForm("", connectionPath))
        handler.loadVpnSecrets(connectionPath)
    }

    // Created per click, so each one goes away with its window.
    onClosing: destroy()

    function save() {
        if (saving || !_editor.complete)
            return
        saving = true
        error = ""
        handler.saveVpn(connectionPath, _editor.collect())
    }

    Connections {
        target: handler

        function onVpnSecretsLoaded(connection, secrets) {
            if (connection === control.connectionPath)
                _editor.setSecrets(secrets)
        }

        function onVpnSaved(connection) {
            if (connection !== control.connectionPath || !control.saving)
                return
            control.saving = false
            control.close()
        }

        function onVpnSaveFailed(message) {
            if (!control.saving)
                return
            control.saving = false
            control.error = message
        }

        function onVpnExportFailed(message) {
            control.error = message
        }
    }

    FileDialog {
        id: exportDialog
        title: qsTr("Export VPN Configuration")
        fileMode: FileDialog.SaveFile
        nameFilters: [qsTr("VPN configurations (%1)").arg((_editor.form.exportPatterns ?? []).join(" ")), qsTr("All files (*)")]
        onAccepted: {
            control.error = ""
            handler.exportVpn(control.connectionPath, selectedFile)
        }
    }

    Shortcut {
        sequence: "Escape"
        onActivated: control.close()
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

        RoundedItem {
            verticalPadding: Metrics.cardHorizontalPadding

            Flickable {
                id: _form
                Layout.fillWidth: true
                // Spans the padding so the bar runs along the card's edge, not the fields'.
                Layout.leftMargin: -Metrics.cardHorizontalPadding
                Layout.rightMargin: -Metrics.cardHorizontalPadding
                Layout.preferredHeight: Math.min(_editor.implicitHeight, control.maxFormHeight)
                clip: true
                contentWidth: width
                contentHeight: _editor.implicitHeight
                boundsBehavior: Flickable.StopAtBounds
                FishUI.ScrollBar.vertical: FishUI.ScrollBar {}
                FishUI.WheelHandler { target: _form }

                VpnEditor {
                    id: _editor
                    x: Metrics.cardHorizontalPadding
                    width: _form.width - 2 * Metrics.cardHorizontalPadding
                    onAccepted: control.save()
                }
            }
        }

        ListCard {
            id: _statusCard

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

            ListRow {
                visible: !!_editor.form.exportable
                text: qsTr("Export to file…")
                onClicked: exportDialog.open()
            }
        }

        FishUI.Label {
            id: _messageLabel
            Layout.fillWidth: true
            visible: control.error.length > 0 || !!row.statusMessage
            text: control.error.length > 0 ? control.error : (row.statusMessage || "")
            textFormat: Text.PlainText
            color: control.error.length > 0 ? FishUI.Theme.redColor : FishUI.Theme.disabledTextColor
            wrapMode: Text.Wrap
        }

        RowLayout {
            id: _buttons
            spacing: Metrics.dialogButtonSpacing
            Layout.alignment: Qt.AlignRight

            FishUI.ConfirmDialogButton {
                text: qsTr("Remove VPN")
                destructive: true
                onClicked: {
                    handler.removeConnection(control.connectionPath)
                    control.close()
                }
            }

            Item { Layout.fillWidth: true }

            FishUI.ConfirmDialogButton {
                text: qsTr("Cancel")
                onClicked: control.close()
            }

            FishUI.ConfirmDialogButton {
                text: control.saving ? qsTr("Saving…") : qsTr("Save")
                primary: true
                enabled: !control.saving && _editor.complete
                onClicked: control.save()
            }
        }
    }
}
