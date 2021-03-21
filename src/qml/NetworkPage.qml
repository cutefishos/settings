import QtQuick 2.4
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0

import MeuiKit 1.0 as Meui
import Cutefish.NetworkManagement 1.0 as NM

ItemPage {
    id: control
    headerTitle: qsTr("Network")

    property var itemHeight: 50
    property var settingsMap: ({})

    NM.Networking {
        id: networking
    }

    NM.NetworkModel {
        id: networkModel
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: mainLayout.implicitHeight

        ColumnLayout {
            id: mainLayout
            anchors.fill: parent

            // Wired connection
            ColumnLayout {
                id: wiredLayout

                visible: networking.enabled && wiredView.count > 0

                RowLayout {
                    Label {
                        text: qsTr("Wired")
                        color: Meui.Theme.disabledTextColor
                        Layout.fillWidth: true
                    }
                }

                ListView {
                    id: wiredView

                    Layout.fillWidth: true
                    implicitHeight: wiredView.count * control.itemHeight
                    clip: true

                    model: NM.TechnologyProxyModel {
                        type: NM.TechnologyProxyModel.WiredType
                        sourceModel: networkModel
                    }

                    ScrollBar.vertical: ScrollBar {}

                    delegate: WiredItem {
                        height: control.itemHeight
                        width: wiredView.width
                    }
                }
            }

            HorizontalDivider {
                visible: wiredView.visible && networking.wirelessHardwareEnabled
            }

            // Wireless
            ColumnLayout {
                id: wirelessLayout
                spacing: Meui.Units.largeSpacing

                RowLayout {
                    spacing: Meui.Units.largeSpacing

                    Label {
                        text: qsTr("Wi-Fi")
                        color: Meui.Theme.disabledTextColor
                    }

                    Meui.BusyIndicator {
                        id: wlanBusyIndicator
                        width: wirelessSwitch.height
                        height: width
                        visible: networking.wirelessEnabled && wirelessView.count === 0
                        running: wlanBusyIndicator.visible
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Switch {
                        id: wirelessSwitch
                        Layout.fillHeight: true
                        leftPadding: 0
                        rightPadding: 0
                        onCheckedChanged: networking.wirelessEnabled = checked
                    }

                    Component.onCompleted: {
                        wirelessSwitch.checked = networking.wirelessEnabled
                    }
                }

                ListView {
                    id: wirelessView

                    visible: networking.wirelessEnabled && networking.wirelessHardwareEnabled
                    Layout.fillWidth: true

                    implicitHeight: count * control.itemHeight
                    interactive: false
                    clip: true

                    model: NM.TechnologyProxyModel {
                        type: NM.TechnologyProxyModel.WirelessType
                        sourceModel: networkModel
                        showInactiveConnections: true
                    }

                    ScrollBar.vertical: ScrollBar {}

                    delegate: WirelessItem {
                        height: control.itemHeight
                        width: wirelessView.width
                    }
                }
            }
        }
    }
}
