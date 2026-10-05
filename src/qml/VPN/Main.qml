import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import FishUI as FishUI
import Cutefish.Settings
import Cutefish.Network as NM
import "../"

// The VPNs set up on this machine, each with its own switch, and ways to add one: by
// hand, or from the configuration file a VPN provider hands out.
ItemPage {
    id: control
    headerTitle: qsTr("VPN")

    property bool importing: false
    property string importError

    NM.Handler {
        id: handler

        onVpnAdded: {
            control.importing = false
            control.importError = ""
        }
        onVpnAddFailed: (message) => {
            // The Add VPN dialog shows its own failures.
            if (!control.importing)
                return
            control.importing = false
            control.importError = message
        }
    }

    AddVpnDialog {
        id: addDialog
        handler: handler
    }

    NM.NetworkModel {
        id: networkModel
    }

    NM.AppletProxyModel {
        id: vpnModel
        type: NM.AppletProxyModel.VpnType
        sourceModel: networkModel
    }

    FileDialog {
        id: fileDialog
        title: qsTr("Import VPN Configuration")
        nameFilters: [qsTr("VPN configurations (*.ovpn *.conf *.pcf)"), qsTr("All files (*)")]
        onAccepted: {
            control.importing = true
            control.importError = ""
            handler.importVpn(selectedFile)
        }
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent
            spacing: Metrics.sectionLabelSpacing

            ListCard {
                Repeater {
                    model: vpnModel
                    delegate: VpnItem {}
                }

                // Declared after the Repeater, which inserts its rows before itself.
                ListRow {
                    text: qsTr("Add VPN…")
                    enabled: !control.importing
                    onClicked: addDialog.show()
                }

                ListRow {
                    text: control.importing ? qsTr("Importing…") : qsTr("Import from file…")
                    enabled: !control.importing
                    onClicked: fileDialog.open()
                }
            }

            FishUI.Label {
                Layout.fillWidth: true
                Layout.leftMargin: Metrics.cardHorizontalPadding
                Layout.rightMargin: Metrics.cardHorizontalPadding
                text: control.importError.length > 0
                      ? control.importError
                      : qsTr("Import the configuration file your VPN provider gives you, such as an OpenVPN .ovpn or a WireGuard .conf file.")
                textFormat: Text.PlainText
                color: control.importError.length > 0 ? FishUI.Theme.redColor : FishUI.Theme.disabledTextColor
                wrapMode: Text.Wrap
            }
        }
    }
}
