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

    property var itemHeight: 45
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

    NM.Configuration {
        id: configuration
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
            anchors.bottomMargin: FishUI.Units.largeSpacing
            spacing: FishUI.Units.largeSpacing * 2

            RoundedItem {
                WifiView {
                    Layout.fillWidth: true
                    visible: enabledConnections.wirelessHwEnabled
                }
            }

            // Hotspot
            // 还未完善
//            RoundedItem {
//                id: hotspotItem
//                visible: handler.hotspotSupported

//                RowLayout {
//                    Label {
//                        text: qsTr("Hotspot")
//                        color: FishUI.Theme.disabledTextColor
//                    }

//                    Item {
//                        Layout.fillWidth: true
//                    }

//                    Switch {
//                        Layout.fillHeight: true
//                        rightPadding: 0

//                        onToggled: {
//                            if (checked) {
//                                handler.createHotspot()
//                            } else {
//                                handler.stopHotspot()
//                            }
//                        }
//                    }
//                }

//                Item {
//                    height: FishUI.Units.largeSpacing
//                }

//                TextField {
//                    id: ssidName
//                    text: configuration.hotspotName
//                    placeholderText: qsTr("SSID")
//                }

//                TextField {
//                    id: hotspotPassword
//                    placeholderText: qsTr("Password")
//                    text: configuration.hotspotPassword
//                }
//            }

            // Wired connection
            RoundedItem {
                visible: enabledConnections.wwanHwEnabled

                RowLayout {
                    spacing: FishUI.Units.largeSpacing

                    Label {
                        text: qsTr("Wired")
                        color: FishUI.Theme.disabledTextColor
                        Layout.fillWidth: true
                    }

                    Switch {
                        Layout.fillHeight: true
                        rightPadding: 0
                        checked: enabledConnections.wwanEnabled
                        onCheckedChanged: {
                            if (checked) {
                                if (!enabledConnections.wwanEnabled) {
                                    handler.enableWwan(checked)
                                }
                            } else {
                                if (enabledConnections.wwanEnabled) {
                                    handler.enableWwan(checked)
                                }
                            }
                        }
                    }
                }

                ListView {
                    id: wiredView

                    visible: enabledConnections.wwanEnabled && wiredView.count > 0

                    Layout.fillWidth: true
                    Layout.preferredHeight: wiredView.count * control.itemHeight
                    interactive: false
                    clip: true

                    model: NM.TechnologyProxyModel {
                        type: NM.TechnologyProxyModel.WiredType
                        showInactiveConnections: true
                        sourceModel: networkModel
                    }

                    ScrollBar.vertical: ScrollBar {}

                    delegate: WiredItem {
                        height: control.itemHeight
                        width: wiredView.width
                    }
                }
            }

            Item {
                height: FishUI.Units.largeSpacing
            }
        }
    }
}
