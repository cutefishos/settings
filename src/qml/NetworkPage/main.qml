import QtQuick 2.4
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0

import FishUI 1.0 as FishUI
import Cutefish.NetworkManagement 1.0 as NM

import "../"

ItemPage {
    id: control
    headerTitle: qsTr("Network")

    property var itemHeight: 50
    property var settingsMap: ({})

    NM.Handler {
        id: handler
    }

    NM.WifiSettings {
        id: wifiSettings
    }

    NM.NetworkModel {
        id: networkModel
    }

    NM.EnabledConnections {
        id: enabledConnections
    }

    NM.IdentityModel {
        id: connectionModel
    }

    NM.AppletProxyModel {
        id: appletProxyModel
        sourceModel: connectionModel
    }

    Component.onCompleted: handler.requestScan()

    Timer {
        id: scanTimer
        interval: 10200
        repeat: true
        running: control.visible
        onTriggered: handler.requestScan()
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

                visible: enabledConnections.wwanEnabled && wiredView.count > 0

                RowLayout {
                    Label {
                        text: qsTr("Wired")
                        color: FishUI.Theme.disabledTextColor
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
                visible: wiredView.visible && enabledConnections.wirelessHwEnabled
            }

            WifiView {
                Layout.fillWidth: true
                visible: enabledConnections.wirelessHwEnabled
            }
        }
    }
}
