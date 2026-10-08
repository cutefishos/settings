import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cutefish.Settings
import FishUI as FishUI
import Cutefish.Network as NM
import "../"

// Sets up a VPN from what the provider or the network's administrator hands out: the
// server and how to sign in. A password left empty is asked for when connecting.
FishUI.Window {
    id: control

    required property NM.Handler handler

    windowButtonsTopMargin: Metrics.dialogWindowButtonsMargin
    windowButtonsRightMargin: Metrics.dialogWindowButtonsMargin
    header.height: 44
    contentTopMargin: 0

    width: contentWidth
    height: contentHeight
    minimumWidth: contentWidth
    minimumHeight: contentHeight
    maximumWidth: contentWidth
    maximumHeight: contentHeight
    modality: Qt.WindowModal

    property int contentWidth: 460
    property int contentHeight: _mainLayout.implicitHeight + 2 * Metrics.dialogPadding
    // A long form scrolls instead of outgrowing the screen.
    readonly property int maxFormHeight: rootWindow.height - 2 * Metrics.xxl - 2 * Metrics.dialogPadding
                                         - _titleLabel.implicitHeight - _kindRow.implicitHeight - _buttons.implicitHeight
                                         - (_errorLabel.visible ? _errorLabel.implicitHeight + Metrics.dialogSpacing : 0)
                                         - 3 * Metrics.dialogSpacing

    title: qsTr("Add VPN")
    visible: false
    minimizeButtonVisible: false

    background.color: FishUI.Theme.secondBackgroundColor
    // The background item reaches a grown window a frame or two after its buffer does.
    color: background.color
    flags: Qt.Dialog | Qt.FramelessWindowHint

    // Read again each time the dialog opens, so a plugin installed meanwhile shows up.
    property var kinds: []
    readonly property var kindInfo: kinds[_kindBox.currentIndex] ?? ({})
    readonly property string kind: kindInfo.kind ?? ""

    property bool adding: false
    property string error

    readonly property bool canAdd: !adding && _editor.complete

    function add() {
        if (!canAdd)
            return

        adding = true
        error = ""
        handler.saveVpn("", _editor.collect())
    }

    function loadKind() {
        if (kind.length === 0)
            return
        _editor.load(handler.vpnForm(kind, ""))
        _editor.measure(_kindLabel.implicitWidth)
        _form.contentY = 0
        error = ""
        if (_editor.installed)
            _editor.focusFirst()
    }

    onVisibleChanged: {
        if (visible) {
            kinds = handler.vpnKinds().map(function(item) {
                item.text = item.installed ? item.label : qsTr("%1 (Not Installed)").arg(item.label)
                return item
            })
            _kindBox.currentIndex = 0
            loadKind()
            return
        }

        _editor.load({})
        _kindBox.currentIndex = 0
        adding = false
        error = ""
    }

    Connections {
        target: control.handler

        function onVpnAdded() {
            if (!control.adding)
                return
            control.adding = false
            control.close()
        }

        function onVpnAddFailed(message) {
            if (!control.adding)
                return
            control.adding = false
            control.error = message
        }
    }

    Shortcut {
        sequence: "Escape"
        onActivated: control.close()
    }

    ColumnLayout {
        id: _mainLayout
        // Not filled to the window: the window height follows this layout's implicit
        // height a polish later, and a filled layout would be spread over the old height.
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: Metrics.dialogPadding
        spacing: Metrics.dialogSpacing

        FishUI.Label {
            id: _titleLabel
            Layout.fillWidth: true
            Layout.rightMargin: Metrics.xxl
            text: control.title
            textFormat: Text.PlainText
            font.pixelSize: 14
            font.weight: Font.DemiBold
            color: FishUI.Theme.textColor
            wrapMode: Text.Wrap
        }

        RowLayout {
            id: _kindRow
            Layout.fillWidth: true
            spacing: Metrics.dialogSpacing

            FishUI.Label {
                id: _kindLabel
                Layout.preferredWidth: _editor.labelWidth
                Layout.alignment: Qt.AlignVCenter
                text: qsTr("Type")
                textFormat: Text.PlainText
            }

            FishUI.ComboBox {
                id: _kindBox
                Layout.fillWidth: true
                textRole: "text"
                valueRole: "kind"
                model: control.kinds
                onActivated: control.loadKind()
            }
        }

        Flickable {
            id: _form
            Layout.fillWidth: true
            // Spans the padding so the bar runs along the window's edge, not the fields'.
            Layout.leftMargin: -Metrics.dialogPadding
            Layout.rightMargin: -Metrics.dialogPadding
            Layout.preferredHeight: Math.min(_editor.implicitHeight, control.maxFormHeight)
            clip: true
            contentWidth: width
            contentHeight: _editor.implicitHeight
            boundsBehavior: Flickable.StopAtBounds
            FishUI.ScrollBar.vertical: FishUI.ScrollBar {}
            FishUI.WheelHandler { target: _form }

            VpnEditor {
                id: _editor
                x: Metrics.dialogPadding
                width: _form.width - 2 * Metrics.dialogPadding
                onAccepted: control.add()
            }
        }

        FishUI.Label {
            id: _errorLabel
            Layout.fillWidth: true
            visible: control.error.length > 0
            text: control.error
            textFormat: Text.PlainText
            color: FishUI.Theme.redColor
            wrapMode: Text.Wrap
        }

        RowLayout {
            id: _buttons
            spacing: Metrics.dialogButtonSpacing
            Layout.alignment: Qt.AlignRight

            FishUI.ConfirmDialogButton {
                text: qsTr("Cancel")
                onClicked: control.close()
            }

            FishUI.ConfirmDialogButton {
                enabled: control.canAdd
                primary: true
                text: qsTr("Add")
                onClicked: control.add()
            }
        }
    }
}
